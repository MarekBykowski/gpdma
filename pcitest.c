/*
 *  PCIe Enpoint Test Driver.
 *
 *  Copyright (c) LSI Corporation, 2013
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License 2 as published by the
 *  Free Software Foundation.
 *
 */
#define CREATE_TRACE_POINTS
#include "l3-dma-trace.h"

/*#define ITERATIONS 500 -> defined in Makfile*/
/*#define L3_LOCKED -> defined in Makefile */
#define UPDATED_DMA_MAP 1

#undef KBUILD_MODNAME
#define KBUILD_MODNAME "pcitest"


#include <linux/random.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/dmaengine.h>
#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/highmem.h>

#include "average.h"


#define DRV_NAME		"pcietest"
#define NS				1000000000

static ktime_t ktime_start;
static ktime_t ktime_avg_duration = { .tv64 = 0 };
static struct sma sma_avg;

struct pet_private {
        struct pci_dev  *pdev;
	void __iomem    *bar[5];
        size_t           rlen;
        char             result[1024];
};

#define pr_result(_pet, _fmt, ...) \
    (_pet)->rlen += sprintf(&(_pet)->result[(_pet)->rlen], _fmt, ## __VA_ARGS__)

/* below is currently assuming 64-bit BAR 2 configuration on CT1 in BIOS */

#define BAR_MEM    2
#define BAR_OFFSET 0
#define BAR_SIZE 0x100000
#define BAR_OFFSET_2 BAR_SIZE/2

static unsigned long testsize = 16384;

#if L3_LOCKED
static unsigned long __maybe_unused
display_mapping(unsigned long address) {
    unsigned long par_el1;

    pr_info("----- Translating VA 0x%lx\n", address);
    __asm__ __volatile__ ("at s1e1r, %0" : : "r" (address));
    __asm__ __volatile__ ("mrs %0, PAR_EL1\n" : "=r" (par_el1));

    if (0 != (par_el1 & 1)) {
        pr_info("Address Translation Failed: 0x%lx\n"
              "    FSC: 0x%lx\n"
              "    PTW: 0x%lx\n"
              "      S: 0x%lx\n",
              address,
              (par_el1 & 0x7e) >> 1,
              (par_el1 & 0x100) >> 8,
              (par_el1 & 0x200) >> 9);
    } else {
        pr_info("Address Translation Succeeded: 0x%lx\n"
              "  SH: 0x%lx\t(b11 -> Innnershareable)\n"
              "  NS: 0x%lx\t(unknown for non-secure)\n"
              "  PA: 0x%lx\t(bits[47:12] of PA)\n"
              "ATTR: 0x%lx\t(bits[7:4]11RW -> Outer WBNT,bits[3:0]11RW -> Inner WBNT)\n",
              address,
              (par_el1 & 0x180) >> 7,
              (par_el1 & 0x200) >> 9,
              par_el1 & 0xfffffffff000,
              (par_el1 & 0xff00000000000000) >> 56);
    }

    return (par_el1 & 0xfffffffff000);
}
#endif

static inline u32 get_cntfrq(void)
{
	return arch_timer_get_cntfrq();
}

static inline u64 get_cntpct(void)
{
    return arch_counter_get_cntvct(); /* cntpct not implemented, hangs kernel use vct instead */
}


static struct dma_chan *
request_dma(struct dma_device **devp)
{
    dma_cap_mask_t mask;
    struct dma_chan *chan;

    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
	dma_cap_set(DMA_SG, mask);

    chan = dma_request_channel(mask, NULL, NULL);
    if (!chan)
        pr_err("Failed to request DMA channel\n");
    else
	{
		*devp = chan->device;
	}
    return chan;
}

static void
release_dma(struct dma_chan *chan)
{
    dma_release_channel(chan);
}


dma_cookie_t
 dma_async_memcpy_pg_to_pg(struct dma_chan *chan, struct page *dest_pg,
         unsigned int dest_off, struct page *src_pg, unsigned int src_off,
         size_t len)
 {
         struct dma_device *dev = chan->device;
         struct dma_async_tx_descriptor *tx;
         struct dmaengine_unmap_data *unmap;
         dma_cookie_t cookie;
         unsigned long flags;
         unmap = dmaengine_get_unmap_data(dev->dev, 2, GFP_NOWAIT);
         if (!unmap)
         {
		 return -ENOMEM;
         }
         unmap->to_cnt = 1;
         unmap->from_cnt = 1;
         unmap->addr[0] = dma_map_page(dev->dev, src_pg, src_off, len,
                                       DMA_TO_DEVICE);
         unmap->addr[1] = dma_map_page(dev->dev, dest_pg, dest_off, len,
                                       DMA_FROM_DEVICE);
         unmap->len = len;
         flags = DMA_CTRL_ACK;
         tx = dev->device_prep_dma_memcpy(chan, unmap->addr[1], unmap->addr[0],
                                          len, flags);

         if (!tx) {
                 dmaengine_unmap_put(unmap);
                 return -ENOMEM;
         }

         dma_set_unmap(tx, unmap);
         cookie = tx->tx_submit(tx);
         dmaengine_unmap_put(unmap);

         preempt_disable();
         __this_cpu_add(chan->local->bytes_transferred, len);
         __this_cpu_inc(chan->local->memcpy_count);
         preempt_enable();
         return cookie;
 }

static dma_cookie_t
dma_async_memcpy_pci_to_buf(struct dma_chan *chan, void *dest, phys_addr_t src, size_t len)
{
	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *tx;
	dma_addr_t dma_dest;
	dma_cookie_t cookie;
	unsigned long flags;
	enum dma_status status;
	ktime_t ktime_duration;

#if UPDATED_DMA_MAP
	dma_dest = (dma_addr_t) dma_map_single_par_l1(dev->dev, (void*)dest, len, DMA_FROM_DEVICE);
#else
	dma_dest = dma_map_single(dev->dev, dest, len, DMA_FROM_DEVICE);
#endif

	if (dma_mapping_error(dev->dev, dma_dest)) {
		return -ENOMEM;
	}
	flags = DMA_CTRL_ACK;

	tx = dev->device_prep_dma_memcpy(chan, dma_dest, src, len, flags);
	if (!tx) {
#if UPDATED_DMA_MAP
		dma_unmap_single_attrs_par_l1(dev->dev, dma_dest, len, DMA_FROM_DEVICE, (unsigned long)dest);
#else
		dma_unmap_single(dev->dev, dma_dest, len, DMA_FROM_DEVICE);
#endif
		return -ENOMEM;
	}
	tx->callback = NULL;

	trace_l3_dma_entry("dmaengine_submit ----> measure from here");
	ktime_start = ktime_get();
	cookie = dmaengine_submit(tx); /*cookie = tx->tx_submit(tx);*/
	if (dma_submit_error(cookie)) {
		printk(KERN_ERR "%s: Failed to start DMA transfer\n", __func__);
#if UPDATED_DMA_MAP
		dma_unmap_single_attrs_par_l1(dev->dev, dma_dest, len, DMA_FROM_DEVICE, (unsigned long)dest);
#else
		dma_unmap_single(dev->dev, dma_dest, len, DMA_FROM_DEVICE);
#endif
		return -ENODEV;
	}

	trace_l3_dma_entry("dma_sync_wait");
	status = dma_sync_wait(chan, cookie);
	trace_l3_dma_exit("dma_sync_wait <---- up to here");

	ktime_duration = ktime_sub(ktime_get(), ktime_start);
	ktime_avg_duration.tv64 = sma_add(&sma_avg, ktime_duration.tv64);
	trace_l3_dma_duration(smp_processor_id(),
				ktime_to_timespec(ktime_duration),
				ktime_to_timespec(ktime_avg_duration)
	);

#if UPDATED_DMA_MAP
	dma_unmap_single_attrs_par_l1(dev->dev, dma_dest, len, DMA_FROM_DEVICE, (unsigned long)dest);
#else
	dma_unmap_single(dev->dev, dma_dest, len, DMA_FROM_DEVICE);
#endif

	preempt_disable();
	__this_cpu_add(chan->local->bytes_transferred, len);
	__this_cpu_inc(chan->local->memcpy_count);
	preempt_enable();
	return status;
}

static enum dma_status
dma_async_memcpy_buf_to_pci(struct dma_chan *chan, phys_addr_t dest, void *src, size_t len)
{
	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *tx;
	dma_addr_t dma_src;
	dma_cookie_t cookie;
	unsigned long flags;
	enum dma_status status;
	ktime_t ktime_duration;
#if 0
{
	/*void* pa;*/
	phys_addr_t phys1, phys2;
	unsigned long page_addr1;
	struct page *page1;
	void* kmap1;

	/*pr_info("mb: %s() PAGE_OFFSET %lx\n", __func__, (unsigned long) page_address(virt_to_page(PAGE_OFFSET)));*/

	page1 = virt_to_page(src);
	kmap1=kmap(virt_to_page(src));
	pr_info("kmap returns %p\n", (void*) kmap1);


	/*void *page_address(const struct page *page)*/
	page_addr1 = (unsigned long) page_address(page1);
	pr_info("src %p src page_address %lx\n", (void*) src, page_addr1);

	phys1 = page_to_phys(page1);
	phys2 = virt_to_phys(src);
	/*pa = page_address(p1);*/
	pr_info("mb: %s() phys1 %lx phys2 %lx\n", __func__, (unsigned long) phys1,(unsigned long) phys2);

	if (PageHighMem(virt_to_page(src)))
		pr_info("mb: src is in highmem\n");
}
#endif

	trace_l3_dma_entry("dma_map_single");
#if UPDATED_DMA_MAP
	dma_src = (dma_addr_t) dma_map_single_par_l1(dev->dev, (void*)src, len, DMA_TO_DEVICE);
#else
	dma_src = dma_map_single(dev->dev, src, len, DMA_TO_DEVICE);
#endif
	trace_l3_dma_exit("dma_map_single");

	trace_l3_dma_entry("dma_mapping_error");
	if (dma_mapping_error(dev->dev, dma_src)) {
		return -ENOMEM;
	}
	trace_l3_dma_exit("dma_mapping_error");

	flags = DMA_CTRL_ACK;

	trace_l3_dma_entry("device_prep_dma_memcpy");
	tx = dev->device_prep_dma_memcpy(chan, dest, dma_src, len, flags);
	trace_l3_dma_exit("device_prep_dma_memcpy");

	if (!tx) {
			trace_l3_dma_entry("dma_unmap_single");
#if UPDATED_DMA_MAP
			dma_unmap_single_attrs_par_l1(dev->dev, dma_src, len, DMA_TO_DEVICE, (unsigned long)src);
#else
			dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
#endif
			trace_l3_dma_exit("dma_unmap_single");
			return -ENOMEM;
	}

	tx->callback = NULL;

	trace_l3_dma_entry("dmaengine_submit ----> measure from here");
	ktime_start = ktime_get();
	pr_info("mb: dma_submit\n");
	cookie = dmaengine_submit(tx); /*cookie = tx->tx_submit(tx);*/
	trace_l3_dma_exit("dmaengine_submit");
	if (dma_submit_error(cookie)) {
		printk(KERN_ERR "%s: Failed to start DMA transfer\n", __func__);
#if UPDATED_DMA_MAP
		dma_unmap_single_attrs_par_l1(dev->dev, dma_src, len, DMA_TO_DEVICE, (unsigned long)src);
#else
		dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
#endif
		return -ENODEV;
	}

	trace_l3_dma_entry("dma_sync_wait");
	status = dma_sync_wait(chan, cookie);
	trace_l3_dma_exit("dma_sync_wait <---- up to here");

	ktime_duration = ktime_sub(ktime_get(), ktime_start);
	ktime_avg_duration.tv64 = sma_add(&sma_avg, ktime_duration.tv64);
	trace_l3_dma_duration(smp_processor_id(),
				ktime_to_timespec(ktime_duration),
				ktime_to_timespec(ktime_avg_duration)
	);

	trace_l3_dma_entry("dma_unmap_single");
#if UPDATED_DMA_MAP
	dma_unmap_single_attrs_par_l1(dev->dev, dma_src, len, DMA_TO_DEVICE, (unsigned long)src);
#else
	dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
#endif
	trace_l3_dma_exit("dma_unmap_single");

	preempt_disable();
	__this_cpu_add(chan->local->bytes_transferred, len);
	__this_cpu_inc(chan->local->memcpy_count);
	preempt_enable();

	return status;
}


static enum dma_status
dma_async_memcpy_mem2mem(struct dma_chan *chan, void * dest, void *src, size_t len)
{
        struct dma_device *dev = chan->device;
        struct dma_async_tx_descriptor *tx;
        dma_addr_t dma_src;
		dma_addr_t dma_dest;
        dma_cookie_t cookie;
        unsigned long flags;
		enum dma_status status;
		ktime_t ktime_duration;

#if UPDATED_DMA_MAP
		dma_src = (dma_addr_t) dma_map_single_par_l1(dev->dev, (void*)src, len, DMA_TO_DEVICE);
		dma_dest = (dma_addr_t) dma_map_single_par_l1(dev->dev, (void*)dest, len, DMA_TO_DEVICE);
#else
		dma_src = dma_map_single(dev->dev, src, len, DMA_TO_DEVICE);
		dma_dest = dma_map_single(dev->dev, dest, len, DMA_TO_DEVICE);
#endif
        flags = DMA_CTRL_ACK;

        tx = dev->device_prep_dma_memcpy(chan, dma_dest, dma_src, len, flags);

        if (!tx) {
                dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
				dma_unmap_single(dev->dev, dma_dest, len, DMA_TO_DEVICE);
                return -ENOMEM;
        }

        tx->callback = NULL;

		/*cookie = tx->tx_submit(tx);*/
		trace_l3_dma_entry("dmaengine_submit ----> measure from here");
		ktime_start = ktime_get();
		cookie = dmaengine_submit(tx); /*cookie = tx->tx_submit(tx);*/
		trace_l3_dma_exit("dmaengine_submit");

		if (dma_submit_error(cookie)) {
			printk(KERN_ERR "%s: Failed to start DMA transfer\n", __func__);
#if UPDATED_DMA_MAP
			dma_unmap_single_attrs_par_l1(dev->dev, dma_src, len, DMA_TO_DEVICE, (unsigned long)src);
			dma_unmap_single_attrs_par_l1(dev->dev, dma_dest, len, DMA_TO_DEVICE, (unsigned long)dest);
#else
			dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
			dma_unmap_single(dev->dev, dma_dest, len, DMA_TO_DEVICE);
#endif
			return -ENODEV;
		}

		trace_l3_dma_entry("dma_sync_wait");
		status = dma_sync_wait(chan, cookie);
		trace_l3_dma_exit("dma_sync_wait <---- up to here");

		ktime_duration = ktime_sub(ktime_get(), ktime_start);
		ktime_avg_duration.tv64 = sma_add(&sma_avg, ktime_duration.tv64);
		trace_l3_dma_duration(smp_processor_id(),
					ktime_to_timespec(ktime_duration),
					ktime_to_timespec(ktime_avg_duration)
		);

#if UPDATED_DMA_MAP
		dma_unmap_single_attrs_par_l1(dev->dev, dma_src, len, DMA_TO_DEVICE, (unsigned long)src);
		dma_unmap_single_attrs_par_l1(dev->dev, dma_dest, len, DMA_TO_DEVICE, (unsigned long)dest);
#else
		dma_unmap_single(dev->dev, dma_src, len, DMA_TO_DEVICE);
		dma_unmap_single(dev->dev, dma_dest, len, DMA_TO_DEVICE);
#endif

        preempt_disable();
        __this_cpu_add(chan->local->bytes_transferred, len);
        __this_cpu_inc(chan->local->memcpy_count);
        preempt_enable();

        return status;
}

static dma_cookie_t
dma_async_memcpy_ep_to_ep(struct dma_chan *chan, phys_addr_t dest, phys_addr_t src, size_t len)
{
        struct dma_device *dev = chan->device;
        struct dma_async_tx_descriptor *tx;
        dma_cookie_t cookie;
        unsigned long flags;

         flags = DMA_CTRL_ACK;

        tx = dev->device_prep_dma_memcpy(chan, dest, src, len, flags);

        if (!tx) {
                return -ENOMEM;
        }

        tx->callback = NULL;

        cookie = tx->tx_submit(tx);

        preempt_disable();
        __this_cpu_add(chan->local->bytes_transferred, len);
        __this_cpu_inc(chan->local->memcpy_count);
        preempt_enable();

        return cookie;
}


static int
test_memcpy(struct pet_private *pet, int write)
{
    char *buf;
    u64 t0,t1,elapsed;
    unsigned long buffset = BAR_OFFSET;
	u64 i=0, average = 0;
	enum _test_result {fail = 0, pass} test_result = pass;

    buf = kmalloc(testsize, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;
    memset(buf, 0, testsize);

	for(i = 0; i < ITERATIONS; i++)
	{
		t0 = get_cntpct();
		if (write)
			memcpy((pet->bar[BAR_MEM] + buffset), buf, testsize);
		else
			memcpy(buf, (pet->bar[BAR_MEM] + buffset), testsize);
		t1 = get_cntpct();

		average += div_u64(NS*(t1-t0), get_cntfrq());
		if (crc32(~0,pet->bar[BAR_MEM]+buffset,testsize) != crc32(~0,buf,testsize)) {
			pr_result(pet, "ERROR: crc failed\n");
			test_result = fail;
			break;
		}
	}

    elapsed = average/ITERATIONS;

	if (testsize == 64) {
		pr_result(pet, "memcpy_%s %lu bytes (overhead) %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, buf, testsize), test_result?"pass":"fail" );
	} else {
		pr_result(pet, "memcpy_%s %lu bytes %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, buf, testsize), test_result?"pass":"fail" );
	}

    kfree(buf);

    return 0;
}

static int
test_dma(struct pet_private *pet, int write)
{
    void *buf;
	void *dest = NULL;
    u64 t0,t1,elapsed;
	u64 i=0, average = 0;
    struct dma_device *dev;
    struct dma_chan *chan;
    enum dma_status status;
#if L3_LOCKED
	unsigned long l3_base_address = 0x80000000;
#endif
	enum _test_result {fail = 0, pass} test_result = pass;
    phys_addr_t boffset = BAR_OFFSET;

#if L3_LOCKED
{
	/*
  	  request_mem_region(start,n,name);
	  void __iomem *ioremap_cache(phys_addr_t phys_addr, size_t size);
	  pfn = page_to_pfn (virt_to_page (buffer + offset)); */

	if (!request_mem_region(l3_base_address, testsize, "pcitest"))
		return -EBUSY;

	buf = (void*) ioremap_cache(l3_base_address, testsize);
	if (!buf)
		return -ENOMEM;

	/*display_mapping((unsigned long)buf);*/
}
#else

    buf = kmalloc(testsize, GFP_KERNEL);

    if (!buf)
        return -ENOMEM;

#endif

    chan = request_dma(&dev);
    for(i = 0; i < ITERATIONS; i++)
	{
		/* Mimick the worst-case scenario where cache gets taken by another Masters
 		   These Masters are for example cpus that run LmBench.
 		 */
		if (write) {
			memset(buf, 0x12, testsize);
			msleep(5);
		}

		t0 = get_cntpct();
		if (write) {
			status = dma_async_memcpy_buf_to_pci(chan, boffset + pci_resource_start(pet->pdev, BAR_MEM), buf, testsize);
		} else {
			status = dma_async_memcpy_pci_to_buf(chan, buf, boffset + pci_resource_start(pet->pdev, BAR_MEM), testsize);
		}

		t1 = get_cntpct();

		average += div_u64(NS * (t1-t0), get_cntfrq());

		/* map memory CT1 memory and do checksum check */
		dest = ioremap(boffset+pci_resource_start(pet->pdev, BAR_MEM), testsize);
		if(dest == NULL){
			pr_result(pet, "ERROR: Unable to map memory to verify results, verification must be manual\n");
			goto cleanup;
		}
		else {
			#define DEBUG 0
			#if DEBUG
			pr_info("buf: 0x%x\n", *(volatile unsigned *)buf);
			pr_info("dest: 0x%x\n", *(volatile unsigned *)dest);
			#endif
			if (crc32(~0, buf, testsize) != crc32(~0, dest, testsize)) {
				iounmap(dest);
				pr_result(pet, "ERROR: crc failed\n");
				test_result = fail;
				break;
			}
			iounmap(dest);
		}
	}

    elapsed = average/ITERATIONS;
    if (status != 0) {
        pr_result(pet, "ERROR: DMA failed, status %d\n", status);
		test_result = fail;
    }

	if (testsize == 64) {
		pr_result(pet, "dma_%s %lu bytes (overhead) %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, buf, testsize), test_result?"pass":"fail" );
	} else {
		pr_result(pet, "dma_%s %lu bytes %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, buf, testsize), test_result?"pass":"fail" );
	}

cleanup:
    release_dma(chan);
#if L3_LOCKED
	iounmap((void __iomem*)buf);
	release_mem_region(l3_base_address, testsize);
#else
    kfree(buf);
#endif

    return 0;
}

static int
test_ep2ep(struct pet_private *pet, int write)
{
    u64 t0,t1,elapsed;
	u64 i=0, average = 0;
    struct dma_device *dev;
    struct dma_chan *chan;
    dma_cookie_t cookie;
    enum dma_status status;
	enum _test_result {fail = 0, pass} test_result = pass;
    phys_addr_t boffset = BAR_OFFSET;
    phys_addr_t boffset2 = BAR_OFFSET_2;
	void *src = NULL;
	void *dest = NULL;

	chan = request_dma(&dev);

	for(i = 0; i < ITERATIONS; i++)
	{
		t0 = get_cntpct();
		if (write)
			cookie = dma_async_memcpy_ep_to_ep(chan, boffset+pci_resource_start(pet->pdev, BAR_MEM), boffset2+pci_resource_start(pet->pdev, BAR_MEM), testsize);
		else
			cookie = dma_async_memcpy_ep_to_ep(chan, boffset2+pci_resource_start(pet->pdev, BAR_MEM), boffset+pci_resource_start(pet->pdev, BAR_MEM), testsize);
		status = dma_sync_wait(chan, cookie);

		t1 = get_cntpct();

    	average += div_u64(NS * (t1-t0), get_cntfrq());

		if (status != 0) {
			pr_result(pet, "ERROR: DMA failed, status %d\n", status);
			test_result = fail;
			break;
		}


		/* checking results takes a bit more work as memory is on endpoint */
		src = ioremap(boffset+pci_resource_start(pet->pdev, BAR_MEM), testsize);
		dest= ioremap(boffset2+pci_resource_start(pet->pdev, BAR_MEM), testsize);

		if(src == NULL || dest == NULL) {
			pr_result(pet, "ERROR: Unable to map memory to verify results, verification must be manual\n");
			goto cleanup;
		} else {
			if (crc32(~0, src, testsize) != crc32(~0, dest, testsize)) {
				pr_result(pet, "ERROR: crc failed\n");
				test_result = fail;
				break;
			}
		}
	}

	elapsed = average/ITERATIONS;

	if (testsize == 64) {
		pr_result(pet, "ep2ep_%s %lu bytes (overhead) %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	} else {
		pr_result(pet, "ep2ep_%s %lu bytes %llu ns crc32 %#x %s\n",
				  write?"write":"read", testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	}

cleanup:
	if(src != NULL)
		iounmap(src);
	if(dest != NULL)
		iounmap(dest);

    release_dma(chan);

    return 0;
}


static int
test_mem2mem(struct pet_private *pet)
{
    char *src, *dst;
    u64 t0,t1,elapsed;
    struct dma_device *dev;
    struct dma_chan *chan;
    enum dma_status status;
	enum _test_result {fail = 0, pass} test_result = pass;
	u64 i=0, average = 0;
	enum dma_status;
#if L3_LOCKED
	unsigned long l3_base_address = 0x80000000;
#endif

	/*
  	  request_mem_region(start,n,name);
	  void __iomem *ioremap_cache(phys_addr_t phys_addr, size_t size);
	  pfn = page_to_pfn (virt_to_page (buffer + offset)); */

	if (!request_mem_region(l3_base_address, SZ_32M, "pcitest"))
		return -EBUSY;

#if 1
	src = (void*) ioremap_cache(l3_base_address, SZ_16M);
	if (!src)
		return -ENOMEM;
#else
    src = kmalloc(testsize, GFP_KERNEL);
    if (!src)
        return -ENOMEM;
#endif

#if 1
	dst = (void*) ioremap_cache(l3_base_address+SZ_16M, SZ_16M);
	if (!dst)
		return -ENOMEM;
#else
    dst = kmalloc(testsize, GFP_KERNEL);
    if (!dst)
        return -ENOMEM;
#endif

    chan = request_dma(&dev);

	for(i = 0; i < ITERATIONS; i++)
	{
		/* Mimick the worst-case scenario where cache gets taken by another Masters
 		   These Masters are eg. LmBench that gets run on all the cpus but cpu running this task.
 		 */
    	memset(src, 0xab, testsize);
    	memset(dst, 0xba, testsize);
		msleep(5);

		t0 = get_cntpct();
		status = dma_async_memcpy_mem2mem(chan, dst, src, testsize);
		t1 = get_cntpct();
#if 0
		pr_info("mb: src %x\n", *(volatile unsigned*)src);
		pr_info("mb: dst %x\n", *(volatile unsigned*)dst);
#endif

		average += div_u64(NS * (t1-t0), get_cntfrq());

		if (status != 0) {
			pr_result(pet, "ERROR: DMA failed, status %d\n", status);
			test_result = fail;
			break;
		}

		if (crc32(~0, src, testsize) != crc32(~0, dst, testsize)) {
			pr_result(pet, "ERROR: crc failed\n");
			test_result = fail;
			break;
		}

	}

	elapsed = average/ITERATIONS;

	if (testsize == 64) {
		pr_result(pet,"mem2mem %lu bytes (overhead) %llu ns crc32 %#x %s\n",
				  testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	} else {
		pr_result(pet, "mem2mem %lu bytes %llu ns crc32 %#x %s\n",
				  testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	}

    release_dma(chan);
#if L3_LOCKED
	iounmap((void __iomem*)dst);
	iounmap((void __iomem*)src);
	release_mem_region(l3_base_address, SZ_32M);
#else
    kfree(dst);
    kfree(src);
#endif

    return 0;
}

/* simple host memcopy test no DMA */
static int
test_memcpy_host(struct pet_private *pet)
{
    char *src, *dst;
	enum _test_result {fail = 0, pass} test_result = pass;
    u64 t0,t1,elapsed;
	u64 i=0, average = 0;

    src = kmalloc(testsize, GFP_KERNEL);

    if (!src)
        return -ENOMEM;

    memset(src, 0, testsize);

    dst = kmalloc(testsize, GFP_KERNEL);
    if (!dst)
        return -ENOMEM;

    for(i = 0; i < ITERATIONS; i++)
	{
		t0 = get_cntpct();
		memcpy (src, dst, testsize);
		t1 = get_cntpct();

		average += div_u64(NS * (t1-t0), get_cntfrq());

		if (crc32(~0, src, testsize) != crc32(~0, dst, testsize))
		{
			pr_result(pet, "ERROR: crc failed\n");
			test_result = fail;
			break;
		}
	}


	elapsed = average/ITERATIONS;

	if (testsize == 64) {
		pr_result(pet,"memcpy %lu bytes (overhead) %llu ns crc32 %#x %s\n",
				  testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	} else {
		pr_result(pet, "memcpy %lu bytes %llu ns crc32 %#x %s\n",
				  testsize, elapsed, crc32(~0, src, testsize), test_result?"pass":"fail" );
	}

    kfree(dst);
    kfree(src);

    return 0;
}

static int test_time(void)
{
	 u64 t0,t1;
	 t0 = get_cntpct();
	 msleep(10000);	/* sleep 10 seconds */
	 t1 = get_cntpct();
	 printk(KERN_INFO "Sleep 10 seconds, measured elapsed time %llu us\n",
              div_u64(1000000*(t1-t0), get_cntfrq()));

	return 0;

}

static ssize_t __ref
show_test(struct device *dev, struct device_attribute *attr,
          char *buf)
{
    strcpy(buf, "memcpy_read memcpy_write dma_read memcpy dma_write mem2mem ep2ep_read ep2ep_write\n");
    return strlen(buf);
}


/**
 * scatter_buffer - Chop up the buffer @buf into random sized chunks and put
 * them into the scatterlist @sg.
 */
static int
scatter_buffer(void *buf, size_t len, struct scatterlist *sg, size_t max_ents)
{
	size_t n;
	size_t chunk;
	size_t remain = len;

	for (n = 0; n < max_ents && remain > 0; ++n) {
		if (n == max_ents - 1) {
			chunk = remain;
		} else {
			chunk = 1 + (get_random_int() % remain);
			chunk = min_t(size_t, remain, chunk);
		}
		sg_set_buf(&sg[n], buf, chunk);
		buf += chunk;
		remain -= chunk;
	}

	return n;
}


dma_cookie_t
dma_async_memcpy_sg_to_sg(struct dma_chan *chan,
			  struct scatterlist *dst_sg, unsigned int dst_nents,
			  struct scatterlist *src_sg, unsigned int src_nents)
{

	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *tx;
	int dst_pents, src_pents;
	dma_cookie_t cookie;

	dst_pents = dma_map_sg(dev->dev, dst_sg, dst_nents, DMA_BIDIRECTIONAL);
	if (dst_pents == 0) {
		pr_err("dma_async_memcpy_sg_to_sg: Failed to map sg)\n");
		return -EINVAL;
	}

	src_pents = dma_map_sg(dev->dev, src_sg, src_nents, DMA_BIDIRECTIONAL);
	if (src_pents == 0) {
		pr_err("dma_async_memcpy_sg_to_sg: Failed to map sg\n");
		return -EINVAL;
	}

	tx = dev->device_prep_dma_sg(chan,
				     dst_sg, dst_pents,
				     src_sg, src_pents,
				     DMA_CTRL_ACK);
	if (!tx) {
		dma_unmap_sg(dev->dev, dst_sg, dst_nents, DMA_BIDIRECTIONAL);
		dma_unmap_sg(dev->dev, src_sg, src_nents, DMA_BIDIRECTIONAL);
		return -ENOMEM;
	}

	tx->callback = NULL;

	cookie = tx->tx_submit(tx);

	return cookie;
}

static void *
kmalloc_zero(size_t len)
{
	return kzalloc(len, GFP_KERNEL);
}

static void *
kmalloc_pattern(size_t len)
{
	u32 seed = len;
	u8 *buf;
	size_t i;

	buf = kmalloc(len, GFP_KERNEL);
	if (buf) {
		for (i = 0; i < len; ++i) {
			seed = next_pseudo_random32(seed);
			buf[i] = seed & 0xff;
		}
	}

	return buf;
}


static int
test_sg2sg(struct pet_private *pet)
{
	char *src, *dst;
	u64 t0,t1,elapsed;
	u64 i=0, average = 0;
	struct dma_device *dev;
	struct dma_chan *chan;
	dma_cookie_t cookie;
	enum dma_status status;
	enum _test_result {fail = 0, pass} test_result = pass;
	struct scatterlist dst_sg[16], src_sg[16];
	int dst_nents, src_nents;
	int result = -EINVAL;

	src = kmalloc_pattern(testsize);
	dst = kmalloc_zero(testsize);
	if (!src || !dst)
		return -ENOMEM;

	/* Split src and dst into scatterlists */
	sg_init_table(src_sg, ARRAY_SIZE(src_sg));
	src_nents = scatter_buffer(src, testsize, src_sg, ARRAY_SIZE(src_sg));
	sg_init_table(dst_sg, ARRAY_SIZE(dst_sg));
	dst_nents = scatter_buffer(dst, testsize, dst_sg, ARRAY_SIZE(dst_sg));

	/* Allocate DMA channel */
	chan = request_dma(&dev);
	if (!chan) {
		pr_result(pet, "sg2sg: request dma failed\n");
		test_result = fail;
		goto cleanup;
	}

	for(i = 0; i < ITERATIONS; i++)
	{
	    t0 = get_cntpct();

		cookie = dma_async_memcpy_sg_to_sg(chan, dst_sg, dst_nents, src_sg, src_nents);
		if (cookie < 0) {
			pr_result(pet, "sg2sg: dma prep failed, error %d\n", cookie);
			test_result = fail;
		}
		status = dma_sync_wait(chan, cookie);

		t1 = get_cntpct();

		average += div_u64(NS * (t1-t0), get_cntfrq());

		if (status != 0) {
			pr_result(pet, "sg2sg: dma failed, status %d\n", status);
			test_result = fail;
			break;
		}

		if ((result = memcmp(src, dst, testsize)) != 0) {
			pr_result(pet, "ERROR: memcmp failed\n");
			test_result = fail;
			break;
		}
	}

	elapsed = average/ITERATIONS;

	pr_result(pet, "sg2sg sglist %d -> %d\n", src_nents, dst_nents);

	if (testsize == 64) {
		pr_result(pet, "sg2sg %6lu bytes (overhead) %llu ns memcmp %s\n",
				  testsize, elapsed, test_result?"pass":"fail" );
	} else {
		pr_result(pet, "sg2sg %6lu bytes %llu ns memcpy %s\n",
				  testsize, elapsed, test_result?"pass":"fail" );
	}

cleanup:
	release_dma(chan);
	if (src) kfree(dst);
	if (dst) kfree(src);

	return result ? -EINVAL : 0;
}


static ssize_t __ref
run_test(struct device *dev, struct device_attribute *attr,
         const char *buf, size_t count)
{
    struct pet_private *pet = dev_get_drvdata(dev);
    ssize_t ret = 0;

    pet->rlen = 0;

    if (!pet->bar[BAR_MEM])
        return -ENODEV;

    if (0 == strncmp(buf, "memcpy_read", 10))
        ret = test_memcpy(pet, 0);
    else if (0 == strncmp(buf, "memcpy_write", 10))
        ret = test_memcpy(pet, 1);
    else if (0 == strncmp(buf, "dma_read", 5))
        ret = test_dma(pet, 0);
	else if (0 == strncmp(buf, "sg2sg", 5))
        ret = test_sg2sg(pet);
    else if (0 == strncmp(buf, "dma_write", 5))
        ret = test_dma(pet, 1);
    else if (0 == strncmp(buf, "mem2mem", 5))
        ret = test_mem2mem(pet);
	else if (0 == strncmp(buf, "memcpy", 5))
        ret = test_memcpy_host(pet);
	else if (0 == strncmp(buf, "ep2ep_read", 10))
        ret = test_ep2ep(pet, 0);
	else if (0 == strncmp(buf, "ep2ep_write", 10))
        ret = test_ep2ep(pet, 1);
	else if (0 == strncmp(buf, "time", 4))
        ret = test_time();
    else
        ret = -EINVAL;

    return ret < 0 ? ret : count;
}

static ssize_t __ref
show_result(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pet_private *pet = dev_get_drvdata(dev);

    memcpy(buf, pet->result, pet->rlen);
    return strlen(buf);
}

/* All transactions destined for the AMIB will pass through
   Register: [pcie0_security] AMIB Security (@0x172.0x1.0x8).... */
static void
disable_PCIe_Security(void)
{
	void __iomem *virtual_addr=ioremap(0xa000400000, 0x20);
/* only one bit in this register */
	writel(1, (void *)(virtual_addr + 8));
	writel(1, (void *)(virtual_addr + 0xc));
	writel(1, (void *)(virtual_addr + 0x10));

	printk(KERN_NOTICE "\nPEI0,1,2 AMIB Security set to pass through");
}

static DEVICE_ULONG_ATTR(testsize, 0644, testsize);
static DEVICE_ATTR(testcase, 0644, show_test, run_test);
static DEVICE_ATTR(result, 0444, show_result, NULL);

static int
pet_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
        struct pet_private *pet;
        int bar;
        int ret;

        pet = kzalloc(sizeof(struct pet_private), GFP_KERNEL);
        if (!pet)
                return -ENOMEM;

        ret = pci_enable_device(pdev);
        if (ret) {
                dev_err(&pdev->dev, "can't enable device.\n");
                goto done;
        }

        ret = pci_request_regions(pdev, KBUILD_MODNAME);
        if (ret) {
                dev_err(&pdev->dev, "pci_request_regions FAILED-%d", ret);
                goto disable_pci;
        }

	 if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
                dev_warn(&pdev->dev, "mydev: No suitable DMA available\n");
                goto disable_pci;
        }


        for (bar = 0; bar < ARRAY_SIZE(pet->bar); bar++) {
                pet->bar[bar] = pci_iomap(pdev, bar, 0);
        }

        if (pet->bar[BAR_MEM] == 0) {
            dev_err(&pdev->dev, "Unable to map PCI memory space\n");
            goto release_reg;
        }

        pet->pdev = pdev;
        pci_set_drvdata(pdev, pet);
        dev_info(&pdev->dev, "PCIe Enpoint Test Driver registered.\n");

        device_create_file(&pdev->dev, &dev_attr_testcase);
        device_create_file(&pdev->dev, &dev_attr_testsize.attr);
        device_create_file(&pdev->dev, &dev_attr_result);

	disable_PCIe_Security(); /* for test purposes only!!! */

		sma_init(&sma_avg, 1000);

{
		struct dma_device *dev;
    	pr_info("mb: (request_dma(&dev))->chan_id %d\n",
					(request_dma(&dev))->chan_id);
    	pr_info("mb: (request_dma(&dev))->chan_id %d\n",
					(request_dma(&dev))->chan_id);
}

        return 0;

release_reg:
        pci_release_regions(pdev);
disable_pci:
        pci_disable_device(pdev);
done:
        kfree(pet);
        dev_info(&pdev->dev, "Probe failed (%d)\n", ret);
        return ret;
}


static void
pet_remove(struct pci_dev *pdev)
{
	struct pet_private *pet = pci_get_drvdata(pdev);
        int i;

	pci_release_regions(pdev);
        for (i = 0; i<ARRAY_SIZE(pet->bar); ++i)
            if (pet->bar[i])
                iounmap(pet->bar[i]);
        device_remove_file(&pdev->dev, &dev_attr_testcase);
        device_remove_file(&pdev->dev, &dev_attr_testsize.attr);
        device_remove_file(&pdev->dev, &dev_attr_result);
	pci_disable_device(pdev);
	kfree(pet);
}

static struct pci_device_id pet_pci_ids[] = {
/*	{ PCI_DEVICE(0x1957, 0x0420) }, */
	{ PCI_DEVICE(0x10b5, 0x87a0) },
	{ 0, },
};

/*
 dtpaulso test  mem2mem only

static int __init my_init(void)
{
	printk(KERN_INFO "Doug got here");


	 test_mem2mem(NULL);

	printk(KERN_INFO "Doug got here dang!");

	 return 0;
}



module_init(my_init);
*/

static struct pci_driver pet_gpio_driver = {
	.name     = DRV_NAME,
	.id_table = pet_pci_ids,
	.probe    = pet_probe,
	.remove   = pet_remove,
};

module_pci_driver(pet_gpio_driver);
/* original author: Anders Berg <anders.berg@lsi.com> */

MODULE_AUTHOR("Douglas Paulson <douglas.t.paulson@intel.com>");
MODULE_DESCRIPTION("PCIe Endpoint Test Driver");
MODULE_LICENSE("GPL v2");
