#include <AzureIoTHub.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>

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

//cusomize
ESP8266WiFiMulti wifimulti;


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
  //WiFi.begin("SSID", "Password");    // 2.4GHz帯のWiFiを指定すること

  // 複数のアクセスポイントを登録する場合 2.4GHz帯のWiFiを指定すること
/*  wifimulti.addAP("SSID-1", "Password-1");
  wifimulti.addAP("SSID-2", "Password-2");
  wifimulti.addAP("SSID-3", "Password-3");
*/
  wifimulti.addAP("FabLabSagaGuest", "FLS140530");
  wifimulti.addAP("XperiaX", "Jinnouchi");
  wifimulti.addAP("Buffalo-G-0F7A", "ki8s6kt7hht8x");
  wifimulti.addAP("001601BFB8E6", "bb2abc7b89");
  // 以下、必要に応じて追加
  
  Azure.begin("HostName=jinnouchiIoTHuB.azure-devices.net;DeviceId=Jinnouchi05WiFiAP;SharedAccessKey=hL3rIeMcBqGpOC1W51WMZ+s8LYoko6wUvNNzaua5I70=");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
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

  delay(5); 
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop()
{
  wl_status_t wifistatus;
  while(wifimulti.run() != WL_CONNECTED){
    Serial.println("Re-connecting...");
    delay(1000);
    }
  wifistatus = WL_CONNECTED;
  if (wifimulti.run() == WL_CONNECTED) {
    // Azureへ送るデータの用意(温度の
    Azure.connect();
    DataElement a = DataElement();
    
    //Customize
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

    
    Serial.print("SSID: "); 
    Serial.println(WiFi.SSID());
    Serial.print("RSSI: "); 
    Serial.println(WiFi.RSSI());
    Serial.print("BSSID: "); 
    Serial.println(WiFi.BSSIDstr());  //アクセスポイントのMACアドレス

    a.setValue("sensor", "WiFi");    
    a.setValue("SSID", WiFi.SSID().c_str());    
//  a.setValue("BSSID", WiFi.BSSIDstr().c_str());   //アクセスポイントのMACアドレス
    a.setValue("EspValue", WiFi.RSSI());

    // Azure IoT Hub へプッシュ
    Azure.push(&a);

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
