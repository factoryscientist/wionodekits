// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include <pgmspace.h>
#include <ESP8266WiFi.h>
#include "src/iotc/common/string_buffer.h"
#include "src/iotc/iotc.h"

#define WIFI_SSID "<ENTER WIFI SSID HERE>"
#define WIFI_PASSWORD "<ENTER WIFI PASSWORD HERE>"

const char* SCOPE_ID = "<ENTER SCOPE ID HERE>";
const char* DEVICE_ID = "<ENTER DEVICE ID HERE>";
const char* DEVICE_KEY = "<ENTER DEVICE primary/secondary KEY HERE>";

const uint8_t PORT1B = 5;   //右　黄
const uint8_t PORT_POWER = 15; // (common with RED_LED)
const uint8_t BUZZER_PIN = 5;   //右　黄
const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo);
#include "src/connection.h"

int alert = 0;
unsigned long pretime=0;

int eventCount=0;
//----------------------------------------------------------------------
void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo) {
  char msg[256] = {0};
  int len = 0, errorCode = 0;

  eventCount++;
  LOG_VERBOSE("Event recieved %d",eventCount);

  // ConnectionStatus
  if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0) {
    LOG_VERBOSE("Is connected ? %s (%d)",
                callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO",
                callbackInfo->statusCode);
    isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
    return;
  }

  // payload buffer doesn't have a null ending.
  // add null ending in another buffer before print
  AzureIOT::StringBuffer buffer;
  if (callbackInfo->payloadLength > 0) {
    buffer.initialize(callbackInfo->payload, callbackInfo->payloadLength);
  }

  LOG_VERBOSE("- [%s] event was received. Payload => [%s]\n",
              callbackInfo->eventName, buffer.getLength() ? *buffer : "EMPTY");

  //コマンド
  if (strcmp(callbackInfo->eventName, "Command") == 0) {
    LOG_VERBOSE("- Command name was => [%s]\r\n", callbackInfo->tag);

    if ( 0==strcmp("AlertOn",callbackInfo->tag)){
      //ブザー鳴らす　　アラートＯＮ
      tone(BUZZER_PIN, 440);
      alert = 1;
    } else if  (0==strcmp("AlertOff",callbackInfo->tag)){
      //ブザー鳴らす　　アラートＯＮ
      noTone(BUZZER_PIN);
      alert = 0;
    }
    //テレメトリ更新:  Alert
    len = snprintf(msg, sizeof(msg) - 1, "{\"Alert\":%d}", alert);
    errorCode = iotc_send_telemetry(context, msg, len);
    if (errorCode != 0) {
      LOG_ERROR("Sending telemetry has failed with error code %d", errorCode);
    }
  }

}
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");

  connect_wifi(WIFI_SSID, WIFI_PASSWORD);
  connect_client(SCOPE_ID, DEVICE_ID, DEVICE_KEY);

  if (context != NULL) {
    pretime = 0;  // set timer in the past to enable first telemetry a.s.a.p
  }

  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(BLUE_LED, HIGH);

  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  
  delay(500); 

  // ----------
  alert = 0;

  //send telemetry
  char msg[256] = {0};
  int len = 0, errorCode = 0;
  len = snprintf(msg, sizeof(msg) - 1, "{\"Alert\":%d}", alert);
  errorCode = iotc_send_telemetry(context, msg, len);
  if (errorCode != 0) {
    LOG_ERROR("Sending telemetry has failed with error code %d", errorCode);
  }
  }
//----------------------------------------------------------------------
void loop() {
  char msg[256] = {0};
  int len = 0, errorCode = 0;
  
  if (isConnected) {

    digitalWrite(BLUE_LED, LOW);
    delay(1000);

   //funcボタンが押されたらブザー停止
   if ( (1 == alert) && (LOW == digitalRead(FUNC_BTN)) ){
      noTone(BUZZER_PIN);
      alert = 0;
      //send property
      len = snprintf(msg, sizeof(msg) - 1, "{\"Alert\":%d}", alert);
      errorCode = iotc_send_telemetry(context, msg, len);
      if (errorCode != 0) {
        LOG_ERROR("Sending telemetry has failed with error code %d", errorCode);
      }
    }

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);

    iotc_do_work(context);  // do background work for iotc
    delay(1000);
        
  } else {
    iotc_free_context(context);
    context = NULL;
    connect_client(SCOPE_ID, DEVICE_ID, DEVICE_KEY);
  }
}
