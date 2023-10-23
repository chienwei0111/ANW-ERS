#include "hw4.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
void print_buffer(const unsigned char *bufer,int lingth){
        for (int i =0; i<lingth; i++){
                printf("%02x ", bufer[i]);
	}

        printf("\n");
}

int main(int argc, char* argv[])
{
	int sockfd_recv=0,sockfd_send=0; 
	char buffer[1024];
	//Check identity & Print Usage & Create socket
	uid_t uid = geteuid();
	if( uid != 0)
	{
		perror("Error: You are not a root user");
		exit(1);
	}
	if (argc < 3 || !strcmp(argv[1],"--help") || !strcmp(argv[1],"-h"))
	{
		print_usage();
		exit(0);
	}
	if((sockfd_send = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0)
	{
		perror("socket creation failed!");
		exit(1);
	}
	if((sockfd_recv = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0)
	{
		perror("socket creation failed!");
		exit(1);
	}

	// Send ICMP packet with incremental TTL.
	int max_hop = atoi(argv[1]);
	for( int i = 1; i < max_hop+1 ; i++ )
	{	
		struct sockaddr_in sa;
		struct icmphdr icmp;
		socklen_t addr_len = sizeof(sa);
		sa.sin_family = AF_INET;
                sa.sin_addr.s_addr = inet_addr(argv[2]);
		if(setsockopt(sockfd_send, IPPROTO_IP, IP_TTL, &i, sizeof(i)))
		{
			perror("setsockopt fail");
			exit(1);
		}

		icmp.type = ICMP_ECHO;
		icmp.code = 0;
		icmp.un.echo.id = htons(1+i);
		icmp.un.echo.sequence = htons(i);
		icmp.checksum = 0;
		icmp.checksum = checksum(&icmp,sizeof(icmp));
		if(sendto(sockfd_send,&icmp, sizeof(icmp),0,(struct sockaddr*)&sa,addr_len)<0)
		{
			perror("send error!");
			exit(1);
		}
		int numbytes_recv = recv(sockfd_recv, buffer, sizeof(buffer), 0);
		//print_buffer(buffer,numbytes_recv);
		if(numbytes_recv <= 0)
		{
			perror("receive error!");
			exit(1);
		}
		struct icmp_packet* pkt = (struct icmp_packet*)buffer;
		unsigned char type = pkt->icmp.type;
		struct in_addr in;
		in = pkt->ip.ip_src;
		char *src_ip = inet_ntoa(in);
		if( type == 11 )
		{
			printf("ICMP Packet Received From Ring %d (%s)\n",i,src_ip);
		}
		else if ( type == 0 )
		{
			printf("Destination Reached (%s)\n",src_ip);
			exit(0);
		}
		else
		{
			printf("Exception occurs.\n");
			exit(1);
		}
		//printf("%s",src_ip);
	}
		
	close(sockfd_send);	
	close(sockfd_recv);
	return 0;	




}
