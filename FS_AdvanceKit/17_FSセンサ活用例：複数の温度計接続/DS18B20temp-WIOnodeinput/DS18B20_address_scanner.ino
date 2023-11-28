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
DeviceAddress temp;

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Dallas Temperature IC Control Library Demo");

   // IO周りの初期化
  pinMode(PORT1B, INPUT);
  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(5); 

  sensors.begin();
  sensors.getAddress(temp,0);

 for(int j=0;j<5;j++){
 digitalWrite(BLUE_LED, LOW);
  
  for(int i=0;i<8;i++){
    Serial.print("0x");
    Serial.print(temp[i],HEX);
    Serial.print(",");
  }
 
  Serial.println();
  sensors.requestTemperatures();
  Serial.println(sensors.getTempC(temp));
  digitalWrite(BLUE_LED, HIGH);
  delay(500);
 }
}

void loop(void)
{ 
}
