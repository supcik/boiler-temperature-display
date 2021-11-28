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

#include "screens.h"

#include <Arduino.h>
#include <M5Stack.h>
#include <Wifi.h>

#include "IBMPlexMonoRegular9pt8b.h"
#include "IBMPlexSansBold18pt8b.h"
#include "IBMPlexSansRegular18pt8b.h"
#include "IBMPlexSansRegular24pt8b.h"
#include "IBMPlexSansSemiBold32pt8b.h"
#include "IBMPlexSansSemiBold40pt8b.h"
#include "global.h"
#include "secret.h"

void displayWelcome()
{
    int bgColor = BLUE;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular24pt8b);
    int yPos = 10;
    M5.lcd.drawCentreString("Maison", kCenterX, yPos, 1);
    NewLine(yPos, 0.8);
    M5.lcd.drawCentreString("intelligente", kCenterX, yPos, 1);
    NewLine(yPos, 1);
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kTitle, kCenterX, yPos, 1);
    NewLine(yPos, 1);
    yPos += 5;
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.lcd.drawCentreString("Démarrage...", kCenterX, yPos, 1);
    NewLine(yPos);
    delay(5000);
}

void displayInfo()
{
    int bgColor = NAVY;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular24pt8b);
    int yPos = 10;
    M5.lcd.drawCentreString("Maison", kCenterX, yPos, 1);
    NewLine(yPos, 0.8);
    M5.lcd.drawCentreString("intelligente", kCenterX, yPos, 1);
    NewLine(yPos, 1);
    M5.Lcd.setFreeFont(&IBMPlexSans_Bold18pt8b);
    M5.lcd.drawCentreString(kTitle, kCenterX, yPos, 1);
    NewLine(yPos, 1);
    yPos += 5;
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.lcd.drawCentreString("Version " + kVersion, kCenterX, yPos, 1);
}

void displayWifiConnectionError()
{
    int bgColor = RED;
    M5.Lcd.fillScreen(bgColor);
    M5.Lcd.setTextColor(WHITE);
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
    M5.Lcd.setTextColor(WHITE);
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

static void displayTimeStamp()
{
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setFreeFont(&IBMPlexMono_Regular9pt8b);
    M5.lcd.drawCentreString(gData.timestamp, kCenterX, 4, 1);
}

static void displayValues1()
{
    int bgColor;
    int fgColorTemp;
    int fgColorConsumption = WHITE;

    if (gData.temp > 60) {
        fgColorTemp = RED;
        bgColor     = BLACK;
    } else {
        fgColorTemp = CYAN;
        bgColor     = BLACK;
    }

    M5.Lcd.fillScreen(bgColor);
    displayTimeStamp();

    char text[16];
    int yPos = 32;
    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.Lcd.setTextColor(RED);
    M5.lcd.drawCentreString("Eau chaude", kCenterX, yPos, 1);
    NewLine(yPos, 0.95);

    sprintf(text, "%4.1f°C", gData.temp);
    String s = String(text);
    s.trim();
    M5.Lcd.setTextColor(fgColorTemp);
    M5.Lcd.setFreeFont(&IBMPlexSans_SemiBold40pt8b);
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
    NewLine(yPos, 0.72);

    sprintf(text, "%4.0f l", gData.consumption);
    s = String(text);
    s.trim();
    M5.Lcd.setTextColor(fgColorConsumption);
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
}

static void displayValues2()
{
    int bgColor = BLACK;
    int fgColor = YELLOW;

    M5.Lcd.fillScreen(bgColor);
    displayTimeStamp();

    char text[16];
    int yPos = 32;

    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.Lcd.setTextColor(fgColor);
    M5.lcd.drawCentreString("Consommation", kCenterX, yPos, 1);
    NewLine(yPos, 0.8);

    M5.Lcd.setFreeFont(&IBMPlexSans_SemiBold32pt8b);
    sprintf(text, "%5.0fW", gData.electricityConsumption);
    String s = String(text);
    s.trim();
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
    NewLine(yPos, 0.8);

    M5.Lcd.setFreeFont(&IBMPlexSans_Regular18pt8b);
    M5.Lcd.setTextColor(fgColor);
    M5.lcd.drawCentreString("Production", kCenterX, yPos, 1);
    NewLine(yPos, 0.8);

    M5.Lcd.setFreeFont(&IBMPlexSans_SemiBold32pt8b);
    sprintf(text, "%5.0fW", gData.electricityProduction);
    s = String(text);
    s.trim();
    M5.lcd.drawCentreString(s.c_str(), kCenterX, yPos, 1);
}

static void displayValues3() {
    displayInfo();
}


void displayValues()
{
    switch (gScreenNo) {
        case 0:
            displayValues1();
            break;
        case 1:
            displayValues2();
            break;
        case 2:
            displayValues3();
            break;
    }
    M5.Lcd.fillRect(
        160 + 93 * (gScreenNo - 1) - 30, TFT_WIDTH - 6, 60, 6, DARKGREY);
}
