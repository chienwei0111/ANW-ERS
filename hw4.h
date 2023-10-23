#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

struct icmp_packet{
	struct ip ip;
	struct icmphdr icmp;
};
void print_usage();
unsigned short checksum(void *b, int len);
