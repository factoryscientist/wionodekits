#include <AzureIoTHub.h>
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
const uint8_t PIR = PORT1B;
int pretime;

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");
  WiFi.begin("SSID", "PASS WORD");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("IoThub接続文字列"); //YourKey

  Azure.setCallback(azureCallback);
  // ----------
  // IO周りの初期化
  pinMode(PIR, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5);
  Serial.println("WioNode With PIR Motion Sensor started!");
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

//10秒間に１回でも検出があると”１”を送信する。
void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
  Azure.connect();
  unsigned char detect = 0;
  digitalWrite(BLUE_LED, HIGH); // LED off
  pretime = 100; //100*100ms-> 10sec
  for(int i = 0; i < pretime; i++){
     if(digitalRead(PIR)>0){
       detect=1;
       digitalWrite(BLUE_LED, LOW); // LED on
     }
     delay(100); //0.1s Wait
   }
  Serial.print("PIR Motion: ");
  Serial.print(detect);
  Serial.println("");
  DataElement a = DataElement();
  // Azureへ送るデータの用意
  a.setValue("sensor","PIR");
  a.setValue("EspValue", detect);
  // Azure IoT Hub へプッシュ
  Azure.push(&a);
  } else {
      Serial.print("Not connected to the Internet:");
      Serial.println(WiFi.status());
      delay(250);
    }
}
