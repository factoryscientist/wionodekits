#include <AzureIoTHub.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
#include "ClosedCube_SHT31D.h"
#include <ESP8266WiFiMulti.h>
extern "C" {
#include "user_interface.h"
}

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

int sensorPin = A0;
int sensorValue = 0;
float vinput = 0.0;//電圧変換用
float mpainput = 0.0;//圧力変換用
//下記を初期に補正
float vstart = 0.514;//補正値用 圧力ゼロの状態の時の電圧値を入力
float calmpa = 1;//補正値用　センサーの圧力値を入力 MPa
float calv = 2.5;//補正値用　上記calmpaの電圧値を入力
//ここまで
float slope = (calv - vstart) / calmpa;

ESP8266WiFiMulti wifimulti;

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "pass");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("デバイスの接続文字列");   //YourKey Exampl接続文字列e:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  
  Serial.println("");
  Serial.println("*** Wake up WioNode...");
  
  // ----------
  // WiFi及びAzure周りの初期化
  // 固定IP化する為の処理(DHCP利用時は以下をコメントアウト)
  /*
  WiFi.config(IPAddress(192,168,0,12),IPAddress(192,168,0,1),IPAddress(255,255,255,0));
  Serial.println("---");
  Serial.print("Local IP  :");Serial.println(address(WiFi.localIP()));
  Serial.print("Gateway IP:");Serial.println(address(WiFi.gatewayIP()));
  Serial.print("SubnetMask:");Serial.println(address(WiFi.subnetMask()));
  Serial.print("macAddress:");Serial.println(WiFi.macAddress());
  //*/


  // 複数のアクセスポイントを登録する場合 2.4GHz帯のWiFiを指定すること
//  wifimulti.addAP("SSID-1", "Password-1");
//  wifimulti.addAP("SSID-2", "Password-2");
//  wifimulti.addAP("SSID-3", "Password-3");
  // 以下、必要に応じて追加

  
  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
   digitalWrite(15, 1);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(5); 
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
  
// センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

// read the value from the sensor:
    sensorValue = system_adc_read();
//数値変換
    vinput = 3.0 * sensorValue / 1024;
    mpainput = vinput/slope - vstart/slope;
//シリアル出力情報
    Serial.print("value = " );
    Serial.println(sensorValue);
    Serial.print("V = " );
    Serial.println(vinput);
    Serial.print("MPA = " );
    Serial.println(mpainput);

    // Azureへ送るデータの用意
    a.setValue("Sensor", "MPa");    
    a.setValue("EspValue", mpainput);
    
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
    
    // delay/1000secのwait
    delay(5000);
  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if(50 <= fail_count++){
      ESP.restart();
    }
  }
}
