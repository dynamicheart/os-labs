#include "ns.h"

#include <inc/error.h>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
    int res;
    envid_t whom;

    while (1) {
        if ((res = ipc_recv(&whom, &nsipcbuf, NULL)) < 0)
            panic("ipc_recv: %e", res);
        if (whom != ns_envid) {
            cprintf("NS OUTPUT: output thread got IPC message from env %x not NS\n", whom);
            continue;
        }
        if (res != NSREQ_OUTPUT) {
            cprintf("NS OUTPUT: output thread did not get IPC message of type NSREQ_OUTPUT\n");
            continue;
        }

        while ((res = sys_net_try_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0){
            if (res != -E_TRANSMIT_QUEUE_FULL)
                panic("sys_net_try_transmit: %e", res);
        }
    }
}
