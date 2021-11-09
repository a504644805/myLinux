#include <arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>/*for uint16_t*/
#include<stdio.h>
#include<unistd.h>
#include<net/if.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
struct lldpdu
{
	char data[60];

};
 

int main(int argc,char* argv[])
{
	struct lldpdu* lpacket = malloc(sizeof(struct lldpdu));

	memset(lpacket,0,sizeof(struct lldpdu));

	struct sockaddr_ll sll;

	memset(&sll,0,sizeof(struct sockaddr_ll));

	int sk = socket(PF_PACKET, SOCK_RAW, htons(0x88a4));/*使用pf_packet接口创建套接字*/

	sll.sll_family = PF_PACKET;
 
	sll.sll_ifindex = if_nametoindex("enp1s0");/*sll虽然是用来指定目的地址的，但是在这个结构体中sll_ifindex 却指定的是本机发送报文的接口索引*/

	sll.sll_protocol = htons(0x88a4); 
	
	sll.sll_addr[0] = 0xff;/*sll_addr指定目的MAC地址*/
	sll.sll_addr[1] = 0xff;
	sll.sll_addr[2] = 0xff;
	sll.sll_addr[3] = 0xff;
	sll.sll_addr[4] = 0xff;
	sll.sll_addr[5] = 0xff;


	lpacket->data[0]=0xff;
	lpacket->data[1]=0xff;
	lpacket->data[2]=0xff;
	lpacket->data[3]=0xff;
	lpacket->data[4]=0xff;
	lpacket->data[5]=0xff;

	lpacket->data[6]=0x01;
	lpacket->data[7]=0x01;
	lpacket->data[8]=0x01;
	lpacket->data[9]=0x01;
	lpacket->data[10]=0x01;
	lpacket->data[11]=0x01;

	lpacket->data[12]=0x88;
	lpacket->data[13]=0xa4;
	
	lpacket->data[14]=0x0e;
	lpacket->data[15]=0x10;
	lpacket->data[16]=0x08;
	lpacket->data[17]=0x00;
	lpacket->data[18]=0x00;
	lpacket->data[19]=0x00;
	lpacket->data[20]=0x20;
	lpacket->data[21]=0x01;
	lpacket->data[22]=0x02;
	lpacket->data[23]=0x00;
	lpacket->data[24]=0x00;
	lpacket->data[25]=0x00;
	lpacket->data[26]=0x11;

	while(1){ 

		int rtval;
		if((rtval=sendto(sk, lpacket, 60, 0, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll)))==-1){
                //if((rtval=send(sk, lpacket, 60, 0))==-1){
			printf("err,error no is %d: %s \n",errno,strerror(errno));
		}	
 		else{
			printf("send %d byes\n",rtval);

		}

		sleep(1);
	}

}

