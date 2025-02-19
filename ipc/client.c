#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MSG_RX 0xfff0 //recv osp tx msg
//#define MSG_RX 0xfff1 // recv server tx msg

static int msgid_rx;

struct msgbuf_t{
	int mtype;		
	char text[512];
};

struct msgbuf_t buf;

enum MSGQ
{
    START_ADV = 1,
    SET_DEVICE_NAME,
    SET_ADV_DATA,
    SET_SCAN_RSP_DATA,
    SET_ADV_PARA,
    SET_CONNTECTABLE,
    SET_PASSKEY,
    SEND_JSON_CONF,
    CONN_STATUS,
    DISCONN_STATUS,
    PAIRING_STATUS
};

void print_json(char *text)
{
    printf("json rcv: ");
    for(int i=0; i<strlen(text); i++)
    {
        printf("%c ",text[i]);
    }
    printf("\n");
}


void print_hex(char *text)
{
    printf("hex rcv: ");
    for(int i=0; i<strlen(text); i++)
    {
        printf("%x ",text[i]);
    }
    printf("\n");
}


int osp_handle_msg_queue()
{
	int ret = 0;
	while(1)
	{
		memset(&buf,0,sizeof(struct msgbuf_t));
		ret = msgrcv(msgid_rx, &buf, sizeof(buf.text), buf.mtype, IPC_NOWAIT);
       		if(buf.mtype == 0){
	   		return 0;
		}
		printf("recv type: %d\n",buf.mtype);
		switch(buf.mtype){
    			case CONN_STATUS: printf("device connect\n"); print_hex(buf.text); break;
    			case DISCONN_STATUS: printf("device disconnect\n"); break;
    			case PAIRING_STATUS: printf("device paired\n"); break;
    			case SEND_JSON_CONF: printf("device get json\n"); print_json(buf.text);break;
    			default:break;
  		}
	}

	return 0;	
}

int main()
{
	int k;
	int ret = -1;
	ret = msgget(MSG_RX,0666|IPC_CREAT);
	if(ret < 0)
		printf("create msg error\n");
	msgid_rx = ret;
	while(1)
	{
		osp_handle_msg_queue();
	}
	return 0;
}

