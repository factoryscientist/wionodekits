const uint8_t BUZZER_PIN = 5; // sig pin of the buzzer
const uint8_t PORT_POWER = 15; // (common with RED_LED)

void setup()
{
  pinMode(PORT_POWER, OUTPUT);
  digitalWrite(PORT_POWER, HIGH);

  pinMode(BUZZER_PIN, OUTPUT);
    
}

void loop()
{
    tone(BUZZER_PIN, 440);
    delay(500);
    noTone(BUZZER_PIN);
    delay(2000);
}
 
