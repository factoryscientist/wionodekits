#include <AzureIoTHub.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

//cusomize
OneWire  oneWire(PORT1B);  // This is where DQ of your DS18B20 will connect.
DallasTemperature sensors(&oneWire);

//使用するセンサーのアドレスを記載してください
DeviceAddress temp_addr1 = {0x28,0xCA,0xFA,0x96,0xF0,0x1,0x3C,0xE9};//例
DeviceAddress temp_addr2 = {};
DeviceAddress temp_addr3 = {};
DeviceAddress temp_addr4 = {};

void setup(void)
{
 Serial.begin(115200);
 Serial.println("");
 Serial.println("DS18B20 Temperature");

// ----------
  // WiFi及びAzure周りの初期化
  WiFi.begin("wifiSSID", "wifi password");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("HostName= ");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  


  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 

  //DS18B20の初期化
  sensors.begin();
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop(void)
{
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();

    digitalWrite(BLUE_LED, LOW);//LED点灯
     // センサ値の取得および出力
    sensors.requestTemperatures();
    //センサ数に合わせてください
    float temp_data1=sensors.getTempC(temp_addr1);
    float temp_data2=sensors.getTempC(temp_addr2);
    float temp_data3=sensors.getTempC(temp_addr3);
    float temp_data4=sensors.getTempC(temp_addr4);

    //センサ数に合わせてください
    Serial.print(temp_data1);
    Serial.print(" ");
    Serial.print(temp_data2);
    Serial.print(" ");
    Serial.print(temp_data3);
    Serial.print(" ");
    Serial.println(temp_data4);

    // Azureへ送るデータの用意
    //センサ数に合わせてください
    a.setValue("sensor", "temp1");  
    a.setValue("EspValue",temp_data1);
    Azure.push(&a);  // Azure IoT Hub へプッシュ
  
    a.setValue("sensor", "temp2");  
    a.setValue("EspValue",temp_data2);
    Azure.push(&a);  // Azure IoT Hub へプッシュ

    a.setValue("sensor", "temp3");  
    a.setValue("EspValue",temp_data3);  
    Azure.push(&a);  // Azure IoT Hub へプッシュ

    a.setValue("sensor", "temp4");  
    a.setValue("EspValue",temp_data4);
    Azure.push(&a);  // Azure IoT Hub へプッシュ
 
    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
   
    // 5secのwait
    delay(5000);
  
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}
