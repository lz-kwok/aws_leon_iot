/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file shadow_sample.c
 * @brief A simple connected window example demonstrating the use of Thing Shadow
 */
#include <unistd.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <net/if.h>
#include <linux/sockios.h>

#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "cJSON.h"
#include "leonIoT_API.h"


typedef struct {
	int  mqtt_status; 					/* AWS Platform  Status */
	
	Thread_t thr_sync_proc;
	property_node_t proplist[MAX_PROPERTY_NUMS]; /*used for property iteator */	
	
	AWS_IoT_Client mqttClient;
}AWS_Manager;
static AWS_Manager aManager;

static int aws_syncproc_main(void* param);


int send_type = 0;



#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

static char certDirectory[PATH_MAX + 1] = "../../certs";
#define HOST_ADDRESS_SIZE 255
static char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;
static uint32_t port = AWS_IOT_MQTT_PORT;
static uint8_t numPubs = 5;



void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
								const char *pReceivedJsonDocument, void *pContextData) {
	IOT_UNUSED(pThingName);
	IOT_UNUSED(action);
	IOT_UNUSED(pReceivedJsonDocument);
	IOT_UNUSED(pContextData);

	if(SHADOW_ACK_TIMEOUT == status) {
		IOT_INFO("Update Timeout--");
	} else if(SHADOW_ACK_REJECTED == status) {
		IOT_INFO("Update RejectedXX");
	} else if(SHADOW_ACK_ACCEPTED == status) {
		IOT_INFO("Update Accepted !!");
	}
}

void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
	IOT_UNUSED(pJsonString);
	IOT_UNUSED(JsonStringDataLen);

	if(pContext != NULL) {
		IOT_INFO("Delta - pressureShow state changed to %d", *(bool *) (pContext->pData));
		if(*(bool *) (pContext->pData) == true){
			send_type = 0;
		}else{
			send_type = 1;
		}
	}
}



int main(int argc, char **argv) {
	static data_air AirData;
	memset(&aManager,0x0,sizeof(aManager));
	leonIoT_init();
	IoT_Error_t rc = FAILURE;
	int32_t i = 0;

	char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);
	char *pJsonStringToUpdate;
	float temperature = 0.0;
	float humi = 0.0;
	int press = 0;

	bool windowOpen = false;
	jsonStruct_t windowActuator;
	windowActuator.cb = windowActuate_Callback;
	windowActuator.pData = &windowOpen;
	windowActuator.dataLength = sizeof(bool);
	windowActuator.pKey = "pressureShow";
	windowActuator.type = SHADOW_JSON_BOOL;

	jsonStruct_t temperatureHandler;
	temperatureHandler.cb = NULL;
	temperatureHandler.pKey = "temperature";
	temperatureHandler.pData = &temperature;
	temperatureHandler.dataLength = sizeof(float);
	temperatureHandler.type = SHADOW_JSON_FLOAT;

	jsonStruct_t humiHandler;
	humiHandler.cb = NULL;
	humiHandler.pKey = "humidity";
	humiHandler.pData = &humi;
	humiHandler.dataLength = sizeof(float);
	humiHandler.type = SHADOW_JSON_FLOAT;

	jsonStruct_t pressureHandler;
	pressureHandler.cb = NULL;
	pressureHandler.pKey = "pressure";
	pressureHandler.pData = &press;
	pressureHandler.dataLength = sizeof(int);
	pressureHandler.type = SHADOW_JSON_UINT16;

	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];

	IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	getcwd(CurrentWD, sizeof(CurrentWD));
	snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
	snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
	snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

	IOT_DEBUG("rootCA %s", rootCA);
	IOT_DEBUG("clientCRT %s", clientCRT);
	IOT_DEBUG("clientKey %s", clientKey);

	// initialize the mqtt client

	ShadowInitParameters_t sp = ShadowInitParametersDefault;
	sp.pHost = AWS_IOT_MQTT_HOST;
	sp.port = AWS_IOT_MQTT_PORT;
	sp.pClientCRT = clientCRT;
	sp.pClientKey = clientKey;
	sp.pRootCA = rootCA;
	sp.enableAutoReconnect = false;
	sp.disconnectHandler = NULL;

	IOT_INFO("Shadow Init");
	rc = aws_iot_shadow_init(&aManager.mqttClient, &sp);
	if(SUCCESS != rc) {
		IOT_ERROR("Shadow Connection Error");
		return rc;
	}

	ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
	scp.pMyThingName = AWS_IOT_MY_THING_NAME;
	scp.pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
	scp.mqttClientIdLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);

	IOT_INFO("Shadow Connect");
	rc = aws_iot_shadow_connect(&aManager.mqttClient, &scp);
	if(SUCCESS != rc) {
		IOT_ERROR("Shadow Connection Error");
		return rc;
	}

	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_shadow_set_autoreconnect_status(&aManager.mqttClient, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	rc = aws_iot_shadow_register_delta(&aManager.mqttClient, &windowActuator);

	if(SUCCESS != rc) {
		IOT_ERROR("Shadow Register Delta Error");
	}
	aManager.mqtt_status = AWS_STATUS_MQTT_CONNECTION;
	aManager.thr_sync_proc = hal_ThreadCreate(aws_syncproc_main, NULL, 0x1000);
	if (aManager.thr_sync_proc == NULL) {
		IOT_ERROR("%s thread create failed",__func__);
		return -1;
	}

	// loop and publish a change in temperature
	while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
		rc = aws_iot_shadow_yield(&aManager.mqttClient, 200);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			sleep(1);
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
		aManager.mqtt_status = AWS_STATUS_MQTT_CONNECTED;
		IOT_INFO("\n=======================================================================================\n");
		IOT_INFO("On Device: pressureShow state %s", windowOpen ? "true" : "false");
		
		// simulateRoomTemperature(&temperature);
		AirData = GetAirData();
		temperature = AirData.temp;
		humi = AirData.humi;
		press = AirData.press;

		rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
		if(SUCCESS == rc) {
			if(send_type == 0){
				rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 3, &temperatureHandler,
											 &humiHandler,&windowActuator);
			}else{
				rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &pressureHandler,
											 &windowActuator);
			}
			
			if(SUCCESS == rc) {
				rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
				if(SUCCESS == rc) {
					IOT_INFO("Update Shadow: %s", JsonDocumentBuffer);
					rc = aws_iot_shadow_update(&aManager.mqttClient, AWS_IOT_MY_THING_NAME, JsonDocumentBuffer,
											   ShadowUpdateStatusCallback, NULL, 4, true);
				}
			}
		}
		IOT_INFO("*****************************************************************************************\n");
		sleep(1);
	}

	if(SUCCESS != rc) {
		IOT_ERROR("An error occurred in the loop %d", rc);
	}

	IOT_INFO("Disconnecting");
	rc = aws_iot_shadow_disconnect(&aManager.mqttClient);
	aManager.mqtt_status = AWS_STATUS_MQTT_DISCONNECTED;

	if(SUCCESS != rc) {
		IOT_ERROR("Disconnect error %d", rc);
	}

	return rc;
}


static int aws_syncproc_main(void* param)
{
	while (1)
	{
		HAL_SleepMs(1000);
		if(aManager.mqtt_status == AWS_STATUS_MQTT_CONNECTED){
			
		}
	}
	
}
