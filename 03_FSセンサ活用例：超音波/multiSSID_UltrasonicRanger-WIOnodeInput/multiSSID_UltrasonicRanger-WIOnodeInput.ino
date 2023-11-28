#include <AzureIoTHub.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
extern "C" {
#include "user_interface.h"
}
#include <Ultrasonic.h>

int fail_count = 0;

//Wio node用ポート番号指定
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

ESP8266WiFiMulti wifimulti;
Ultrasonic ultrasonic(5);

void setup()
{
  Serial.begin(115200);
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

  // 複数のアクセスポイントが登録できます。必要のないアクセスポイントはコメントアウトするか削除します。2.4GHz帯のWiFiを指定すること
  wifimulti.addAP("SSID-1", "Password-1");  //1つめのアクセスポイント情報
  //wifimulti.addAP("SSID-2", "Password-2");  //2つめのアクセスポイント情報
  //wifimulti.addAP("SSID-3", "Password-3");  //3つめのアクセスポイント情報
  // 以下、必要に応じて追加

// Azure IoT Hub のデバイスから取得したプライマリ文字列を反映
  Azure.begin("YourKey");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  
  
  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(5); 
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop()
{
  wl_status_t wifistatus;
  while(wifimulti.run() != WL_CONNECTED){
    Serial.println("Re-connecting...");
    delay(1000);
    }
    wifistatus = WL_CONNECTED;
  if (wifimulti.run() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();
    
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

    int RangeInInches;
    int RangeInCentimeters;
 
    Serial.println("The distance to obstacles in front is: ");
    RangeInInches = ultrasonic.MeasureInInches();
    Serial.print(RangeInInches);//0~157 inches
    Serial.println(" inch");
    delay(250);
 
    RangeInCentimeters = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
    Serial.print(RangeInCentimeters);//0~400cm
    Serial.println(" cm");
    delay(250);

    // Azureへ送るデータの用意
    a.setValue("Sensor", "ultrasonicranger");    
    a.setValue("EspValue", RangeInCentimeters);
    
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
    // delay/1000secのwait
    delay(1000);
  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if(50 <= fail_count++){
      ESP.restart();
    }
  }
}
