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
#include <Regexp.h>
#include <WiFi.h>

#include "IBMPlexMonoSemiBold40pt8b.h"
#include "IBMPlexSansBold18pt8b.h"
#include "IBMPlexSansRegular18pt8b.h"
#include "secret.h"

const int kConnectRetry = 10;
const int kConnectDelay = 500;      // milliseconds
const int kPollPeriod   = 15 * 60;  // in seconds
const int kCenterX      = 160;

void displayStart()
{
    const auto bgColor = BLUE;
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
    delay(5000);
}

void displayUpdate()
{
    M5.Lcd.setTextColor(PURPLE, BLACK);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 180;
    M5.lcd.drawCentreString("Mise à jour...", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
}

void displayConnexionError()
{
    const auto bgColor = RED;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE, bgColor);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    int yPos = 30;
    M5.lcd.drawCentreString("Impossible de se", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.lcd.drawCentreString("connecter au Wifi", kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kSSID, kCenterX, yPos, 1);
    yPos += M5.Lcd.fontHeight();
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
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

void displayTemp(float t)
{
    static float old_t = -1;
    if (t == old_t) return;
    char text[8];
    sprintf(text, "%4.1f°C", t);
    if (t > 60) {
        M5.Lcd.setTextColor(RED, BLACK);
    } else {
        M5.Lcd.setTextColor(CYAN, BLACK);
    }
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setFreeFont(&IBMPlexMono_SemiBold40pt8b);
    M5.lcd.drawCentreString(text, kCenterX, 70, 1);
    old_t = t;
}

float extractTemperature(String s)
{
    // search key
    int pos = s.indexOf("id=\"analogOutTemp\"");
    if (pos < 0) return NAN;
    // search end of tag
    pos = s.indexOf(">", pos);
    if (pos < 0) return NAN;
    // search next tag
    int end = s.indexOf("<", pos + 1);
    if (end < 0) return NAN;
    // search first digit
    while (pos < end && !((isdigit(s.charAt(pos))) || s.charAt(pos) == '-')) {
        pos++;
    }
    if (pos >= end) return NAN;
    return s.substring(pos, end).toFloat();
}

float getTemp()
{
    float result = NAN;
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String payload;
        http.begin(kUrl.c_str());
        Serial.println(kUrl);
        int res = http.GET();
        if (res > 0) {
            result = extractTemperature(http.getString());
        } else {
            Serial.printf("HTTP GET failed, error: %s\n",
                          http.errorToString(res).c_str());
        }
        http.end();
    } else {
        Serial.println("Not connected");
    }
    return result;
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
        displayTemp(getTemp());
        WiFi.disconnect();
        lastUpdate = now;
    }
    delay(20);
}