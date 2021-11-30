// Copyright 2021 Jacques Supcik <jacques@supcik.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file main.cpp
 * @author Jacques Supcik <jacques@supcik.net>
 *
 * @brief Water Consumption and temperature Display
 *
 * @date 2021-11-10
 * @version 0.1.0
 ***************************************************************************/

#include <Arduino.h>
#include <M5Stack.h>
#include <MQTT.h>
#include <Wifi.h>

#include "IBMPlexMonoRegular9pt8b.h"
#include "IBMPlexSansBold18pt8b.h"
#include "IBMPlexSansRegular18pt8b.h"
#include "IBMPlexSansSemiBold40pt8b.h"
#include "global.h"
#include "screens.h"
#include "secret.h"

const int kConnectTimeout = 10000;  // milliseconds
const int kConnectDelay   = 1000;   // milliseconds

// Global variables
WiFiClient net;
MQTTClient client;

String getField(String& text)
{
    int pos = text.indexOf(";");
    if (pos < 0) {
        pos = text.length();
    }
    String result = text.substring(0, pos);
    text          = text.substring(pos + 1);
    return result;
}

void messageReceived(String& topic, String& payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
    String field = getField(payload);
    if (field.length() > 0) gData.timestamp = field;
    field = getField(payload);
    if (field.length() > 0) gData.consumption = field.toFloat();
    field = getField(payload);
    if (field.length() > 0) gData.temp = field.toFloat();
    field = getField(payload);
    if (field.length() > 0) gData.electricityConsumption = field.toFloat();
    field = getField(payload);
    if (field.length() > 0) gData.electricityProduction = field.toFloat();
    displayValues();
}

void connect()
{
    bool errorDisplayed = false;
    unsigned long now   = millis();
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        if (millis() - now > kConnectTimeout) {
            if (!errorDisplayed) {
                displayWifiConnectionError();
                errorDisplayed = true;
            }
        }
        delay(kConnectDelay);
    }

    errorDisplayed = false;
    now            = millis();
    Serial.print("\nconnecting...");
    while (!client.connect(("meter:" + WiFi.macAddress()).c_str())) {
        Serial.print(".");
        if (millis() - now > kConnectTimeout) {
            if (!errorDisplayed) {
                displayMqttConnectionError();
                errorDisplayed = true;
            }
        }
        delay(kConnectDelay);
    }

    Serial.println("\nconnected!");
    client.subscribe(kMqttTopic);
}

void setup()
{
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setBrightness(kMaxBrightness);
    M5.Lcd.fillScreen(BLACK);
    Serial.begin(115200);
    displayWelcome();
    WiFi.begin(kSSID, kPassPhrase);
    client.begin(kMqttServer, net);
    client.onMessage(messageReceived);
    connect();
}

void loop()
{
    static unsigned long lastPressed = millis();
    unsigned long now = millis();
    M5.update();
    bool anyPressed = false;

    if (M5.BtnA.wasPressed()) {
        gScreenNo  = 0;
        anyPressed = true;
    }
    if (M5.BtnB.wasPressed()) {
        gScreenNo  = 1;
        anyPressed = true;
    }
    if (M5.BtnC.wasPressed()) {
        gScreenNo  = 2;
        anyPressed = true;
    }

    if (anyPressed) {
        displayValues();
        M5.Lcd.setBrightness(kMaxBrightness);
        lastPressed = now;
    }

    if (now - lastPressed > kScreenTimeout) {
        M5.Lcd.setBrightness(kDimBrightness);
    }


    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
        connect();
    }
}
