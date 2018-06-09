#include "ns.h"

#include <inc/error.h>
#include <inc/types.h>

#define E1000_RX_PACKET_SIZE 2048

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
    char buf[E1000_RX_PACKET_SIZE];
    int res;
    envid_t whom;
    while (1) {
        if ((res = sys_net_recv(buf)) < 0){
            if (res != -E_NO_RECV_PACKET)
                panic("sys_net_recv: %e", res);
            sys_yield();
            continue;
        }

        while (sys_page_alloc(0, &nsipcbuf, PTE_P | PTE_W | PTE_U) < 0) ;
        nsipcbuf.pkt.jp_len = res;
        memmove(nsipcbuf.pkt.jp_data, buf, res);
        ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P | PTE_U | PTE_W);
    }
}
