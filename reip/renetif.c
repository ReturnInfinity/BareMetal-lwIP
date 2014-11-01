/* Ethernet driver for lwip. netif/ethernetif.c was taken as sample */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
//#include "lwip/ethip6.h"
#include "netif/etharp.h"
#include "renetif.h"

#define ETH_PKT_SIZE 1536

#define IFNAME0 'e'
#define IFNAME1 'n'

/* for private data */
struct reif_s
{
	char* drv_ver;
	struct eth_addr *ethaddr;
};

static struct stats_s
{
	unsigned int tx;
	unsigned int rx;
} stats;

/* TX ETH packet. b_ethernet_tx() called inside.
   We store the whole chain of pbufs in one buffer and send it 
   with b_ethernet_tx() then. This is important. */ 
err_t reif_ll_output(struct netif *netif, struct pbuf *p)
{
	int i;
	struct pbuf *q;
	char packet[ETH_PKT_SIZE] = {0};
	int used = 0; /* used space in the buffer */

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	for(i = 0, q = p; q != NULL; q = q->next) 
	{
		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		if (q->payload && q->len)
		{
			printf("pbuf %d . len: %d used: %d \n", i++, q->len, used);
			memcpy(packet + used, q->payload, q->len);
			used += q->len;
			if (used > ETH_PKT_SIZE)
			{
				printf("<TX ERROR: buffer overflow during copying pbufs into TX buffer. used: %d\n",
					used);
				return ERR_BUF;
			}
		}
	}
	b_ethernet_tx(packet, used);
	printf("TX. len: %d total: %u\n", used, stats.tx++);

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf* reif_ll_input(struct netif *netif, char *pkt, int pkt_len)
{
	struct reif_s *reif = netif->state;
	struct pbuf *p, *q;
	u16_t len;

	/* Obtain the size of the packet and put it into the "len" variable. */
	len = (u16_t)pkt_len;

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
	if (p)
	{
#if ETH_PAD_SIZE
		pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
		/* We iterate over the pbuf chain until we have read the entire
		* packet into the pbuf. */
		for(q = p; q != NULL; q = q->next) 
		{
		/* Read enough bytes to fill this pbuf in the chain. The
		* available data in the pbuf is given by the q->len
		* variable.
		* This does not necessarily have to be a memcpy, you can also preallocate
		* pbufs for a DMA-enabled MAC and after receiving truncate it to the
		* actually received size. In this case, ensure the tot_len member of the
		* pbuf is the sum of the chained pbuf len members.
		*/
			memcpy(q->payload, pkt, q->len);
			stats.rx++;
			printf("RX. len: %d total: %u\n", q->len, stats.rx);
		}
#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	LINK_STATS_INC(link.recv);
	} 
	else 
	{
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
	}
	return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void reif_input(struct netif *netif, char *pkt, int len)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;
	struct reif_s *reif = netif->state;

	/* move received packet into a new pbuf */
	p = reif_ll_input(netif, pkt, len);
	/* no packet could be read, silently ignore this */
	if (p == NULL) 
		return;
	/* points to packet payload, which starts with an Ethernet header */
	ethhdr = p->payload;

	switch (htons(ethhdr->type)) 
	{
		/* IP or ARP packet? */
		case ETHTYPE_IP:
		//case ETHTYPE_IPV6:
		case ETHTYPE_ARP:
			/* full packet send to tcpip_thread to process */
			if (netif->input(p, netif) != ERR_OK)
			{ 
				LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
				pbuf_free(p);
				p = NULL;
			}
		break;
		default:
			pbuf_free(p);
			p = NULL;
		break;
	}
}

static void reif_ll_init(struct netif *netif)
{
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	unsigned long mac;
	mac = b_system_config(30, 0);
	printf("mac is: 0x%lX \n", mac);

	/* set MAC address. XXX: this hardcode should be removed! */
	netif->hwaddr[0] = 0x08;
	netif->hwaddr[1] = 0x00;
	netif->hwaddr[2] = 0x27;
	netif->hwaddr[3] = 0x51;
	netif->hwaddr[4] = 0x28;
	netif->hwaddr[5] = 0x16;

	/* MTU */
	netif->mtu = ETH_PKT_SIZE;

	/* device capabilities */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
}

/* init func, passed to netif_add() */
err_t reif_init(struct netif *netif)
{
	struct reif_s *reif;
	memset(&stats, 0x0, sizeof(struct stats_s));

	if (netif)
		printf("reif drv init is ok\n");

	reif = mem_malloc(sizeof(struct reif_s));
	if (!reif)
	{
		printf("out of memory\n");
		return ERR_MEM;
	}

	netif->state = reif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;

	/* LWIP IP module -> TX IP packet  
	   Resolve and fill-in Ethernet address header for outgoing IP packet.*/
	netif->output = etharp_output;
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif 
	/* TX ETH packet. b_ethernet_tx() called inside reif_ll_output() */
	netif->linkoutput = reif_ll_output;

	reif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

	/* initialize the lowlevel*/
	reif_ll_init(netif);

	return ERR_OK;
}
