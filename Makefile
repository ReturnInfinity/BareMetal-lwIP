# repu1sion Makefile for reip

LWIPDIR = src
LWIPARCH = reip
NEWLIBDIR = newlib
HTTPDDIR = httpd_raw
CC = gcc
LD = ld

CFLAGS = -c -m64 -nostdlib -nostartfiles -nodefaultlibs -fomit-frame-pointer -mno-red-zone
CFLAGS:=$(CFLAGS) \
	-I$(LWIPDIR)/include -I$(LWIPARCH)/include -I$(LWIPDIR)/include/ipv4 \
	-I$(LWIPDIR)/include/ipv6 -I$(LWIPDIR) -I$(HTTPDDIR) -I$(NEWLIBDIR)/include -I.
LDFLAGS = -T app.ld

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES=$(LWIPDIR)/core/def.c $(LWIPDIR)/core/dhcp.c $(LWIPDIR)/core/dns.c \
	$(LWIPDIR)/core/inet_chksum.c $(LWIPDIR)/core/init.c $(LWIPDIR)/core/mem.c \
	$(LWIPDIR)/core/memp.c $(LWIPDIR)/core/netif.c $(LWIPDIR)/core/pbuf.c \
	$(LWIPDIR)/core/raw.c $(LWIPDIR)/core/stats.c $(LWIPDIR)/core/sys.c \
	$(LWIPDIR)/core/tcp.c $(LWIPDIR)/core/tcp_in.c $(LWIPDIR)/core/tcp_in.c \
	$(LWIPDIR)/core/tcp_out.c $(LWIPDIR)/core/timers.c $(LWIPDIR)/core/udp.c
CORE4FILES=$(LWIPDIR)/core/ipv4/autoip.c $(LWIPDIR)/core/ipv4/icmp.c \
	$(LWIPDIR)/core/ipv4/igmp.c $(LWIPDIR)/core/ipv4/ip_frag.c \
	$(LWIPDIR)/core/ipv4/ip4.c $(LWIPDIR)/core/ipv4/ip4_addr.c
#CORE6FILES=$(LWIPDIR)/core/ipv6/dhcp6.c $(LWIPDIR)/core/ipv6/ethip6.c \
	$(LWIPDIR)/core/ipv6/icmp6.c $(LWIPDIR)/core/ipv6/ip6.c \
	$(LWIPDIR)/core/ipv6/ip6_addr.c $(LWIPDIR)/core/ipv6/ip6_frag.c \
	$(LWIPDIR)/core/ipv6/mld6.c $(LWIPDIR)/core/ipv6/nd6.c

# APIFILES: The files which implement the sequential and socket APIs.
#APIFILES=$(LWIPDIR)/api/api_lib.c $(LWIPDIR)/api/api_msg.c $(LWIPDIR)/api/err.c \
	$(LWIPDIR)/api/netbuf.c $(LWIPDIR)/api/netdb.c $(LWIPDIR)/api/netifapi.c \
	$(LWIPDIR)/api/sockets.c $(LWIPDIR)/api/tcpip.c

# NETIFFILES: Files implementing various generic network interface functions.'
NETIFFILES=$(LWIPDIR)/netif/etharp.c

# ARCHFILES: Architecture specific files.
#ARCHFILES=$(wildcard $(LWIPARCH)/*.c)
ARCHFILES=

# LWIPFILES: All the above.
#LWIPFILES=$(COREFILES) $(CORE4FILES) $(CORE6FILES) $(APIFILES) $(NETIFFILES) $(ARCHFILES)
LWIPFILES=$(COREFILES) $(CORE4FILES) $(NETIFFILES) $(ARCHFILES)
LWIPFILESW=$(wildcard $(LWIPFILES))
LWIPOBJS=$(notdir $(LWIPFILESW:.c=.o))

TARGET = liblwip.a

.PHONY: all
all:
	$(CC) $(CFLAGS) $(LWIPFILES)
	ar -cvq $(TARGET) $(LWIPOBJS)
	$(CC) $(CFLAGS) reip/reip.c reip/renetif.c reip/libBareMetal.c
	$(CC) $(CFLAGS) httpd_raw/fs.c httpd_raw/httpd.c
	$(LD) $(LDFLAGS) -o reip.app newlib/crt0.o reip.o renetif.o fs.o httpd.o libBareMetal.o liblwip.a newlib/libc.a
	mv reip.app httpd.app 

.PHONY: clean
clean:
	rm -f *.o $(TARGET) *.s *.app
