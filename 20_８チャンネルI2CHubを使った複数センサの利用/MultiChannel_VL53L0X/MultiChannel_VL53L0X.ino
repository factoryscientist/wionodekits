// マルチチャンネルI2CHub（TCA9548A)　で、2個の　VL53L0X　TOF距離センサーを測定する
/*
 * The MIT License (MIT)
 * 
 * Author: Hongtai Liu (lht856@foxmail.com)
 * 
 * Descript: This Usage show the basic use of TCA9548A.
 * 
 * Copyright (C) 2019  Seeed Technology Co.,Ltd. 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//WioNode用
const uint8_t PORT0A = 1;
const uint8_t PORT0B = 3;
const uint8_t PORT1A = 4;
const uint8_t PORT1B = 5;
const uint8_t PORT_POWER = 15; // (common with RED_LED)
const uint8_t I2C_SDA = PORT1A;
const uint8_t I2C_SCL = PORT1B;
const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

//マルチチャンネルI2CHub　
#include "TCA9548A.h"
// if you use the software I2C to drive, you can uncommnet the define SOFTWAREWIRE which in TCA9548A.h. 
#undef SOFTWAREWIRE
#ifdef SOFTWAREWIRE
  #include <SoftwareWIRE.h>
  SoftwareWire myWIRE(3, 2);
  TCA9548A<SoftwareWire> TCA;
  #define WIRE myWIRE
#else   
  #include <Wire.h>
  TCA9548A<TwoWire> TCA;
  #define WIRE Wire
#endif
#define MUX_ADDRESS TCA9548_DEFAULT_ADDRESS

//超音波距離センサー　
#define SERIAL Serial
#include "Seeed_vl53l0x.h"
const int Cnt_VL53L0X=2;
Seeed_vl53l0x VL53L0X[Cnt_VL53L0X];
int i;

void setup()
{
 // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 

  SERIAL.begin(115200);
  while(!SERIAL){};

  //マルチチャンネルI2CHub初期化
  //WIRE.begin();
  TCA.begin(WIRE);
  //defaut all channel was closed
  //TCA.openAll();
  //TCA.closeAll();

  // Select the channels you want to open. You can open as many channels as you want
  TCA.openChannel(TCA_CHANNEL_0);   //TCA.closeChannel(TCA_CHANNEL_0);
  TCA.openChannel(TCA_CHANNEL_1); //TCA.closeChannel(TCA_CHANNEL_1);
  //TCA.openChannel(TCA_CHANNEL_2); //TCA.closeChannel(TCA_CHANNEL_2);
  //TCA.openChannel(TCA_CHANNEL_3); //TCA.closeChannel(TCA_CHANNEL_3);
  //TCA.openChannel(TCA_CHANNEL_4); //TCA.closeChannel(TCA_CHANNEL_4);
  //TCA.openChannel(TCA_CHANNEL_5); //TCA.closeChannel(TCA_CHANNEL_5);
  //TCA.openChannel(TCA_CHANNEL_6); //TCA.closeChannel(TCA_CHANNEL_6);
  //TCA.openChannel(TCA_CHANNEL_7); //TCA.closeChannel(TCA_CHANNEL_7); 

  //チャンネル数分　TOFセンサー初期化
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  for(i=0; i<Cnt_VL53L0X; i++){
    switchChannel(i);
    
    Status = VL53L0X[i].VL53L0X_common_init();
    if (VL53L0X_ERROR_NONE != Status) {
        SERIAL.println("start vl53l0x mesurement failed!");
        VL53L0X[i].print_pal_error(Status);
        while (1);
    }

    VL53L0X[i].VL53L0X_high_accuracy_ranging_init();

    if (VL53L0X_ERROR_NONE != Status) {
        SERIAL.println("start vl53l0x mesurement failed!");
        VL53L0X[i].print_pal_error(Status);
        while (1);
    }    
  }

}

void loop()
{
  for(i=0; i<Cnt_VL53L0X; i++){
    switchChannel(i);

    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;

    memset(&RangingMeasurementData, 0, sizeof(VL53L0X_RangingMeasurementData_t));
    Status = VL53L0X[i].PerformSingleRangingMeasurement(&RangingMeasurementData);
    if (VL53L0X_ERROR_NONE == Status) {
        if (RangingMeasurementData.RangeMilliMeter >= 2000) {
            SERIAL.print("Chan ");
            SERIAL.print(i);
            SERIAL.println(":out of range!!");
        } else {
            SERIAL.print("Chan ");
            SERIAL.print(i);
            SERIAL.print(":Measured distance:");
            SERIAL.print(RangingMeasurementData.RangeMilliMeter);
            SERIAL.println(" mm");
        }
    } else {
        SERIAL.print("Chan ");
        SERIAL.print(i);
        SERIAL.print(":mesurement failed !! Status code =");
        SERIAL.println(Status);
    }

    delay(300);
  }
}

//チャンネル切り換え
void switchChannel(int channel) {
  WIRE.beginTransmission(MUX_ADDRESS);
  WIRE.write(1 << channel);
  WIRE.endTransmission();
}
