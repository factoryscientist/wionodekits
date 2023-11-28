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
const uint8_t I2C_SDA = PORT1A;
const uint8_t I2C_SCL = PORT1B;

unsigned long durationtime;
unsigned long pretime;

unsigned long previousMicros = 0;  // 前回パルスをHIGHにした時間
unsigned long pulseInterval = 0;   // パルス間隔
bool pulseState = false;           // パルスの状態

const unsigned long logInterval = 5000; // ログ間隔（ミリ秒）
unsigned long previousLogMillis = 0;  // 前回ログを取った時間

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");
  Serial.println(__FILE__);
  Serial.println(__DATE__ " " __TIME__);
  Serial.println("DeviceId=  ");

  WiFi.begin("SSID" , "PASS");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("Keyをコピペ");   //YourKey Example:""
  Azure.setCallback(azureCallback);

  // IO周りの初期化
  pinMode(PORT1A, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(FUNC_BTN, INPUT);
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

void loop() {
  if (true) {
//  if (WiFi.status() == WL_CONNECTED) {
//    Azure.connect();
    DataElement a = DataElement();

    digitalWrite(BLUE_LED, LOW);

    // 計測
    int v = analogRead(A0);
    

    // パルス間隔を計算
    // 注意: analogValueが0のときはdivide by zeroエラーを防ぐため特別に扱う
    if (v != 0) {
      pulseInterval = map(v, 5, 1024, 1000000, 40000);  // 1～1023を1000000μs～40000μsに変換
    } else {
      pulseInterval = LONG_MAX;  // analogValueが0のときは最大値を割り当て
    }
  
    unsigned long currentMicros = micros();
  
    if (pulseState == false) {
      if (currentMicros - previousMicros >= pulseInterval) {
        digitalWrite(PORT1A, HIGH);
        pulseState = true;
        previousMicros = currentMicros;
      }
    } else {
      if (currentMicros - previousMicros >= 40000) {  // 40msのHIGH
        digitalWrite(PORT1A, LOW);
        pulseState = false;
        previousMicros = currentMicros;
      }
    }

    // ログ出力
    unsigned long currentMillis = millis();
    durationtime = currentMillis - previousLogMillis;
    if (durationtime >= logInterval) {
      Serial.print("duration time: "); 
      Serial.print(durationtime);
      Serial.println(" msec for this loop");
      Serial.print("v: ");
      Serial.println(v);
      Serial.print("pulseInterval: ");
      Serial.print(pulseInterval);

      // Azureへ送るデータの用意
      //湿度の場合はコメントアウトを差し替える
      //a.setValue("duration",(int)durationtime);
      //a.setValue("Sensor", "Vol");
      //a.setValue("EspValue", v);
  
      // Azure IoT Hub へプッシュ
      //Azure.push(&a);
      
      previousLogMillis = currentMillis;
    }

    // wait中は青LEDを消灯する
//    digitalWrite(BLUE_LED, HIGH);


  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if (50 <= fail_count++) {
      ESP.restart();
    }
  }
}
