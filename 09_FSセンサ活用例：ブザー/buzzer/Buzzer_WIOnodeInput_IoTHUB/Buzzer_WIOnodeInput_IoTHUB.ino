#include <AzureIoTHub.h>
#include <Wire.h>

int fail_count = 0;
const uint8_t PORT0A = 1;   //左　白
const uint8_t PORT0B = 3;   //左　黄
const uint8_t PORT1A = 4;   //右　白
const uint8_t PORT1B = 5;   //右　黄
const uint8_t PORT_POWER = 15; // (common with RED_LED)
//set PORT_POWER as HIGH for power supply. Low means no power.

const uint8_t BUZZER_PIN = PORT1B;   //右　黄

const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

const uint8_t UART_TX = PORT0A;
const uint8_t UART_RX = PORT0B;
const uint8_t I2C_SDA = PORT1A;
const uint8_t I2C_SCL = PORT1B;

//cusomize

int alert = 0;
unsigned long pretime=0;

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
  WiFi.begin("SSIDを入力してください", "パスワードを入力してください");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("デバイスの接続文字列を入力してください");  
  //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);  
  
  // ----------
  // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(BLUE_LED, HIGH);

  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  
  delay(1000); 

  // ----------
  DataElement a = DataElement();
  alert = 0;

  // Azureへ送るデータの用意  停止　開始
  a.setValue("sensor", "buzzer");    
  a.setValue("EspValue", alert);
  a.setValue("duration", (int)0);
  // Azure IoT Hub へプッシュ
  Azure.push(&a); 
  pretime = millis();
}

void azureCallback(String s) {
  DataElement a = DataElement();
  
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");

  // Azureへ送るデータの用意  停止　終了
  a.setValue("sensor", "buzzer");    
  a.setValue("EspValue", alert);
  a.setValue("duration", (int)(millis()-pretime)/1000);
  // Azure IoT Hub へプッシュ
  Azure.push(&a); 

  tone(BUZZER_PIN, 440);
  alert = 1;

  pretime = millis();

  // Azureへ送るデータの用意  ブザー開始
  a.setValue("sensor", "buzzer");    
  a.setValue("EspValue", alert);
  a.setValue("duration", (int)0);
  // Azure IoT Hub へプッシュ
  Azure.push(&a); 
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();

    //Customize
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

   //funcボタンが押されたらブザー停止
   if ( (1 == alert) && (LOW == digitalRead(FUNC_BTN)) ){

      // Azureへ送るデータの用意　　ブザー終了
      a.setValue("sensor", "buzzer");    
      a.setValue("EspValue", alert);
      a.setValue("duration", (int)(millis()-pretime)/1000);
      // Azure IoT Hub へプッシュ
      Azure.push(&a);

      noTone(BUZZER_PIN);
      alert = 0;
      pretime = millis();
   
      // Azureへ送るデータの用意　　停止　開始
      a.setValue("sensor", "buzzer");    
      a.setValue("EspValue", alert);
      a.setValue("duration", (int)0);
      // Azure IoT Hub へプッシュ
      Azure.push(&a);
    }
   
    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
    
    //wait
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

String address(IPAddress ip){
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  return ipStr;
}
