#include <AzureIoTHub.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
extern "C" {
#include "user_interface.h"
}

/*****************************************************************************/
//	Function:    Get the accelemeter of X/Y/Z axis and print out on the
//					serial monitor.
//  Hardware:    3-Axis Digital Accelerometer(+-16g)
//	Arduino IDE: Arduino-1.0
//	Author:	 Frankie.Chu
//	Date: 	 Jan 11,2013
//	Version: v1.0
//	by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/*******************************************************************************/

#include <ADXL345.h>

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

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library
ESP8266WiFiMulti wifimulti;

void setup() {
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

    adxl.powerOn();

    //set activity/ inactivity thresholds (0-255)
    adxl.setActivityThreshold(75); //62.5mg per increment
    adxl.setInactivityThreshold(75); //62.5mg per increment
    adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?

    //look of activity movement on this axes - 1 == on; 0 == off
    adxl.setActivityX(1);
    adxl.setActivityY(1);
    adxl.setActivityZ(1);

    //look of inactivity movement on this axes - 1 == on; 0 == off
    adxl.setInactivityX(1);
    adxl.setInactivityY(1);
    adxl.setInactivityZ(1);

    //look of tap movement on this axes - 1 == on; 0 == off
    adxl.setTapDetectionOnX(0);
    adxl.setTapDetectionOnY(0);
    adxl.setTapDetectionOnZ(1);

    //set values for what is a tap, and what is a double tap (0-255)
    adxl.setTapThreshold(50); //62.5mg per increment
    adxl.setTapDuration(15); //625us per increment
    adxl.setDoubleTapLatency(80); //1.25ms per increment
    adxl.setDoubleTapWindow(200); //1.25ms per increment

    //set values for what is considered freefall (0-255)
    adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
    adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment

    //setting all interrupts to take place on int pin 1
    //I had issues with int pin 2, was unable to reset it
    adxl.setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT,   5);
    adxl.setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT,   5);
    adxl.setInterruptMapping(ADXL345_INT_FREE_FALL_BIT,    5);
    adxl.setInterruptMapping(ADXL345_INT_ACTIVITY_BIT,     5);
    adxl.setInterruptMapping(ADXL345_INT_INACTIVITY_BIT,   5);

    //register interrupt actions - 1 == on; 0 == off
    adxl.setInterrupt(ADXL345_INT_SINGLE_TAP_BIT, 1);
    adxl.setInterrupt(ADXL345_INT_DOUBLE_TAP_BIT, 1);
    adxl.setInterrupt(ADXL345_INT_FREE_FALL_BIT,  1);
    adxl.setInterrupt(ADXL345_INT_ACTIVITY_BIT,   1);
    adxl.setInterrupt(ADXL345_INT_INACTIVITY_BIT, 1);
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  wl_status_t wifistatus;
  while(wifimulti.run() != WL_CONNECTED){
    Serial.println("Re-connecting...");
    delay(1000);
    }
    wifistatus = WL_CONNECTED;
  if (wifimulti.run() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();

    durationtime = (millis() - pretime)/1000;
    pretime = millis();
    
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

    //Boring accelerometer stuff
    int x, y, z;
    adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
    // Output x,y,z values
    Serial.print("values of X , Y , Z: ");
    Serial.print(x);
    Serial.print(" , ");
    Serial.print(y);
    Serial.print(" , ");
    Serial.println(z);

    double xyz[3];
    double ax, ay, az;
    adxl.getAcceleration(xyz);
    ax = xyz[0];
    ay = xyz[1];
    az = xyz[2];
    Serial.print("X=");
    Serial.print(ax);
    Serial.println(" g");
    Serial.print("Y=");
    Serial.print(ay);
    Serial.println(" g");
    Serial.print("Z=");
    Serial.print(az);
    Serial.println(" g");
    Serial.print("duration:");
    Serial.println(durationtime);
    Serial.println("**********************");

    // Azureへ送るデータの用意
    a.setValue("Sensor", "3-axis Accelerator");
    a.setValue("EspValueX", ax);
    a.setValue("EspValueY", ay);
    a.setValue("EspValueZ", az);
    a.setValue("duration", (int)durationtime);
    
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);

    // wait for delay/1000 sec
    delay(60000);
  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if(50 <= fail_count++){
      ESP.restart();
    }
  }
}
