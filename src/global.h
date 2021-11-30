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

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <Arduino.h>
#include <M5Stack.h>

const int kCenterX       = TFT_HEIGHT / 2;  // Note : TFT_HEIGHT is the width
const int kMaxBrightness = 255;
const int kDimBrightness = 1;
const int kScreenTimeout = 10000;  // milliseconds
const String kVersion    = "0.1.1";

struct data {
    String timestamp;
    float consumption;
    float temp;
    float electricityConsumption;
    float electricityProduction;
};

extern data gData;
extern int gScreenNo;

void NewLine(int& pos, float k = 0.9);

#endif /* GLOBAL_H_ */
