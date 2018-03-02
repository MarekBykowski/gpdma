#!/usr/bin/env python3
import babeltrace
import glob
import os
from enum import Enum

trace_collection = babeltrace.TraceCollection();

class hwio_type(Enum):
    NCP_HWIO_READ32 = 0
    NCP_HWIO_WRITE32 = 1
    NCP_HWIO_FILL32 = 2
    NCP_HWIO_CRBF = 3
    NCP_HWIO_CRRMW = 4
    NCP_HWIO_CRSW = 5
    NCP_HWIO_CRBB = 6
    NCP_HWIO_CRBSW = 7
    NCP_HWIO_CRBBF = 8
    NCP_HWIO_CSMBR = 9
    NCP_HWIO_CSMBW = 10
    NCP_HWIO_USLEEP = 11
    NCP_HWIO_RESET = 12 
    NCP_HWIO_POLL = 13
    NCP_HWIO_POST_RESET = 14
    NCP_HWIO_COMMENT = 15
    NCP_HWIO_TRANS_START = 16
    NCP_HWIO_TRANS_UDELAY = 17
    NCP_HWIO_TRANS_STALL_2ND_PASS_POLL = 18
    NCP_HWIO_TRANS_END = 19
    NCP_HWIO_READ16 = 20
    NCP_HWIO_WRITE16 = 21
    NCP_HWIO_READ8 = 22
    NCP_HWIO_WRITE8 = 23 
    NCP_HWIO_READ64 = 24  
    NCP_HWIO_WRITE64 = 25

def get_node(regionId):
    return (regionId >> 16) & 0xFFFF

def get_target(regionId):
    return regionId & 0xFFFF



def write_16(pre,event): #copy from write 32
    add_offset = 0;
    buf = event['buffer'];
    count = event['count']
    data = ""
    lastVal = 0;
    #print("count {}",count)
    for x in range (0, count):
        lista = buf[x*2:(x+1)*2]
        data = data + "0x{:02x}{:02x} ".format(lista[1],lista[0]);
    if ((len(data)>0)):
        cmd = "{}ncpWrite -w16 {}.{}.{}.0x{:010x}".format(pre,event['devNum'],get_node(event['regionId']),\
                get_target(event['regionId']),int(event['offset'])+0x10*add_offset)
        print("{} {}".format(cmd, data))
    data = ""




def write_32(pre,event):
    add_offset = 0;
    buf = event['buffer'];
    count = event['count']
    data = ""
    lastVal = 0;
    #print("count {}",count)
    for x in range (0, count):
        lista = buf[x*4:(x+1)*4]
        data = data + "0x{:02x}{:02x}{:02x}{:02x} ".format(lista[3],lista[2],lista[1],lista[0]);
        lastVal = count;
    if ((len(data)>0)):
        cmd = "{}ncpWrite {}.{}.{}.0x{:010x}".format(pre,event['devNum'],get_node(event['regionId']),\
                get_target(event['regionId']),int(event['offset'])+0x10*add_offset)
        print("{} {}".format(cmd, data))
    data = ""
                


def write_64(pre,event):  # to do - copy from write32
    add_offset = 0;
    buf = event['buffer'];
    count = event['count']
    data = ""
    lastVal = 0;
    #print("count {}",count)
    for x in range (0, count):
        lista = buf[x*8:(x+1)*8]
        data = data + "0x{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x} ".format(lista[7],lista[6],lista[5],lista[4],lista[3],lista[2],lista[1],lista[0]);
        lastVal = count;
    if ((len(data)>0)):
        cmd = "{}ncpWrite -w64 {}.{}.{}.0x{:010x}".format(pre,event['devNum'],get_node(event['regionId']),\
                get_target(event['regionId']),int(event['offset'])+0x10*add_offset)
        print("{} {}".format(cmd, data))
    data = ""





# load traces
for path, subdirs, files in os.walk(".") :
    for file in files :
        if file == "metadata":
            #do something
            trace_collection.add_trace(path, 'ctf')

for event in trace_collection.events:
#    print(event.name);
    if (event.name.startswith("Intel_AXXIA_ncp_common:ncp_poll_entry")):
        print("--- pol begin ----");
    elif (event.name.startswith("Intel_AXXIA_ncp_common:ncp_poll_exit")):
        print("--- pol end ----");    
    elif (event.name.startswith("ncp_xlf_nca_cfg_ncav3_init_pcq_descriptor_")):
        continue
    elif (event.name.startswith("Intel_AXXIA_ncp_common:ncp_reset_entry")):
        print("--- reset begin ----");
    elif (event.name.startswith("Intel_AXXIA_ncp_common:ncp_reset_exit")):
        print("--- reset end ----");
    #elif ((event.name.startswith("Intel_AXXIA_ncp_custom:ncp_custom_hwio")) or event.name.startswith("ncp_custom_hwio_")):
    elif ((event.name.startswith("kubus:netboot_")) or event.name.startswith("netboot_")):
        if (event.name.startswith("netboot_")):
            pre = "kern-"
        else:
            pre = ""
        if (event['hwioType'] == hwio_type.NCP_HWIO_COMMENT.value):
            # comment handing is easy
            if (event['comment'].startswith('#')):
                print("{}".format(event['comment']))
            else:
                print("# {}".format(event['comment']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_READ8.value): #check
            print("{}ncpRead {}.{}.{}.0x{:010x} {}".format(pre,event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],event['count']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_READ16.value): #check
            print("{}ncpRead -w16 {}.{}.{}.0x{:010x} {}".format(pre,event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],event['count']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_READ32.value):
            print("{}ncpRead {}.{}.{}.0x{:010x} {}".format(pre,event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],event['count']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_READ64.value): # check
            print("{}ncpRead -w64 {}.{}.{}.0x{:010x} {}".format(pre,event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],event['count']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_RESET.value):
            print("{}ncpReset -i {}".format(pre,event['devNum']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_WRITE16.value): # todo
            write_16(pre,event)
        elif (event['hwioType'] == hwio_type.NCP_HWIO_WRITE32.value):
            write_32(pre,event)
        elif (event['hwioType'] == hwio_type.NCP_HWIO_WRITE64.value): # todo
            write_64(pre,event)
        elif (event['hwioType'] == hwio_type.NCP_HWIO_POLL.value):
            print("{}ncpPoll -l {} -t {}  {}.{}.{}.0x{:010x} 0x{:08x} 0x{:08x}".format(pre,event['delayLoops'],\
                event['delayTime'],event['devNum'],get_node(event['regionId']),\
                get_target(event['regionId']),event['offset'],event['mask'],event['value']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_USLEEP.value):
            print("{}ncpUsleep {}".format(pre,event['hwioArg']))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_CRRMW.value):
            tmp = event['buffer'][0];
            out1 = tmp & 0xFFFFFFFF
            out2 = tmp >> 32  & 0xFFFFFFFF
            print("{}ncpModify {}.{}.{}.0x{:010x} 0x{:08x} 0x{:08x}".format(pre,event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],out1,out2))
        elif (event['hwioType'] == hwio_type.NCP_HWIO_CRBF.value):
            print("{}ncpFill -c {} {}.{}.{}.0x{:010x} 0x{:08x}".format(pre,event['count'],event['devNum'],get_node(event['regionId']),\
                    get_target(event['regionId']),event['offset'],event['buffer'][0]))
        else:
            print("unhandled type {}".format(event['hwioType']))
    else:
        print("unhandled event {}".format(event.name));
    



