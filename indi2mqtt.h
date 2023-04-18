/*******************************************************************************
  Copyright(c) 2021 Radek Kaczorek  <rkaczorek AT gmail DOT com>

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

#pragma once

#include "baseclient.h"
#include <basedevice.h>

#define CONFIG_FILE "/etc/astroberry-mqtt.conf"
#define INDI_HOST "localhost"
#define INDI_PORT 7624
#define MQTT_HOST "nasa.local"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_ROOT_TOPIC "indiserver"

class Indi2Mqtt : public INDI::BaseClient
{
    public:
        Indi2Mqtt();
        ~Indi2Mqtt() = default;

    protected:
		void newDevice (INDI::BaseDevice dp) override;
		void removeDevice (INDI::BaseDevice dp) override;
		void newProperty(INDI::Property property) override;
		void removeProperty(INDI::Property property) override;
		void newSwitch(ISwitchVectorProperty * svp);
		void newNumber(INumberVectorProperty * nvp);
		void newText(ITextVectorProperty * tvp);
		void newLight(ILightVectorProperty * lvp);
		void newBLOB(IBLOB * bp);
        void newMessage(INDI::BaseDevice dp, int messageID) override;
        void updateProperty (INDI::Property property) override;
		void serverConnected() override;
		void serverDisconnected(int exit_code) override;
		
		char* getDeviceType(uint16_t deviceType);
		char * sanitizeTopic (char topic[1024]);
		void mqttPublish(char topic[1024], char msg[128]);
		void mqttINDIStatus(bool connected);

    private:
        INDI::BaseDevice mSimpleCCD;
};
