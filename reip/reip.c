/*
   Poll ethernet driver to process received frames.
   The driver will pass them up to the lwIP stack using the input function
   you specified in netif_add.
   Periodically invoke the lwIP timer processing functions (etharp_tmr(),
   ip_reass_tmr(), tcp_tmr(), etc.
   the exact list depends on which protocols you have enabled).
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "libBareMetal.h"

#include "lwip/inet.h"
#include "lwip/tcp.h"
#include "lwip/ip_frag.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "netif/etharp.h"
#include "renetif.h"

#define RE_IPADDR_BASE "192.168.242.2"
#define RE_IPADDR_NETMASK "255.255.255.0"
#define RE_IPADDR_GATEWAY "192.168.242.1"

#define MS_PER_TICK 125

struct ip_addr re_ip_addr;
/* Generic data structure used for all lwIP network interfaces. */
static struct netif redrv_netif;

/* ----- PART NEEDED BY LWIP LIBRARY ----- */
static unsigned long start_time; /* save time from system start here */

/* returns current time in ms. Needed for lwip internal timeouts */
u32_t sys_now(void)
{
	unsigned long msec = b_system_config(0,0) * MS_PER_TICK - start_time;
	//printf("msec : %lu \n", msec);

	return msec;
}

void sys_init(void)
{
	printf("sys_init() called!\n");
	start_time = b_system_config(0,0) * MS_PER_TICK;
	//gettimeofday(&starttime, NULL);
}
/* ----- END OF PART NEEDED BY LWIP LIBRARY ----- */

int reip_init(int n_args, char **args)
{
	struct ip_addr gw_addr, netmask;
	void * redrvnet_priv = NULL;

	if (n_args == 4)
	{
		printf("4 args: %s , %s , %s \n", args[1], args[2], args[3]);

		if (!inet_aton(args[1], &re_ip_addr))
			return 1;
		if (!inet_aton(args[2], &netmask))
			return 1;
		if (!inet_aton(args[3], &gw_addr))
			return 1;
	}
	else
	{
		if (!inet_aton(RE_IPADDR_BASE, &re_ip_addr))
			return 1;
		if (!inet_aton(RE_IPADDR_NETMASK, &netmask))
			return 1;
		if (!inet_aton(RE_IPADDR_GATEWAY, &gw_addr))
			return 1;
	}

	/* Initialize LWIP */
	lwip_init();

	/* Initialize httpd_raw */
	httpd_init();

	/* Add a network interface to the list of lwIP netifs */
	if (!netif_add(&redrv_netif, &re_ip_addr, &netmask, &gw_addr,
			 NULL, reif_init, ethernet_input))
	{
		printf("netif_add failed\n");
		return 1;
	}
	netif_set_default(&redrv_netif);
	netif_set_up(&redrv_netif);

	printf("Interface is %s \n", netif_is_up(&redrv_netif) ? "UP" : "DOWN");

	return 0;
}

// Regular polling mechanism.  This should be called each time through
// the main application loop (after each interrupt, regardless of source).
// It handles any received packets, permits NIC device driver house-keeping
// and invokes timer-based TCP/IP functions (TCP retransmissions, delayed
// acks, IP reassembly timeouts, ARP timeouts, etc.)
void reip_poll(void)
{
	char packet[1500] = {0};
	int len;

	len = b_ethernet_rx(packet, 0);
	if (len)
	{
		reif_input(&redrv_netif, packet, len);
	}

	sys_check_timeouts();
}

int main(int argc, char **argv)
{
	printf("reip started v1.00\n");

	if (reip_init(argc, argv))
	{
		printf("reip_init() failed\n");
		return 1;
	}
	while (1) 
	{
		reip_poll();
	}
}
