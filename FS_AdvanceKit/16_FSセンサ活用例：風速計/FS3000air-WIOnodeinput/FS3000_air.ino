#include <AzureIoTHub.h>
#include <Wire.h>
#include <SparkFun_FS3000_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_FS3000

FS3000 fs;
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

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode..."); 

  // ----------
  // WiFi及びAzure周りの初期化
  WiFi.begin("SSID", "Pass word");    // 2.4GHz帯のWiFiを指定すること
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

  Wire.begin(I2C_SDA, I2C_SCL); 
  delay(5); 

// FS3000の初期設定
  fs.begin();
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
 
    digitalWrite(BLUE_LED, LOW);

//    Serial.print("FS3000 Readings \tRaw: ");
//    Serial.print(fs.readRaw()); // note, this returns an int from 0-3686

    float result = fs.readMetersPerSecond();
    
    Serial.print("\tm/s: ");
    Serial.print(result); // note, this returns a float from 0-7.23
    Serial.println("");
    
//    Serial.print("\tmph: ");
//    Serial.println(fs.readMilesPerHour()); // note, this returns a float from 0-16.17

    // Azureへ送るデータの用意
    a.setValue("sensor", "air");    
    a.setValue("EspValue", result);
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);

    // 1secのwait
    delay(1000);
}
}
