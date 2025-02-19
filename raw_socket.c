#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h> 
#include <netpacket/packet.h> 
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct sInfo{
	char dst[6];
	char src[6];
	unsigned short ptype;
	char magic[4];
	char type;
	char len;
	char value[256];
};

int sock_raw_fd;
struct sockaddr_ll sll;
struct ifreq req;

unsigned char a2x(const char c)
{
	switch(c) {
	case '0'...'9':
		return (unsigned char)atoi(&c);
	case 'a'...'f':
		return 0xa + (c-'a');
	case 'A'...'F':
		return 0xa + (c-'A');
	default:
		goto error;
	}
error:
	exit(0);
}

#define MAC_LEN_IN_BYTE 6   
#define COPY_STR2MAC(mac,str)  \
	do { \
		for(int i = 0; i < MAC_LEN_IN_BYTE; i++) {\
  			mac[i] = (a2x(str[i*3]) << 4) + a2x(str[i*3 + 1]);\
		}\
	} while(0)

struct sInfo test = {.ptype=0x2211, .magic="CIG", .type=1, .len=11, .value="hello,world"};
int *thread1(void * arg)
{
	//00:10:18:1a:60:2c
    while(1){
	int len = sendto(sock_raw_fd, &test, test.len + 22, 0 , (struct sockaddr *)&sll, sizeof(sll));
	if(len == -1)
	{
		perror("sendto");
	}
	sleep(1);
    }
}

int *thread2(void * arg)
{
    while(1){
	unsigned char buf[1024] = {0};
        int len = recvfrom(sock_raw_fd, buf, sizeof(buf), 0, NULL, NULL);
        struct sInfo test;
        if(len > 0){
            memcpy((char*)&test,(char*)buf,len);
            if(strstr(test.magic,"CIG")){
                printf("%s\n",test.value);
            }
        }
    }
}

int main(int argc,char *argv[])
{
	COPY_STR2MAC(test.dst,argv[2]);
	COPY_STR2MAC(test.src,argv[3]);
	sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
 
	strncpy(req.ifr_name, argv[1], IFNAMSIZ);			//指定网卡名称
	if(-1 == ioctl(sock_raw_fd, SIOCGIFINDEX, &req))	//获取网络接口
	{
		perror("ioctl");
		close(sock_raw_fd);
		exit(-1);
	}
	bzero(&sll, sizeof(sll));
	sll.sll_ifindex = req.ifr_ifindex;
	if(-1 == ioctl(sock_raw_fd, SIOCGIFHWADDR, &req))	//获取网络接口
	{
		perror("ioctl");
		close(sock_raw_fd);
		exit(-1);
	}
	unsigned char mac[ETH_ALEN];
	memcpy(mac, req.ifr_hwaddr.sa_data, ETH_ALEN);
	printf("MAC address : %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	pthread_t id,id2;
	pthread_create(&id, NULL, (void *)thread1, NULL);
	pthread_create(&id2, NULL, (void *)thread2, NULL);

 	while(1){
	}
	return 0;
}
