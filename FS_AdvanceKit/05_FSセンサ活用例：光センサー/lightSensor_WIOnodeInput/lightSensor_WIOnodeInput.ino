#include <AzureIoTHub.h>
#include <Wire.h>

int fail_count = 0;
const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t PORT_POWER = 15; // (common with RED_LED)
const uint8_t RED_LED = PORT_POWER;

unsigned long durationtime;
unsigned long pretime;

void setup()
{
  Serial.begin(115200);
  delay(1000);
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
  Azure.begin("接続文字列を入力してください");  
  //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
    Azure.setCallback(azureCallback);  

  // ----------
  // IO周りの初期化
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 
 
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}


void loop(){
 
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
    delay(2500);
    
    int value = analogRead(A0);
    Serial.print("Light: "); 
    Serial.println(value);

    // Azureへ送るデータの用意
    a.setValue("sensor", "light");    
    a.setValue("EspValue", value);
    a.setValue("duration", (int)durationtime);      
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);
    
    // 5secのwait
    delay(2500);
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
