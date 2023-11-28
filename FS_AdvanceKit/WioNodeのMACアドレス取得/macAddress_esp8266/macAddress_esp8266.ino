/*
FS講座で使用する WioNode の MAC アドレスを取得するプログラムです。
このプログラムを書き込んでリセットボタンを押すとシリアルモニタに MAC アドレスが表示されます。
*/

#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);  

  delay(10);
  Serial.println();
  Serial.print("ESP8266 MAC: ");
  Serial.println(WiFi.macAddress());

}

void loop() {

}
