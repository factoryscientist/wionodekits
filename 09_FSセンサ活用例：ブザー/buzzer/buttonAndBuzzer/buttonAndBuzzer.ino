#define FUNC_BTN    0
#define BUZZER_PIN  5    /* sig pin of the buzzer */
const uint8_t PORT_POWER = 15; // (common with RED_LED)
//set PORT_POWER as HIGH for power supply. Low means no power.

int length = 15;         /* the number of notes */
char notes[] = "ccggaagffeeddc ";
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;
 
void setup()
{
    /* set buzzer pin as output */
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(FUNC_BTN, INPUT);
    pinMode(PORT_POWER, OUTPUT);
    digitalWrite(PORT_POWER, HIGH);

}
 
void loop()
{
    for(int i = 0; i < length; i++) {
        if(notes[i] == ' ') {
            delay(beats[i] * tempo);
        } else {
            if ( LOW == digitalRead(FUNC_BTN) ){
              playNote(notes[i], beats[i] * tempo);
            }
        }
        delay(tempo / 2);    /* delay between notes */
    }
 
}
 
/* play tone */
void playTone(int tone, int duration) {
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(tone);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(tone);
    }
}
 
void playNote(char note, int duration) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
 
    // play the tone corresponding to the note name
    for (int i = 0; i < 8; i++) {
        if (names[i] == note) {
            playTone(tones[i], duration);
        }
    }
}
