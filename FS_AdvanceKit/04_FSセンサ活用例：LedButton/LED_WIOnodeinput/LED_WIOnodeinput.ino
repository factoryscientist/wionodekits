#include <AzureIoTHub.h>

const uint8_t PORT0A = 1;
const uint8_t PORT0B = 3;
const uint8_t PORT1A = 4;
const uint8_t PORT1B = 5;
const uint8_t PORT_POWER = 15; // (common with RED_LED)
const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

//ボタン側とledの設定
const int ledPin = PORT1B;
const int buttonPin = PORT1A;
int ledState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//データ送信のタイミング
unsigned long prev;
unsigned long interval;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("*** Wake up WioNode...");

  WiFi.begin("SSID" , "PASS");    // 2.4GHz帯のWiFiを指定すること
  Azure.begin("IotHubkey");   //YourKey Example:""
  Azure.setCallback(azureCallback);

  pinMode(FUNC_BTN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  delay(5);
  prev = 0;
  interval = 5000; //データ送信頻度
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

    int reading = digitalRead(buttonPin);
    //ボタンが押されたら
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          ledState = !ledState;
        }
        else if (buttonState == LOW) {
          ledState = HIGH;
        }
      }
    }
    // ボタンの状態に合わせてLEDをオンオフする。
    int LED = digitalRead(ledPin);
    if (ledState != LOW) {
      digitalWrite(ledPin, HIGH);
    }
    else {
      digitalWrite(ledPin, ledState);
    }
    lastButtonState = reading;

    //Azureへデータを送る設定
    unsigned long curr = millis();
    int duration = interval / 1000;

    if ((curr - prev) >= interval) {
      Serial.print("LED_STATE: ");
      Serial.println(LED);
      Serial.print("durationtime: ");
      Serial.println(duration);
      a.setValue("LED", "condition");
      a.setValue("EspValue", LED);
      a.setValue("duration", duration);
      Azure.push(&a);
      prev = curr;
    }

  }
  else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}
