#include <AzureIoTHub.h>

//customize
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

//流量センサ用
const int pulsePin = PORT1B;
volatile unsigned long pulseCount = 0; // count of pulses
void ICACHE_RAM_ATTR countPulse();

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");
  Serial.println(__FILE__);
  Serial.println(__DATE__ " " __TIME__);
  Serial.println("DeviceId=  ");

  WiFi.begin("SSIDを入力", "passを入力");
  Azure.begin("接続文字列を入力"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);

  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);

  //流量センサ受信準備
  attachInterrupt(digitalPinToInterrupt(pulsePin), countPulse, RISING);
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
        
    //Customize
    double flow;
    flow = pulseCount * 10.0* 60.0/5.0;  //(L/min) 　5秒間のパルス数*10L/パルス　*60秒/5秒
    pulseCount=0;
    Serial.print("Flow：");
    Serial.print(flow);
    Serial.println(" L/min");

    //Azure送信
    a.setValue("sensor", "flow");    
    a.setValue("espvalue", flow);
    a.setValue("duration", (int)durationtime);
    Azure.push(&a);

    //5秒待ち
    delay(5000);
  
  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);
  }  
}

void ICACHE_RAM_ATTR countPulse() {
  pulseCount++;
}
