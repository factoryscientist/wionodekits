#include <AzureIoTHub.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"

#include <Arduino.h>
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);         

ClosedCube_SHT31D sht3xd;
int fail_count = 0;

unsigned long durationtime;
unsigned long pretime;

//cusomize

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("*** Wake up Xiao_ESP32C...");
  Serial.println(__FILE__);
  Serial.println(__DATE__ " " __TIME__);
  Serial.println("DeviceID: ********");  
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
  WiFi.begin("SSIDを入力してください", "パスワードを入力してください");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("Keyを入力してください");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  

  // ----------
  // SHT3xの初期化
  Wire.begin();
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

  // OLEDディスプレイを初期化
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  char dispBuf[32];
  char numBuf[16];

  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
  
    durationtime = (millis() - pretime)/1000;
    pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");
    
    //Customize
    // センサ値の取得および出力
    // ClockStrech Modeで計測
    SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    if (result.error != SHT3XD_NO_ERROR) {
      Serial.print("[SHT3X]Error code: ");
      Serial.println(result.error);
      return;
    }
    Serial.print("Temp: "); 
    Serial.print(result.t, 4);
    Serial.println("C");
    Serial.print("Humi: "); 
    Serial.print(result.rh, 4);
    Serial.println("%"); 

    //OLEDへ表示
    u8x8.clear();
    dtostrf(result.t, 4, 1 ,numBuf);
    sprintf(dispBuf, "Temp: %sC", numBuf);
    u8x8.drawString(0,0,dispBuf);

    dtostrf(result.rh, 4, 1 ,numBuf);
    sprintf(dispBuf, "Humi: %s%%", numBuf);
    u8x8.drawString(0,1,dispBuf);
    u8x8.refreshDisplay();    // only required for SSD1606/7  
    
    // Azureへ送るデータの用意(温度のみ)
    DataElement a = DataElement();
    a.setValue("sensor", "temp");    
    a.setValue("EspValue", result.t);
    a.setValue("duration", (int)durationtime);      
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    //一緒にデータを送信しても良いのですが、クエリの変更が必要となるので同じ形式で送信しています。
    DataElement b = DataElement();
    // Azureへ送るデータの用意(湿度のみ)
    b.setValue("sensor", "humi");    
    b.setValue("EspValue", result.rh);
    b.setValue("duration", (int)durationtime);      
    // Azure IoT Hub へプッシュ
    Azure.push(&b);
    
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
