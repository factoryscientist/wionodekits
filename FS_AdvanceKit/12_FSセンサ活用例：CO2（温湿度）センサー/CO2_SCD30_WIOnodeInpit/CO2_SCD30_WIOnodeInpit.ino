#include <AzureIoTHub.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" 

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
float t,h,c;
SCD30 airSensor;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");

  WiFi.begin("SSID", "PASS");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("HostName=　");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  

  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);//これがないとI2Cが動かない。
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 

  //SCD30の初期化
  Wire.begin(I2C_SDA, I2C_SCL);
  airSensor.begin();           // Initialize SCD30
  delay(1000*10); 
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

    durationtime = (millis() - pretime)/1000;
    pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");
        
    //Customize
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

    if (airSensor.dataAvailable()){
       t = airSensor.getTemperature();
       h = airSensor.getHumidity();
       c = airSensor.getCO2();
       Serial.print("Temp    :");
       Serial.println(t);
       Serial.print("Humidity:");
       Serial.println(h);
       Serial.print("CO2     :");
       Serial.println(c);
    }
    else
    Serial.println("Waiting for new data");

    // Azureへ送るデータの用意
    a.setValue("sensor", "CO2");
    a.setValue("EspValue",c);
    a.setValue("duration", (int)durationtime);      
    Azure.push(&a);   // Azure IoT Hub へプッシュ
    
    a.setValue("sensor", "temp");
    a.setValue("EspValue",t);
    a.setValue("duration", (int)durationtime);      
    Azure.push(&a);   // Azure IoT Hub へプッシュ
    
    a.setValue("sensor", "humi");
    a.setValue("EspValue",h);
    a.setValue("duration", (int)durationtime);      
    Azure.push(&a);   // Azure IoT Hub へプッシュ

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
