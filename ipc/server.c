#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#define MSG_ID 0xfff1
#define MSG_ID_2 0xfff0

struct msgbuf_t{
	int type;		
	char text[512];
};

enum MSGQ
{
    START_ADV = 1,
    SET_DEVICE_NAME,
    SET_ADV_DATA,
    SET_SCAN_RSP_DATA,
    SET_ADV_PARA,
    SET_CONNTECTABLE,
    SET_PASSKEY,
    GET_JSON_CONF
};

char *msg2str[] = {
        "START_ADV",
        "SET_DEVICE_NAME",
        "SET_ADV_DATA",
        "SET_SCAN_RSP_DATA",
        "SET_ADV_PARA",
        "SET_CONNTECTABLE",
        "SET_PASSKEY",
        "GET_JSON_CONF"
};


static struct msgbuf_t my_msg_buf;
static int msgid_1 = 0;
static int msgid_2 = 0;

struct adv_data{
        /** Length of this data in range [0, 31] */
        uint8_t len;
        /** Raw advertisement payload (max. 31 bytes for legacy advertising) */
        uint8_t data[31];
};

typedef struct __attribute__((__packed__)) {
    /** Advertise as connectable and accept connection requests from BLE central device */
    bool connectable;
    /** Connection security level (encryption and authentication) - currently ignored, pairing is always enforced */
    uint8_t security;
    /** Whether to perform bonding (save pairing information) - currently ignored (nowhere to save the keys) */
    bool bondable;
} cmd_set_connectable_t;

//0x03 03 71 FE 1A FF 08 00 00 04 5C 00 31 48 47 32 32 34 33 30 30 30 37 30 00 00 00 00 00 00 00
char mdata[]= {0x03,0x03,0x71,0xFE,0x1A,0xFF,0x08,0x00,0x00,0x04,0x5C,0x00,0x31,0x48,0x47,0x32,0x32,0x34,0x33,0x30,0x30,0x30,0x37,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int send_with_ack(int msgid_tx, int msgid_rx ,int type)
{
    int ret = 0;
    memset(&my_msg_buf, 0, sizeof(struct msgbuf_t));
    my_msg_buf.type = type;
    ret = msgsnd(msgid_tx,(void *)&my_msg_buf,0,0);
    if(ret < 0)
	 return -1;
    memset(&my_msg_buf, 0, sizeof(struct msgbuf_t));
    ret = msgrcv(msgid_rx, &my_msg_buf, sizeof(my_msg_buf.text), my_msg_buf.type, 0);
    if(ret < 0)
         return -1;
    printf("get json\n");
    for(int i=0; i<300; i++)
    {
	printf("%c",my_msg_buf.text[i]);
    }
    printf("\n");
    return 0;
}

int send_with_noack(int msgid, int type, char *buf, int size)
{
    int ret = 0;
    memset(&my_msg_buf, 0, sizeof(struct msgbuf_t));
    my_msg_buf.type = type;
    if(buf != NULL){
        memcpy(&my_msg_buf.text,buf,size);
    }
    printf("type: %d %s\n",my_msg_buf.type, msg2str[type-1]);
    printf("server send : ");
    for(int i=0; i < size; i++)
    {
        printf("%x ",my_msg_buf.text[i]);
    }
    printf("\n");

    ret = msgsnd(msgid,(void *)&my_msg_buf,size,0);
    if(ret < 0){
        printf("send msg error %d\n",ret);
        return ret;
    }
    return 0;
}


char *device_name = "my_pod";

typedef struct __attribute__ ((packed)) {
    // Flags - added by the Bluetooth Stack code
    //uint8_t len_flags;
   // uint8_t ad_type_flags;
   // uint8_t flags; // = LE General Discoverable Mode | BR/EDR Not Supported

    /* Complete List of 16-bit Service Class UUIDs */
    uint8_t len_uuid;
    uint8_t ad_type_uuid;
    uint16_t service_uuid;

    uint8_t len_manufacturer;
    uint8_t ad_type_manufacturer;
    /* region 22 bytes of manufacturer specific data */
    uint16_t company_id;
    uint8_t version;
    char serial_num[12]; /*< Serial number is not required to be null-terminated */
    uint8_t msg_type;
    struct __attribute__ ((packed)) {
        uint8_t status;
        uint8_t _rfu[1];
        /** Random token used in pairing passkey generation */
        uint8_t pairing_token[4];
    } msg;
    /* endregion 22 bytes of manufacturer specific data */
} ble_advertising_data_t;

typedef struct __attribute__((__packed__)) {
    /**
     * 6-digit (0-999999) passkey for Passkey Entry Pairing (this value needs to be entered on central device)
     *
     * If passkey is set to 0 then pairing will be rejected.
     */
    uint32_t pairing_passkey;
} cmd_set_pairing_passkey_t;

#define CONFIG_BLEM_GATT_SERVICE_UUID 0xFE71
#define CONFIG_BLEM_MANUFACTURER_DATA_COMPANY_ID 0x0A17
static ble_advertising_data_t g_adv_data = {
   // .len_flags =      2,
   // .ad_type_flags =  0x01,
   // .flags =          (1 << 1)|(1 << 2),
    .len_uuid = 3u,
    .ad_type_uuid = 0x03u,
    .service_uuid = CONFIG_BLEM_GATT_SERVICE_UUID,
    .len_manufacturer = 0x17,
    .ad_type_manufacturer = 0xFFu,
    .company_id = CONFIG_BLEM_MANUFACTURER_DATA_COMPANY_ID,
    .serial_num = "123456789123",
    .version = 5u,
    .msg = {.status=0, ._rfu[0]=0x11, .pairing_token={1,2,3,4}},
};


int main(int argc, char **argv)
{
	int i = 0;
	struct adv_data adv;
        cmd_set_connectable_t conn;

	memset(&conn,0,sizeof(conn));
	if(argv[2] != NULL){
		conn.connectable = atoi(argv[2]) == 0 ? false:true;
	}
	cmd_set_pairing_passkey_t passkey;
	passkey.pairing_passkey = 433321;
	if(argv[1] == NULL)
		return 0;
	int type = atoi(argv[1]);
	int ret = -1;
	ret = msgget(MSG_ID,0666|IPC_CREAT);
	if(ret < 0)
		printf("create msg error\n");
	msgid_1 = ret;

	ret = msgget(MSG_ID_2,0666|IPC_CREAT);
        if(ret < 0)
                printf("create msg error\n");
        msgid_2 = ret;

	char adv_payload[31];
    	adv_payload[0] = 0x02;
   	adv_payload[1] = 1;
    	adv_payload[2] = 0x06;
    	memcpy(adv_payload + 3 , (char*)&g_adv_data, sizeof(g_adv_data));

    	// data + 3 byte header
    	adv.len = sizeof(g_adv_data) + 3;
	printf("adv len %d\n",adv.len);
    	// copy adv data to app_config
   	memcpy(adv.data, adv_payload, adv.len);

	switch(type){
	    case START_ADV: ret = send_with_noack(msgid_1, type , NULL, 1);break;
	    case SET_DEVICE_NAME:ret = send_with_noack(msgid_1, type ,device_name,strlen(device_name));break;
	   // case SET_ADV_DATA: ret = send_with_noack(msgid_1, type ,(char*)&adv,sizeof(struct adv_data));break;
	    case SET_ADV_DATA: ret = send_with_noack(msgid_1, type ,(char*)&adv,adv.len);break;
	    //case SET_SCAN_RSP_DATA:send_with_noack(msgid_1, type ,);break;
	    //case SET_ADV_PARA:send_with_noack(msgid_1, type ,);break;
	    case SET_CONNTECTABLE: ret = send_with_noack(msgid_1, type ,(char*)&conn,sizeof(cmd_set_connectable_t));break;
	    case SET_PASSKEY: ret = send_with_noack(msgid_1, type ,(char*)&passkey,sizeof(cmd_set_pairing_passkey_t));break;
            case GET_JSON_CONF:ret = send_with_ack(msgid_1,msgid_2,GET_JSON_CONF);break;
	}
	if(ret < 0)
		printf("send msg error %d\n",ret);
#if 0
	while(1)
	{
		i++;
		ret = msgsnd(id,(void *)&buf_send,strlen(buf_send.ctext),0);
		buf_send.mtype = i;
		if(ret < 0)
			printf("send msg error %d\n",ret);
		sleep(1);
	}
#endif
	return 0;
}

