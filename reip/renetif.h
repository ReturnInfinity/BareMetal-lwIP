#ifndef RENETIF_H
#define RENETIF_H

void reif_input(struct netif *netif, char *pkt, int len);
err_t reif_init(struct netif *netif);

#endif
