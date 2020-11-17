#include <linux/export.h>  
#include <linux/average.h> 
#include <linux/kernel.h>  
#include <linux/bug.h>     
#include <linux/log2.h>    


typedef struct sma_helper{
	unsigned int count;
} sma_helper_t;

struct sma {
	unsigned long internal;
	unsigned long n;
	sma_helper_t helper;
}; 

void sma_init(struct sma *avg, unsigned long n)
{
        BUG_ON(n == 0);
        avg->internal = 0;
        avg->n = n;
        avg->helper.count = 0;
}

/*
 *  * new average = old average * (n-1)/n + new value/n
 *   * http://stackoverflow.com/questions/12636613/how-to-calculate-moving-average-without-keeping-the-count-and-data-total
 *    */
unsigned long sma_add(struct sma *avg, unsigned long val)
{
        unsigned long internal = ACCESS_ONCE(avg->internal);

        BUG_ON(avg->n == 0);

        /* increment until n */
        if (avg->helper.count <= avg->n)
                avg->helper.count++;

        if (avg->helper.count <= avg->n) 
                ACCESS_ONCE(avg->internal) = avg->helper.count < avg->n ? 
                        internal + val : (internal + val) / avg->n;
        else {
                internal -= internal / avg->n;
                ACCESS_ONCE(avg->internal) = internal += val / avg->n;
        }

        /* until n return sum to date / n, beyond avg calculated */
        return avg->helper.count < avg->n ? 
                        avg->internal / avg->helper.count : avg->internal;
}

