#include <AzureIoTHub.h>
#include <Wire.h>//I2C

//customize

//

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


//


void setup() {
  Serial.begin(115200);
  WiFi.begin("SSIDを入れる", "パスワードを入れる");
  Azure.begin("HostName=から始まる接続キーを入れる"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);


  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);

  //I2C
  //Wire.begin(I2C_SDA,I2C_SCL);

  //Customize area

  //
  
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

    //Customize
    if(digitalRead(PORT1B)){
      float button_data = digitalRead(PORT1B);

       Serial.println("button pushed!\t");


      //
    
      a.setValue("espvalue", button_data);
      Azure.push(&a);
      delay(2000);
    }else{
       //Serial.println("button is off.\t");
    }
    
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }  
}
