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

#include "global.h"

#include <Arduino.h>
#include <M5Stack.h>

void NewLine(int& pos, float k) { pos += M5.Lcd.fontHeight() * k; }

data gData;
int gScreenNo = 0;
