#include <AzureIoTHub.h>
#include <Wire.h>//I2C

//customize

#include <ClosedCube_SHT31D.h>
ClosedCube_SHT31D sht3xd;

#include "Seeed_MCP9600.h"
MCP9600 sensor;
//

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


//

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "Wi-Fi Password");
  Azure.begin("YourKey"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);


  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);



  //Customize area
     if(sensor.init(THER_TYPE_K))
    {
        Serial.println("sensor init failed!!");
    }
    sensor_basic_config(); 


    
   
    Serial.println("tempSensor started!");

  //

  // SHT31, SHT35の初期化
  Wire.begin(I2C_SDA, I2C_SCL);
  sht3xd.begin(0x44); // Seeed製：Grove SHT31 モジュールを使いたい場合に有効
  //sht3xd.begin(0x45); // Seeed製：Grove SHT35 モジュールを使いたい場合に有効

  // SHT31, SHT35をソフトウェアリセット
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

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();
    DataElement b = DataElement();
    DataElement c = DataElement();

    durationtime = (millis() - pretime)/1000;
    pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");


    //Customize
    //float temperature_data = sht31.getTemperature();
    //float humidity_data = sht31.getHumidity();
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);
    // ClockStrech Modeで計測
    SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    if (result.error != SHT3XD_NO_ERROR) {
      Serial.print("[SHT3X]Error code: ");
      Serial.println(result.error);
      return;
    }

    float temperature_data = 0;
    get_temperature(&temperature_data);
    
    Serial.print("k_temp: "); 
    Serial.print(temperature_data);
    //Serial.print(temperature_data,2);
    Serial.println(" *C");

    a.setValue("sensor", "k_temp");
    a.setValue("espvalue", temperature_data);
    a.setValue("duration", (int)durationtime);    

    float r_temp = 0;
    float humi = 0;
    r_temp = result.t;
    humi = result.rh;
    Serial.print("SHT31_temp: "); 
    Serial.print(r_temp, 4);
    Serial.println("*C");
    Serial.print("Humi: "); 
    Serial.print(humi, 4);
    Serial.println("%"); 

    float dif = r_temp - temperature_data;
    if(dif > 10 && humi > 50){
      Serial.println("ketsuro");
      a.setValue("status", 1);
    } else if(dif > 7 && humi > 60){
      Serial.println("ketsuro");
      a.setValue("status", 1);
    } else if(dif > 5 && humi > 70){
      Serial.println("ketsuro");
      a.setValue("status", 1);
    } else if(dif > 3 && humi > 80){
      Serial.println("ketsuro");
      a.setValue("status", 1);
    } else {
      Serial.println("safe");
      a.setValue("status", 0);
    }

    Azure.push(&a);

    //
    b.setValue("sensor", "r_temp");
    b.setValue("espvalue", r_temp);
    b.setValue("duration", (int)durationtime);    
    Azure.push(&b);
    c.setValue("sensor", "humi");
    c.setValue("espvalue", humi);
    c.setValue("duration", (int)durationtime);    
    Azure.push(&c);

    //Serial.println("0");
    delay(5000);
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}


//for temp
err_t sensor_basic_config()
{
    err_t ret=NO_ERROR;
    CHECK_RESULT(ret,sensor.set_filt_coefficients(FILT_MID));
    CHECK_RESULT(ret,sensor.set_cold_junc_resolution(COLD_JUNC_RESOLUTION_0_25));
    CHECK_RESULT(ret,sensor.set_ADC_meas_resolution(ADC_14BIT_RESOLUTION));
    CHECK_RESULT(ret,sensor.set_burst_mode_samp(BURST_32_SAMPLE));
    CHECK_RESULT(ret,sensor.set_sensor_mode(NORMAL_OPERATION));
    return ret;
}


err_t get_temperature(float *value)
{
    err_t ret=NO_ERROR;
    float hot_junc=0;
    float junc_delta=0;
    float cold_junc=0;
    CHECK_RESULT(ret,sensor.read_hot_junc(&hot_junc));
    CHECK_RESULT(ret,sensor.read_junc_temp_delta(&junc_delta));
    
    CHECK_RESULT(ret,sensor.read_cold_junc(&cold_junc));
    
    // SERIAL.print("hot junc=");
    // SERIAL.println(hot_junc);
    // SERIAL.print("junc_delta=");
    // SERIAL.println(junc_delta);
    // SERIAL.print("cold_junc=");
    // SERIAL.println(cold_junc);

    *value=hot_junc;

    return ret;
}
