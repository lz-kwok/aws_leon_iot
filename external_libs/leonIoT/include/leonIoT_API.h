#ifndef _LEONIOT_API_H_
#define _LEONIOT_API_H_
#if 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//#include <curl/curl.h>

#define MAX_PROPERTY_LEN 		(9)
#define MAX_STRING_PROP_LEN     (32)

#define localPromptPlayer			0x01
#define alertsMediaPlayer			0x02
#define urlResPlayer                0x03

#define Hal_Play        			1
#define Hal_Pause       			2
#define Hal_Resume					3
#define Hal_Stop        			4
#define Hal_Play_Loop 				5
#define Hal_Play_Previous			6
#define Hal_Play_Next				7
#define Hal_Play_FAST_FORWARD		8
#define Hal_Play_REWIND				9

#define BT_MSG_TYPE 				1
#define BT_PLAYER_MSG_TYPE 			2
#define GST_PLAYER_MSG_TYPE			3

typedef union {
    float f_value;
    int i_value;
    char* s_value;
}property_value_t;


typedef struct {
	int type;
	property_value_t value;
	char name[MAX_PROPERTY_LEN];
}property_msg_t;

typedef struct 
{
	float temp;
	float humi;
	int press;
}data_air;



typedef void (*message_free_cb)(void* content);
struct hal_message{
	int 	what;					/*msg type*/
	void* 	content;				/*msg content*/
	message_free_cb freecb; 		/* free content func */
};

enum DEVICE_BIND_STATE{
	DEVICE_BIND_UNKNOWN,
	DEVICE_BIND_SUCCESS,
	DEVICE_BIND_FAILED
};


enum WIFI_STATE{
    WIFI_STATION_UP = 0x01,
    WIFI_STATION_DOWN,
    WIFI_CONNECT_FAILED,
};

enum DEVICE_STATUS_e {
	DEVICE_STATUS_WIFI_CONNECTION = 0x01, 		/* ������������ */
	DEVICE_STATUS_WIFI_CONNECTED = 0x02,  		/* �������ӳɹ� */
	DEVICE_STATUS_MQTT_FAILED = 0x03,   		/* MQTT ����������ʧ�� , �ɹ�֮��Ͽ���ʾʧ��*/
	DEVICE_STATUS_MQTT_CONNECTED = 0x04,  		/* MQTT ���������ӳɹ�*/	
	DEVICE_STATUS_FACTORY_CHECKING = 0x05,  	/* ϵͳ���ڹ����Լ� */
	DEVICE_STATUS_FACTORY_CHECK_OK = 0x06,  	/* ϵͳ�����Լ�ɹ� */
	DEVICE_STATUS_FACTORY_CHECK_FAILED = 0x07, 	/* ϵͳ�����Լ�ʧ�� */
	DEVICE_STATUS_AFTER_SALES = 0x08,     		/* ϵͳ�����ۺ�ģʽnever used */	
	DEVICE_STATUS_CANCEL_BINDED = 0x09,  		/* ϵͳ����ǿ�ƽ���� */
	DEVICE_STATUS_UNBIND_OK = 0x0A,  			/* ǿ�ƽ���󶨳ɹ�*/
	DEVICE_STATUS_UNBIND_FAILED = 0x0B, 		/* ǿ�ƽ����ʧ�� */
};

enum MESSAGE_TYPE{
	MESSAGE_NOTIFY_WIFI = 0x1,		/*  WIFI ״̬��Ϣ*/
	MESSAGE_REPORT_ATTR,			/*  �ϱ����� */
	MESSAGE_FACTORY_CHECK,			/*  ���빤���Լ�ģʽ */
	MESSAGE_RESTORE_FACTORY,		/*  ������������  */
	MESSAGE_FACTORY_BACKDOOR,		/*  ��������ģʽ, �����޷������ɹ������*/
	MESSAGE_FORCED_UNBINDED,		/*  ǿ�ƽ����*/
	MESSAGE_NOTIFY_RESTORE_FACTORY,	/*  ֪ͨ���ػָ����� */
	MESSAGE_REPORT_SUBDEV_ATTR,	 	/* �������豸���� */
	MESSAGE_NOTIFY_SUBDEV_ONOFF, 	/* ���豸����/����״̬ */

	MESSAGE_NOTIFY_IOT_DISCOVER,
	MESSAGE_NOTIFY_IOT_CONTROL,
	MESSAGE_NOTIFY_PLAYER_CONTROL,
	MESSAGE_NOTIFY_WAKEUP,
	MESSAGE_NOTIFY_CTRWORDS,
	MESSAGE_NOTIFY_MUSIC_PLAY,
	MESSAGE_NOTIFY_SSE_TTS,
	MESSAGE_NOTIFY_CLOCKMIN,
	MESSAGE_NOTIFY_EXT_PLAY
};

enum PROPERTY_DATA_TYPE {
	PROPERTY_TYPE_CHAR = 0x80,
	PROPERTY_TYPE_SHORT = 0x81,
	PROPERTY_TYPE_INT =  0x82,
	PROPERTY_TYPE_FLOAT = 0x83,
	PROPERTY_TYPE_STRING = 0x84
};
enum SKYIOT_MESSAGE_Content{
	bd_wakeup = 0x01,

};

typedef struct {
    int     volume;
    bool    is_mute;
} user_volume_t;


typedef enum {
	CMD_ALARM_PLAY = 0x01,
	CMD_SHOW,
	CMD_ADD_TEMPPORARY,
	CMD_DELETE

}SKYIOT_CLOCK_CMD;

typedef enum {
	NONPALYER = 0,
    BLUETOOTH = 1,
    GSTRENMER = 2
}player_type;

typedef struct {
	char uid[64];
	char devid[64];
	char sky_iot_url[256];
}commom_url_para;

typedef enum  {
    PLAYMODE_NORMAL,
    PLAYMODE_LOOP
}PlayMode;


typedef enum  {
    normal_voice = 0,
    sse_voice,
    alarm_voice,
}Tts_Type;


typedef struct 
{
	int tts_type;
    char playsource[256];
	bool has_remind;
}Tts_Msg;

typedef struct 
{
    unsigned char   Sec;    //��
    unsigned char   Min;    //��
    unsigned char   Hour;   //ʱ
    unsigned char   Day;    //��
    unsigned char   Month;  //��
    unsigned char   Week;   //����
    unsigned char   Year;   //��
}tTimeInfo;

typedef struct 
{
    char timerBuff[20];
	tTimeInfo MessageTime;
	char path[40];
}Time_Msg;

typedef enum {
    BT_PLAY = 0,
    BT_STOP,
    BT_PAUSE,
    BT_NEXT,
    BT_PREVIOUS,
    BT_FAST_FORWARD = 5,
    BT_REWIND,
    BT_INIT,
    BT_RELEASE
}avrcp_ctrl_cmd;

typedef enum  {
    /// Represents the highest focus a Channel can have.
    FOREGROUND,

    /// Represents the intermediate level focus a Channel can have.
    BACKGROUND,

    /// This focus is used to represent when a Channel is not being used or when an observer should stop.
    NONE
}btFocusState;


void* HAL_Malloc(int size);

void* HAL_Calloc(int count, int size);

void HAL_Free(void *ptr);

void HAL_Printf(const char *fmt, ...);

void HAL_SleepMs(int ms);

int leonIoT_init();

/* ��ȡIOTģ��״̬*/
int SkyIotDevice_getStatus(void);

/*���ò�Ʒ���ͣ��յ��̶�Ϊ 8*/
int SkyIotDevice_setProductType(unsigned int type);

/*���ÿյ��ͺţ���Ҫ����31���ֽ�*/
int SkyIotDevice_setProductModel(const char* model);

/*���ÿյ�Ʒ�����ͣ���ά�̶�Ϊ1*/
int SkyIotDevice_setProductBrand(unsigned int brand);
#endif

struct hal_message SkyIotDevice_RecAVS_message(void);

void SkyIotDevice_propertyFreecb(property_msg_t* pattr);

int SkyIotDevice_getBindStatus(void);

int SkyIotDevice_getNetStatus(void);

void skyiot_WifiInfo_Copy(const char* ssid, int ssid_len, const char* pwd, int pwd_len);

int currentPlayer();

void SkyIotClock_Setalarm_Num(int num);

int SkyIotClock_Writealarm_paras(const char *alarmfiles,const char *datetime,
									const char *action,const char *repeatTime,const int day);

int SkyIotClock_Writealarm_comconfig(const char *alarmid,const char *speechtext,
									int clocksum);

// CURL *http_post_sse(const char *url);

const char* SkyIotDevice_getDeviceId();

int SkyIotDevice_UpdateProperty(const char* name, int type, property_value_t* value);

int SkyIotDevice_getMQTTBindStatus(void);

data_air GetAirData();

#ifdef __cplusplus
}
#endif

#endif
