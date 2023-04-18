/*******************************************************************************
  Copyright(c) 2023 Radek Kaczorek  <rkaczorek AT gmail DOT com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.
 .
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 .
 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include "indi2mqtt.h"

#include <basedevice.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include <unistd.h>
#include <mosquitto.h> // https://mosquitto.org/api/files/mosquitto-h.html
#include <signal.h>
#include "config.h"


int main(int, char *[])
{
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);

    Indi2Mqtt indi2mqtt;
    indi2mqtt.setServer("localhost", 7624);

    //indi2mqtt.setBLOBMode(B_ALSO, "Simple CCD", nullptr);
    //indi2mqtt.enableDirectBlobAccess("Simple CCD", nullptr);

	mosquitto_lib_init();
	sprintf(mqtt_clientid, "indi2mqtt-%d", getpid());
	mosq = mosquitto_new(mqtt_clientid, mqtt_clean_session, 0);

	if (mosq)
	{
		mosquitto_connect_callback_set(mosq, mqttConnectCallback);
		mosquitto_disconnect_callback_set(mosq, mqttDisconnectCallback);
		mosquitto_subscribe_callback_set(mosq, mqttSubscribeCallback);
		mosquitto_message_callback_set(mosq, mqttMsgCallback);

		mosquitto_username_pw_set(mosq, MQTT_USER, MQTT_PASS);

		int rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, MQTT_KEEP_ALIVE);
		
		//mosquitto_subscribe(mosq, NULL, "environment/pws-b5f0/temperature", 0);


		// ADD mqtt will

		// publish immediate status
		indi2mqtt.mqttINDIStatus(false);
		
		while (true)
		{
			//mosquitto_reconnect(mosq);
			mosquitto_loop(mosq, -1, 1);

			// indi reconnection loop
			if (!indi2mqtt.isServerConnected())
			{
				while (!indi2mqtt.isServerConnected())
				{
					indi2mqtt.connectServer();
					sleep(3);
				}
			}
			usleep(1000000);
		}
	}
}

/**************************************************************************************
**
***************************************************************************************/
Indi2Mqtt::Indi2Mqtt()
{
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newDevice (INDI::BaseDevice dp)
{
	IDLog("-> %s\n", dp->getDeviceName());
	
	//uint16_t deviceType = dp->getDriverInterface();	
	//IDLog("%s\n", getDeviceType(deviceType));
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::removeDevice (INDI::BaseDevice dp)
{
	IDLog("<- %s\n", dp->getDeviceName());
	return;
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newProperty(INDI::Property property)
{
	char topic[1024];
	char msg[128];
	//IDLog("%s/%s\n", property->getDeviceName(), property->getName());

	if (property->getType() == INDI_SWITCH)
	{
		ISwitchVectorProperty *svp = property->getSwitch();
		for (int i=0; i < svp->nsp; i++)
		{
			//IDLog("%s/%s/%s:\t%s\n", svp->device, svp->name, svp->sp[i].name, sstateStr(svp->sp[i].s));
			sprintf(topic, "%s/%s/%s", svp->device, svp->name, svp->sp[i].name);
			sprintf(msg, "%s", sstateStr(svp->sp[i].s));
			mqttPublish(topic, msg);
		}
	}

	if (property->getType() == INDI_NUMBER)
	{
		INumberVectorProperty *nvp = property->getNumber();
		for (int i=0; i < nvp->nnp; i++)
		{
			//IDLog("%s/%s/%s:\t%4.2f\n", nvp->device, nvp->name, nvp->np[i].name, nvp->np[i].value);
			sprintf(topic, "%s/%s/%s", nvp->device, nvp->name, nvp->np[i].name);
			sprintf(msg, "%4.2f", nvp->np[i].value);
			mqttPublish(topic, msg);
		}
	}

	if (property->getType() == INDI_TEXT)
	{
		ITextVectorProperty *tvp = property->getText();
		for (int i=0; i < tvp->ntp; i++)
		{
			//IDLog("%s/%s/%s:\t%s\n", tvp->device, tvp->name, tvp->tp[i].name, tvp->tp[i].text);
			sprintf(topic, "%s/%s/%s", tvp->device, tvp->name, tvp->tp[i].name);
			sprintf(msg, "%s", tvp->tp[i].text);
			mqttPublish(topic, msg);
		}
	}

	if (property->getType() == INDI_LIGHT)
	{
		ILightVectorProperty *lvp = property->getLight();
		for (int i=0; i < lvp->nlp; i++)
		{
			//IDLog("%s/%s/%s:\t%i\n", lvp->device, lvp->name, lvp->lp[i].name, lvp->lp[i].s);
			sprintf(topic, "%s/%s/%s", lvp->device, lvp->name, lvp->lp[i].name);
			sprintf(msg, "%i", lvp->lp[i].s);
			mqttPublish(topic, msg);
		}
	}
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::removeProperty(INDI::Property property)
{
	newProperty(property);
}


/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::updateProperty (INDI::Property property)
{
	newProperty(property);
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newSwitch(ISwitchVectorProperty * svp)
{
	char topic[1024];
	char msg[128];
	for (int i=0; i < svp->nsp; i++)
	{
		sprintf(topic, "%s/%s/%s", svp->device, svp->name, svp->sp[i].name);
		sprintf(msg, "%s", sstateStr(svp->sp[i].s));
		mqttPublish(topic, msg);
	}
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newNumber(INumberVectorProperty * nvp)
{
	char topic[1024];
	char msg[128];
	for (int i=0; i < nvp->nnp; i++)
	{
		sprintf(topic, "%s/%s/%s", nvp->device, nvp->name, nvp->np[i].name);
		sprintf(msg, "%4.2f", nvp->np[i].value);
		mqttPublish(topic, msg);
	}
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newText(ITextVectorProperty * tvp)
{
	char topic[1024];
	char msg[128];
	for (int i=0; i < tvp->ntp; i++)
	{
		sprintf(topic, "%s/%s/%s", tvp->device, tvp->name, tvp->tp[i].name);
		sprintf(msg, "%s", tvp->tp[i].text);
		mqttPublish(topic, msg);
	}
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newLight(ILightVectorProperty * lvp)
{
	char topic[1024];
	char msg[128];
	for (int i=0; i < lvp->nlp; i++)
	{
		sprintf(topic, "%s/%s/%s", lvp->device, lvp->name, lvp->lp[i].name);
		sprintf(msg, "%i", lvp->lp[i].s);
		mqttPublish(topic, msg);
	}	
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newBLOB(IBLOB * dp)
{
	/* nop */
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::newMessage(INDI::BaseDevice dp, int messageID)
{
	IDLog("%s\n", dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
char* Indi2Mqtt::getDeviceType(uint16_t deviceType)
{
	char* strDevice;
	switch (deviceType) {
		case 0:
			strDevice =  (char*) "GENERAL";
			break;
		case (1 << 0):
			strDevice =  (char*) "TELESCOPE";
			break;
		case (1 << 1):
			strDevice =  (char*) "CCD";
			break;
		case (1 << 2):
			strDevice =  (char*) "GUIDER";
			break;
		case (1 << 3):
			strDevice =  (char*) "FOCUSER";
			break;
		case (1 << 4):
			strDevice =  (char*) "FILTER";
			break;
		case (1 << 5):
			strDevice =  (char*) "DOME";
			break;
		case (1 << 6):
			strDevice =  (char*) "GPS";
			break;
		case (1 << 7):
			strDevice =  (char*) "WEATHER";
			break;
		case (1 << 8):
			strDevice =  (char*) "AO";
			break;
		case (1 << 9):
			strDevice =  (char*) "DUSTCAP";
			break;
		case (1 << 10):
			strDevice =  (char*) "LIGHTBOX";
			break;
		case (1 << 11):
			strDevice =  (char*) "DETECTOR";
			break;
		case (1 << 12):
			strDevice =  (char*) "ROTATOR";
			break;
		case (1 << 13):
			strDevice =  (char*) "SPECTROGRAPH";
			break;
		case (1 << 14):
			strDevice =  (char*) "CORRELATOR";
			break;
		case (1 << 15):
			strDevice =  (char*) "AUX";
			break;
		default:
			strDevice =  (char*) "AUX";
	}
	return strDevice;
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::serverConnected()
{
	mqttINDIStatus(true);
	IDLog("INDI Server Connected\n");
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::serverDisconnected(int exit_code)
{
	disconnectServer(); // keeps master loop going between reconnects
	mqttINDIStatus(false);
	IDLog("INDI Server Disconnected\n");
}

/**************************************************************************************
**
***************************************************************************************/
void handleSignal(int s)
{
	switch (s) {
		case (SIGINT):
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(0);
			break;
		case (SIGTERM):
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(0);
			break;
		default:
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(0);
	}
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::mqttPublish(char topic[1024], char msg[128])
{
	// publish mqtt message
	char topic_normal[1024];
    sprintf(topic_normal, "%s/%s", MQTT_ROOT_TOPIC, sanitizeTopic(topic));

	//IDLog("%s: %s\n", topic_normal, msg);
	mosquitto_publish(mosq, NULL, topic_normal, strlen(msg), msg, 0, 0);
}

/**************************************************************************************
**
***************************************************************************************/
char * Indi2Mqtt::sanitizeTopic (char topic[1024])
{
	// replace invalid characters with _ & convert all to lower case
	for (int i=0; topic[i] != '\0'; ++i)
	{
		if ( (topic[i] >= 'a' && topic[i]<='z') || (topic[i] >= 'A' && topic[i]<='Z') || (topic[i] >= '/' && topic[i] <= '9') || topic[i] == '-' )
		{
			topic[i] = tolower(topic[i]);
		} else {
			topic[i] = '_';
		}
	}
	return topic;
}

/**************************************************************************************
**
***************************************************************************************/
void Indi2Mqtt::mqttINDIStatus(bool connected)
{
	char status[16];
	strcpy(status, "status");

	if (connected) {
		mqttPublish(status, "connected");
	} else {
		mqttPublish(status, "disconnected");
	}
}

/**************************************************************************************
**
***************************************************************************************/
void mqttConnectCallback(struct mosquitto *mosq, void *obj, int result)
{
	IDLog("Connected to MQTT broker\n");
	//printf("connect callback, rc=%d\n", result);
}

/**************************************************************************************
**
***************************************************************************************/
void mqttDisconnectCallback(struct mosquitto *mosq, void *obj, int result)
{
	IDLog("Disconnected from MQTT broker\n");
	//printf("disconnect callback, rc=%d\n", result);
}

/**************************************************************************************
**
***************************************************************************************/
void mqttSubscribeCallback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	IDLog("Subscribed to MQTT topic\n");
	//printf("subscribe callback, rc=%d\n", mid);
}

/**************************************************************************************
**
***************************************************************************************/
void mqttMsgCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	IDLog("Received MQTT message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
	//printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	mosquitto_topic_matches_sub("environment/pws-b5f0/temperature", message->topic, &match);
	if (match) {
		IDLog("Received temperature from pws-b5f0\n");
		//printf("got temperature from pws-b5f0\n");
	}
}
