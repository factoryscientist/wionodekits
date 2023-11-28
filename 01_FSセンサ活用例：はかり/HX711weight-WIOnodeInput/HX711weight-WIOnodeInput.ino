#include <AzureIoTHub.h>

#include "HX711.h"

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

//HX711 circuit writing
const int LOADCELL_DOUT_PIN = PORT1A; // HX711 circuit wiring
const int LOADCELL_SCK_PIN  = PORT1B;  // HX711 circuit wiring

HX711 scale;

void setup() {
  WiFi.begin("SSID", "Password");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("YourKey");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  

  // デバッグ用のシリアル通信の初期化
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");
  Serial.println("HX711");

  // IO周りの初期化
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 

  // HX711の初期化
  pinMode(LOADCELL_DOUT_PIN, INPUT); //HX711
  pinMode(LOADCELL_SCK_PIN, OUTPUT); //HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();    // 補正値＝ここを空にし得られた結果÷補正に使用したおもりの重さ　
  delay(400);//はかりが安定するまで待つ
  scale.tare();              // 風袋リセット、電源を入れたときの重さがゼロになる
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop(){
  if (WiFi.status() == WL_CONNECTED) {   
    Azure.connect();
    DataElement a = DataElement();

    //Customize
    if (scale.is_ready()) {
      float weight_data = scale.get_units(10);  //10回平均値を得る
      Serial.print("meas : ");
      Serial.println(weight_data);

      // Azureへ送るデータの用意
      a.setValue("sensor","weight");
      a.setValue("espvalue", weight_data);
      Azure.push(&a);    // Azure IoT Hub へプッシュ

      delay(2000);      //2秒の待ち時間、計測に合わせて調整ください
    } else {
    Serial.println("HX711 not found.");
    delay(250);
     }
  
  }else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);
  }
}
