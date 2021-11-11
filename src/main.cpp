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
 * @brief Boiler Temperature Display
 *
 * @date 2021-11-10
 * @version 0.1.0
 ***************************************************************************/

#include <Arduino.h>
#include <HTTPClient.h>
#include <M5Stack.h>
#include <WiFi.h>

#include "IBMPlexMonoRegular9pt8b.h"
#include "IBMPlexMonoSemiBold40pt8b.h"
#include "IBMPlexSansBold18pt8b.h"
#include "IBMPlexSansRegular18pt8b.h"
#include "secret.h"

const int kConnectRetry = 30;
const int kConnectDelay = 500;      // milliseconds
const int kPollPeriod   = 15 * 60;  // in seconds
const int kCenterX      = 160;

// Global variables

float curTemp;
String curDate         = "----.--.--";
String curTime         = "--:--:--";
int valueScreenBgColor = BLACK;
bool isError           = false;

String extractField(String s, String key)
{
    // search key
    int pos = s.indexOf("id=\"" + key + "\"");
    if (pos < 0) return "";
    // search end of tag
    pos = s.indexOf(">", pos);
    if (pos < 0) return "";
    // search next tag
    int end = s.indexOf("<", pos + 1);
    if (end < 0) return "";
    return s.substring(pos + 1, end);
}

void displayStart()
{
    int bgColor = BLUE;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 20;
    M5.lcd.drawCentreString("Température", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.lcd.drawCentreString("du boiler", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kTitle, kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    yPos += 5;
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.lcd.drawCentreString("Démarrage...", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    isError = false;
    delay(5000);
}

void displayUpdate()
{
    if (isError) {
        int bgColor = PURPLE;
        M5.Lcd.fillScreen(bgColor);
        M5.Lcd.setTextColor(WHITE, bgColor);
        M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
        int yPos = 70;
        M5.lcd.drawCentreString("Connexion et", kCenterX, yPos, 1);
        yPos += M5.Lcd.fontHeight();
        M5.lcd.drawCentreString("mise à jour...", kCenterX, yPos, 1);
        yPos += M5.Lcd.fontHeight();
    } else {
        M5.Lcd.setTextColor(WHITE, valueScreenBgColor);
        M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
        int yPos = 180;
        M5.lcd.drawCentreString("Mise à jour...", kCenterX, yPos, 1);
        yPos += M5.Lcd.fontHeight();
    }
}

void displayConnexionError()
{
    int bgColor = RED;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 25;
    M5.lcd.drawCentreString("Impossible de se", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.lcd.drawCentreString("connecter au Wifi", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kSSID, kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    yPos += 7;
    M5.lcd.drawCentreString(kPassPhrase, kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    isError = true;
}

void connect()
{
    if (WiFi.status() == WL_CONNECTED) return;
    WiFi.begin(kSSID.c_str(), kPassPhrase.c_str());
    for (int i = 0; i < kConnectRetry; i++) {
        if (WiFi.status() == WL_CONNECTED) return;
        delay(kConnectDelay);
    }
    displayConnexionError();
}

void displayValues()
{
    int fgColor;
    if (curTemp > 60) {
        fgColor            = RED;
        valueScreenBgColor = BLACK;
    } else {
        fgColor            = CYAN;
        valueScreenBgColor = BLACK;
    }

    M5.Lcd.fillScreen(valueScreenBgColor);
    M5.Lcd.setTextColor(WHITE, valueScreenBgColor);
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    M5.lcd.drawString(curDate, 5, 5, 1);
    M5.lcd.drawRightString(curTime, 315, 5, 1);

    char text[8];
    sprintf(text, "%4.1f°C", curTemp);
    M5.Lcd.setTextColor(fgColor, valueScreenBgColor);
    M5.Lcd.setFreeFont(&IBMPlexMono_SemiBold40pt8b);
    M5.lcd.drawCentreString(text, kCenterX, 70, 1);
    isError = false;
}

void updateValues()
{
    curTemp = NAN;
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(kUrl.c_str());
        int res = http.GET();
        if (res > 0) {
            String payload = http.getString();
            curDate        = extractField(payload, "dateValue");
            curTime        = extractField(payload, "timeValue").substring(0, 8);
            String s       = extractField(payload, "analogOutTemp");
            if (s != "") {
                curTemp = s.toFloat();
            }
        } else {
            Serial.printf("HTTP GET failed, error: %s\n",
                          http.errorToString(res).c_str());
        }
        http.end();
    } else {
        Serial.println("Not connected");
    }
}

void setup()
{
    M5.begin();
    M5.Power.begin();
    M5.Lcd.fillScreen(BLACK);
    Serial.begin(115200);
    displayStart();
}

void loop()
{
    static unsigned long lastUpdate = 0;
    M5.update();
    unsigned long now = millis();
    if (lastUpdate == 0 || M5.BtnA.isPressed() ||
        (now - lastUpdate > kPollPeriod * 1000)) {
        if (M5.BtnA.isPressed()) {
            displayUpdate();
        }
        connect();
        if (WiFi.status() == WL_CONNECTED) {
            updateValues();
            displayValues();
            WiFi.disconnect();
        }
        lastUpdate = now;
    }
    delay(20);
}