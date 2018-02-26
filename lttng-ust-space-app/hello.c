#include <stdio.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_PROBE_DYNAMIC_LINKAGE
#include "netboot-tp.h"

typedef enum                             
{                                        
    NCP_HWIO_READ32 = 0,                 
    NCP_HWIO_WRITE32,                    
    NCP_HWIO_FILL32,                     
    NCP_HWIO_CRBF,                       
    NCP_HWIO_CRRMW,                      
    NCP_HWIO_CRSW,                       
    NCP_HWIO_CRBBW,                      
    NCP_HWIO_CRBSW,                      
    NCP_HWIO_CRBBF,                      
    NCP_HWIO_CSMBR,                      
    NCP_HWIO_CSMBW,                      
    NCP_HWIO_USLEEP,                     
    NCP_HWIO_RESET,                      
    NCP_HWIO_POLL,                       
    NCP_HWIO_POST_RESET,                 
    NCP_HWIO_COMMENT,                    
    NCP_HWIO_TRANS_START,                
    NCP_HWIO_TRANS_UDELAY,               
    NCP_HWIO_TRANS_STALL_2ND_PASS_POLL,  
    NCP_HWIO_TRANS_END,                  
    NCP_HWIO_READ16,                     
    NCP_HWIO_WRITE16,                    
    NCP_HWIO_READ8,                      
    NCP_HWIO_WRITE8,                     
    NCP_HWIO_READ64,                     
    NCP_HWIO_WRITE64                     
} ncp_hwio_type_t;                       


int main(int argc, char *argv[])
{

	unsigned char value = 0x12;
	tracepoint(kubus,
           netboot_rw,    
           NCP_HWIO_READ8,        
           0/*devNum*/,                
           11/*regionId*/,              
           0x40000,                
           (unsigned char *) &value, 
           sizeof(unsigned char),   
           1, /* count */         
           0,                     
           0 /* flags */          
          );                      

    return 0;
}
