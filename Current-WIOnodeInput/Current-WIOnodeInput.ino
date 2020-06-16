#include <AzureIoTHub.h>
#include <Wire.h>//I2C


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

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSIDを入れる", "パスワードを入れる");
  Azure.begin("HostName=から始まる接続コード"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);


  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  //Wire.begin(I2C_SDA,I2C_SCL);


  
}



void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    float sending_data = MeasureCurrent();
    
    Azure.connect();
    DataElement a = DataElement();

    durationtime = (millis() - pretime)/1000;
    pretime = millis();

    Serial.print("duration time: "); 
    Serial.print(durationtime);
    Serial.println(" sec for this loop");


    Serial.println(sending_data);
        
    a.setValue("sensor", "current");
    a.setValue("espvalue", sending_data); 
    a.setValue("duration", (int)durationtime);       
    Azure.push(&a);

    delay(5000); //delay for 30second
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}

float MeasureCurrent(){
  int ad_data[100];
  unsigned long micro_time1 = micros();
  for(int i=0; i<100; i++){
    while(micros() - micro_time1 < 1000){
      delayMicroseconds(10);
    }
    micro_time1 = micros();
    ad_data[i] = (float)analogRead(A0);
  }

//calculate mean
  int max_data = 0;
  int min_data = 1023;
  for(int i=0; i<100; i++){
    if(ad_data[i] > max_data) max_data = ad_data[i];
    if(ad_data[i] < min_data) min_data = ad_data[i];
  }

  int current_amplitude = (max_data - min_data) * 5; // ÷ 2 * 10[mv per unit]
 
  return (float)current_amplitude / 350 ;//y=350xが成り立つ y[A], x[V]
}
