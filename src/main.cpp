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
#include "IBMPlexSansSemiBold40pt8b.h"
#include "IBMPlexSansBold18pt8b.h"
#include "IBMPlexSansRegular18pt8b.h"
#include "secret.h"

const int kConnectTimeout = 10000;           // milliseconds
const int kConnectDelay   = 1000;            // milliseconds
const int kCenterX        = TFT_HEIGHT / 2;  // Note : TFT_HEIGHT is the width

// Global variables
WiFiClient net;
MQTTClient client;

struct data {
    String timestamp;
    float consumption;
    float temp;
};

void NewLine(int& pos, float k = 0.9) { pos += M5.Lcd.fontHeight() * k; }

void displayWelcome()
{
    int bgColor = BLUE;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 20;
    M5.lcd.drawCentreString("Mesures de", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.lcd.drawCentreString("l'eau chaude", kCenterX, yPos, 1);
    NewLine(yPos, 1.2);
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kTitle, kCenterX, yPos, 1);
    NewLine(yPos, 1.2);
    yPos += 5;
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.lcd.drawCentreString("Démarrage...", kCenterX, yPos, 1);
    NewLine(yPos);
    delay(5000);
}

void displayWifiConnectionError()
{
    int bgColor = RED;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 25;
    M5.lcd.drawCentreString("Je ne peux pas me", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.lcd.drawCentreString("connecter au Wifi", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kSSID, kCenterX, yPos, 1);
    NewLine(yPos);
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    yPos += 10;
    M5.lcd.drawCentreString(kPassPhrase, kCenterX, yPos, 1);
    NewLine(yPos);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
}

void displayMqttConnectionError()
{
    int bgColor = MAROON;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 25;
    M5.lcd.drawCentreString("Je ne peux pas me", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.lcd.drawCentreString("connecter au", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.lcd.drawCentreString("serveur", kCenterX, yPos, 1);
    NewLine(yPos);
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    yPos += 10;
    M5.lcd.drawCentreString(kMqttServer, kCenterX, yPos, 1);
    NewLine(yPos);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
}

void displayValues(data& d)
{
    int bgColor;
    int fgColorTemp;
    int fgColorConsumption = WHITE;

    if (d.temp > 60) {
        fgColorTemp = RED;
        bgColor     = BLACK;
    } else {
        fgColorTemp = CYAN;
        bgColor     = BLACK;
    }

    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    M5.lcd.drawCentreString(d.timestamp, kCenterX, 5, 1);

    char text[16];
    int yPos = 60;

    sprintf(text, "%4.1f°C", d.temp);
    String s = String(text);
    s.trim();
    M5.Lcd.setTextColor(fgColorTemp, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_SemiBold40pt8b);
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
    NewLine(yPos);

    sprintf(text, "%4.0f l", d.consumption);
    s = String(text);
    s.trim();
    M5.Lcd.setTextColor(fgColorConsumption, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_SemiBold40pt8b);
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
}

void messageReceived(String& topic, String& payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
    int p1 = payload.indexOf(";");
    int p2 = payload.indexOf(";", p1 + 1);
    int p3 = payload.indexOf(";", p2 + 1);
    if (p3 < 0) {
        p3 = payload.length();
    }
    data d = {
        .timestamp   = payload.substring(0, p1),
        .consumption = payload.substring(p1 + 1, p2).toFloat(),
        .temp        = payload.substring(p2 + 1, p3).toFloat(),
    };
    displayValues(d);
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
    M5.update();
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
        connect();
    }
}
