#include <M5Stack.h>
#include <AzureIoTHub.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"

ClosedCube_SHT31D sht3xd;

void setup() {
  // initialize the M5Stack object
  M5.begin();
  Serial.begin(115200);
  Serial.println("begin...");
  
  //Wifi及びAzure接続設定
  WiFi.begin("SSID","PASS");
  Azure.begin("IOTHUB KEY"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);

  // SHT31, SHT35の初期化
  Wire.begin();
  sht3xd.begin(0x44); // Seeed製：Grove SHT31 モジュールを使いたい場合に有効
  //sht3xd.begin(0x45); // Seeed製：Grove SHT35 モジュールを使いたい場合に有効

  SHT31D_ErrorCode resultSoft = sht3xd.softReset();
  if (resultSoft != SHT3XD_NO_ERROR) {
    Serial.print("[SHT3X]Error code: ");
    Serial.println(resultSoft);
    while (1);
  }

  M5.Power.begin();
  delay(5);

  //M5GOの液晶表示設定（画面の向き"1"〜"4"で指定/フォントカラー）
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(6);
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement(); //温度を入れる
    DataElement b = DataElement(); //湿度を入れる

    //sht3xから温湿度を読み取る
    SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    if (result.error != SHT3XD_NO_ERROR) {
      Serial.print("[SHT3X]Error code: ");
      Serial.println(result.error);
      return;
    }

    //シリアルモニタへ表示
    Serial.print("Temp = ");
    Serial.print(result.t, 4);
    Serial.println(" C"); //The unit for  Celsius because original arduino don't support speical symbols
    Serial.print("Humi = ");
    Serial.print(result.rh, 4);
    Serial.println(" %");
    Serial.println();

    //ここからM5GOの液晶表示に関すること
    //文字の色と背景色の指定
    M5.Lcd.setTextColor(WHITE,BLACK);

    //温度と湿度状況によって3段階に四角の色を変える
    //温度30度以上を赤、30〜20度までを緑、それ以外を青
    if (result.t >= 30.0) {
      M5.Lcd.fillRect(250, 20, 70, 70, RED);
    }
    else if (result.t > 20.0) {
      M5.Lcd.fillRect(250, 20, 70, 70, GREEN);
    }
    else {
      M5.Lcd.fillRect(250, 20, 70, 70, BLUE);
    }
    
    //湿度75%以上を赤、75〜60%を緑、それ以外を青
    if (result.rh >= 75.0) {
      M5.Lcd.fillRect(250, 140, 70, 70, RED);
    }
    else if (result.rh > 60.0) {
      M5.Lcd.fillRect(250, 140, 70, 70, GREEN);
    }
    else {
      M5.Lcd.fillRect(250, 140, 70, 70, BLUE);
    }

    //液晶の基本設定(文字の開始位置/表示する情報)
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print ("Temp");
    M5.Lcd.setCursor(20, 60);
    M5.Lcd.print (result.t);
    M5.Lcd.setCursor(20, 140);
    M5.Lcd.println ("Humi");
    M5.Lcd.setCursor(20, 190);
    M5.Lcd.println (result.rh);

    //azureへデータを送信
    a.setValue("Sensor", "tempM5");
    a.setValue("EspValue", result.t);
    Azure.push(&a);

    b.setValue("Sensor", "humM5");
    b.setValue("EspValue", result.rh);
    Azure.push(&b);

    M5.update();

    //5秒おきに送信
    delay(5000);

  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}
