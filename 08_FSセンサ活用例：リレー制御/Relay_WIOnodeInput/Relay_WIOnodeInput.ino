#include <AzureIoTHub.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"

ClosedCube_SHT31D sht3xd;
int fail_count = 0;
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

const uint8_t PIN = PORT0B;

//unsigned long durationtime;
//unsigned long pretime;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");

  WiFi.begin("SSID" , "PASS");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("Keyをコピペ");   //YourKey Example:""
  Azure.setCallback(azureCallback);

  // IO周りの初期化
  pinMode(PORT1A, INPUT);
  pinMode(PORT0B, OUTPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  pinMode(PIN, OUTPUT);
  delay(5);

  // SHT31の初期化*sht35の場合は下記コメントアウトを差し替える
  Wire.begin(I2C_SDA, I2C_SCL);
  sht3xd.begin(0x44); // Seeed製：Grove SHT31 モジュールを使いたい場合に有効
  //sht3xd.begin(0x45); // Seeed製：Grove SHT35 モジュールを使いたい場合に有効

  // SHT31をソフトウェアリセット
  SHT31D_ErrorCode resultSoft = sht3xd.softReset();
  if (resultSoft != SHT3XD_NO_ERROR) {
    Serial.print("[SHT3X]Error code: ");
    Serial.println(resultSoft);
    while (1);
  }
  delay(5);
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();

    digitalWrite(BLUE_LED, LOW);

    // ClockStrech Modeで計測
    SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    if (result.error != SHT3XD_NO_ERROR) {
      Serial.print("[SHT3X]Error code: ");
      Serial.println(result.error);
      return;
    }

    //durationtime = (millis() - pretime)/1000;
    //pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");
    Serial.print("Temp: ");
    Serial.print(result.t, 4);
    Serial.println("*C");
    Serial.print("Humi: ");
    Serial.print(result.rh, 4);
    Serial.println("%");

    //温度設定を行い、リレーのオンオフ設定を行う
    if (result.t > 28.0) {
      digitalWrite(PIN, HIGH);
      Serial.print("Relay: ");
      Serial.println(HIGH);

    } else {
      digitalWrite(PIN, LOW);
      Serial.print("Relay: ");
      Serial.println(LOW);
    }

    //28度を超えて、27℃に下がるまでリレーをオンにしたい場合
    //if (result.t > 28.0){
    //  digitalWrite(PIN, HIGH);
    //  Serial.print("Relay: ");
    //  Serial.println(HIGH);
    //  }
    //  else if (reslt.t < 27.0){
    //  digitalWrite(PIN, LOW);
    //  Serial.print("Relay: ");
    //  Serial.println(LOW);
    //  }

    //湿度の状態でリレーのオンオフを行いたい場合
    //if (result.rh < 30.0){
    //  digitalWrite(PIN, HIGH);
    //  Serial.print("Relay: ");
    //  Serial.println(HIGH);
    //  }
    //  else {
    //  digitalWrite(PIN, LOW);
    //  Serial.print("Relay: ");
    //  Serial.println(LOW);
    //}

    int condition = digitalRead (PIN);
    Serial.print("condition: ");
    Serial.println(condition);

    // Azureへ送るデータの用意
    //湿度の場合はコメントアウトを差し替える
    //a.setValue("duration",(int)durationtime);
    a.setValue("Sensor", "temp");
    a.setValue("EspValue", result.t);
    //a.setValue("Sensor2", "humi");
    //a.setValue("EspValue2", result.rh);
    a.setValue("Relay", "PIN");
    a.setValue("EspValue3" , condition);

    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);

    //5秒置きにデータを送付する
    delay(5000);

  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if (50 <= fail_count++) {
      ESP.restart();
    }
  }
}
