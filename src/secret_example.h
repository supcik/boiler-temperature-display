#ifndef SECRET_H_
#define SECRET_H_

//------------------------------------------------------------------------------
// COPY THIS FILE TO "secret.h" AND CHANGE THE SECRET CONSTANTS in "secret.h"
//------------------------------------------------------------------------------

#include <Arduino.h>

const String kUrl   = "http://...";
const String kTitle = "TITLE";

const char kSSID[]       = "MyWifiSSID";
const char kPassPhrase[] = "MyWifiPassphrase";
const char kMqttServer[] = "server.url.com";
const char kMqttTopic[]  = "topic";

#endif /* SECRET_H_ */
