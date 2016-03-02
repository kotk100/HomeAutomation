/*
* 1-Reset
* 2-pin 3 (Analog 3)
* 3-pin 4 (Analog 2)
* 4-GND
* 5-pin 0 (pwm)
* 6-pin 1 (pwm)
* 7-pin 2(analog 1)
* 8-VCC+
*/

int const microphone = 3;
int const output = 4;
boolean out = false;

void setup() {
  pinMode(microphone, INPUT);
  pinMode(output, OUTPUT);
  
  digitalWrite(output, LOW);
}

void loop() {
  if(digitalRead(microphone)) {
    delay(100);
    unsigned long time = millis();
    while(millis() - time < 700) {
      if (digitalRead(microphone)) {
        changeOutput();
        delay(100);
      }
    }
  }
}

void changeOutput() {
  if(out) {
    digitalWrite(output, LOW);
    out = false;
  }
  else {
    digitalWrite(output, HIGH);
    out = true;
  }
}
