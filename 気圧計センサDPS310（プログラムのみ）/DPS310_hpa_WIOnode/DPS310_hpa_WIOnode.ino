#include <AzureIoTHub.h>
#include <Wire.h>
#include <Dps310.h>

// Dps310 Opject
Dps310 Dps310PressureSensor = Dps310();
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
  WiFi.begin("SSID", "password");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("Azure IoT Hub の接続文字列");   //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
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

  // ----------
  // DPS310の初期化
  Wire.begin(I2C_SDA, I2C_SCL);
 
  while (!Serial);

  //Call begin to initialize Dps310PressureSensor
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  //Dps310PressureSensor.begin(Wire, 0x76);
  //Use the commented line below instead to use the default I2C address.
  Dps310PressureSensor.begin(Wire);

  //temperature measure rate (value from 0 to 7)
  //2^temp_mr temperature measurement results per second
  int16_t temp_mr = 2;
  //temperature oversampling rate (value from 0 to 7)
  //2^temp_osr internal temperature measurements per result
  //A higher value increases precision
  int16_t temp_osr = 2;
  //pressure measure rate (value from 0 to 7)
  //2^prs_mr pressure measurement results per second
  int16_t prs_mr = 2;
  //pressure oversampling rate (value from 0 to 7)
  //2^prs_osr internal pressure measurements per result
  //A higher value increases precision
  int16_t prs_osr = 2;
  //startMeasureBothCont enables background mode
  //temperature and pressure ar measured automatically
  //High precision and hgh measure rates at the same time are not available.
  //Consult Datasheet (or trial and error) for more information
  int16_t ret = Dps310PressureSensor.startMeasureBothCont(temp_mr, temp_osr, prs_mr, prs_osr);
  //Use one of the commented lines below instead to measure only temperature or pressure
  //int16_t ret = Dps310PressureSensor.startMeasureTempCont(temp_mr, temp_osr);
  //int16_t ret = Dps310PressureSensor.startMeasurePressureCont(prs_mr, prs_osr);


  if (ret != 0)
  {
    Serial.print("Init FAILED! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.println("Init complete!");
  }
  delay(5); 
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
    
    //Customize
    // センサ値の取得および出力
    digitalWrite(BLUE_LED, LOW);

  uint8_t pressureCount = 20;
  float pressure[pressureCount];
  float press_avg = 0;
  float press_avg_h = 0;
  uint8_t temperatureCount = 20;
  float temperature[temperatureCount];
  float temp_avg = 0;

  //This function writes the results of continuous measurements to the arrays given as parameters
  //The parameters temperatureCount and pressureCount should hold the sizes of the arrays temperature and pressure when the function is called
  //After the end of the function, temperatureCount and pressureCount hold the numbers of values written to the arrays
  //Note: The Dps310 cannot save more than 32 results. When its result buffer is full, it won't save any new measurement results
  int16_t ret = Dps310PressureSensor.getContResults(temperature, temperatureCount, pressure, pressureCount);

  if (ret != 0)
  {
    Serial.println();
    Serial.println();
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.println();
    Serial.println();
    Serial.print(temperatureCount);
    Serial.println(" temperature values found: ");
    for (int16_t i = 0; i < temperatureCount; i++)
    {
      Serial.print(temperature[i]);
      Serial.println(" degrees of Celsius");
      temp_avg = temp_avg + temperature[i];
    }
    temp_avg = temp_avg / temperatureCount;
    Serial.print("気温の平均：");
    Serial.println(temp_avg);

    Serial.println();
    Serial.print(pressureCount);
    Serial.println(" pressure values found: ");
    for (int16_t i = 0; i < pressureCount; i++)
    {
      Serial.print(pressure[i]);
      Serial.println(" Pascal");
      press_avg = press_avg + pressure[i];
    }
    press_avg = press_avg / pressureCount;
    press_avg_h = press_avg / 100; //ヘクトパスカルに変換
    Serial.print("気圧の平均：");
    Serial.println(press_avg);
    Serial.print("気圧の平均 (Hpa)：");
    Serial.println(press_avg_h);
 
    // Azureへ送るデータの用意
    a.setValue("Sensor", "temp");    
    a.setValue("EspValue", temp_avg);
    a.setValue("Sensor", "press");    
    a.setValue("EspValue", press_avg);
    a.setValue("Sensor", "press_h");    
    a.setValue("EspValue", press_avg_h);
    
    // Azure IoT Hub へプッシュ
    Azure.push(&a);

    // wait中は青LEDを消灯する
    digitalWrite(BLUE_LED, HIGH);

  }

  //Wait some time, so that the Dps310 can refill its buffer
  delay(10000);
  } else {
    Serial.print("Not connected to the Internet:");
    Serial.println(WiFi.status());
    delay(250);

    if(50 <= fail_count++){
      ESP.restart();
    }
  }
}
