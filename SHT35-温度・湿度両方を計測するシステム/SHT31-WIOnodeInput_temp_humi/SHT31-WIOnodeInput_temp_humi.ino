/*
 * このプログラムを使用する場合はクエリの変更が必要となります。
 * 現在のクエリでは、折角センサーの名前を送信しているのに、
 * それをpowerbiやcosmosdbに送信していません。
 * そのため、フィルターが出来ず、温度と湿度の区別が出来なくなっています。
 * ====================================
 Dev as device,
 params.sensor as sensor, //この一文を挿入してください。最後の「,」は途中に挿入する際に必要となります。
 DATEADD(hour,9,EventEnqueuedUtcTime) as time,
 EventEnqueuedUtcTime as utctime,
 params.espvalue as value //SELECT文の最終行なので、「,」が必要ありません。
 * ====================================
 */

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


unsigned long durationtime;
unsigned long pretime;

//cusomize


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
  WiFi.begin("SSID", "PASS");   // 2.4GHz帯のWiFiを指定すること
  Azure.begin("IoTHub 接続文字列");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
    Azure.setCallback(azureCallback);  
  
  // ----------
  // IO周りの初期化
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
  sht3xd.begin(0x44); // Seeed製：Grove SHT31 モジュールを使いたい場合に有効
  //sht3xd.begin(0x45); // Seeed製：Grove SHT35 モジュールを使いたい場合に有効

  // SHT35をソフトウェアリセット
  SHT31D_ErrorCode resultSoft = sht3xd.softReset();
  if(resultSoft != SHT3XD_NO_ERROR){
    Serial.print("[SHT3X]Error code: ");
    Serial.println(resultSoft);
    while(1);
  }
  delay(5); 
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();

    DataElement a = DataElement();
    DataElement b = DataElement();
    
    durationtime = (millis() - pretime)/1000;
    pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");
    
    
    //Customize
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);
    // ClockStrech Modeで計測
    SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    if (result.error != SHT3XD_NO_ERROR) {
      Serial.print("[SHT3X]Error code: ");
      Serial.println(result.error);
      return;
    }
    Serial.print("Temp: "); 
    Serial.print(result.t, 4);
    Serial.println("*C");
    Serial.print("Humi: "); 
    Serial.print(result.rh, 4);
    Serial.println("%"); 

    
    // Azureへ送るデータの用意(温度のみ)
    a.setValue("sensor", "temp");    
    a.setValue("EspValue", result.t);
    a.setValue("duration", (int)durationtime);      
    // Azure IoT Hub へプッシュ
    Azure.push(&a);
    
    // Azureへ送るデータの用意(湿度のみ)
    b.setValue("sensor", "humi");    
    b.setValue("EspValue", result.rh);
    b.setValue("duration", (int)durationtime);      
    // Azure IoT Hub へプッシュ
    Azure.push(&b);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
    
    // 5secのwait
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

String address(IPAddress ip){
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}
