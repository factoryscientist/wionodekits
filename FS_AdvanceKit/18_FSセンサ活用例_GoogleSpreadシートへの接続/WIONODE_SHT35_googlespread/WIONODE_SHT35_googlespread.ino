#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"

ClosedCube_SHT31D sht3xd;
int fail_count = 0;

//Wio Nodeのポート設定
const uint8_t PORT0A = 1;
const uint8_t PORT0B = 3;
const uint8_t PORT1A = 4;
const uint8_t PORT1B = 5;
const uint8_t PORT_POWER = 15; // (common with RED_LED)
//set PORT_POWER as HIGH for power supply. Low means no power.
const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

const uint8_t UART_TX = PORT0A;
const uint8_t UART_RX = PORT0B;
const uint8_t I2C_SDA = PORT1A;
const uint8_t I2C_SCL = PORT1B;

// wifi設定
const char* ssid = "ssid";
const char* password = "pass";

//Googleスプレッドシートの設定
const char *GScriptId = "GoogleAppScriptのID";
const char* host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";
const String payload_base =  "{\"command\": \"appendRow\", \
                    \"sheet_name\": \"Sheet1\", \
                    \"values\": ";
String payload = "";
HTTPSRedirect* client = nullptr;


void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();

  //WioNode IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5);

  // ----------
  // SHT35の初期化
  Wire.begin(I2C_SDA, I2C_SCL);
  //sht3xd.begin(0x44); // Seeed製：Grove SHT31 モジュールを使いたい場合に有効
  sht3xd.begin(0x45); // Seeed製：Grove SHT35 モジュールを使いたい場合に有効

  // SHT35をソフトウェアリセット
  SHT31D_ErrorCode resultSoft = sht3xd.softReset();
  if (resultSoft != SHT3XD_NO_ERROR) {
    Serial.print("[SHT3X]Error code: ");
    Serial.println(resultSoft);
    while (1);
  }
  delay(5);

  //WIFI接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  static int error_count = 0;
  bool flag;
  
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  
  if (!flag) {
    delete client;
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("DEEP SLEEPING");
    ESP.deepSleep( 0 );
    delay(1000);
  }

  //Googleへ接続成功した場合のメッセージ
  Serial.print("Successfully Connected to ");
  Serial.println(host);

  // センサ値の取得および出力
  digitalWrite(BLUE_LED, LOW);
  
  // ClockStrech Modeで計測
  SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
  if (result.error != SHT3XD_NO_ERROR) {
    Serial.print("[SHT3X]Error code: ");
    Serial.println(result.error);
    return;
  }
  //計測値をシリアル表示
  Serial.print("Temp: ");
  Serial.print(result.t, 4);
  Serial.println("*C");
  Serial.print("Humi: ");
  Serial.print(result.rh, 4);
  Serial.println("%");

  //Googleへ送信する値設定
  float TEMP = result.t;
  float HUMI = result.rh;
  // Send memory data to Google Sheets
  payload = payload_base + "\"" + TEMP + "," + HUMI + "\"}";

  if (client->POST(url, host, payload)) {
    Serial.println("Success! send data");
  }
  else {
    error_count++;
    Serial.print("Error!");
    Serial.println(error_count);
  }
  if (error_count > 3) {
    Serial.println("Halting processor...");
    delete client;
    client = nullptr;
    Serial.flush();
    ESP.deepSleep(0);
  }
  delete client;

  //送信のタイムラグの影響かSpreadsheetへ記載されないタイミングがあるため10秒以下はおすすめしません。
  delay(10000);
}
