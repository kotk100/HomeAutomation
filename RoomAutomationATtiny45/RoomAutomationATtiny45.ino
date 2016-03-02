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


int vhodni=A3; //pin 2, vhodni laser, ki s prvi sproži ob vhodu v sobo
int izhodni=A2; //pin 3, izhodni laser
int st=0; //števec oseb v sobi
int posredovano = 1; //pin 5, pwm izhod vrednosti glavnemu sistemu
int piezo = 0;
long delayMS; //čas za čakanje na drug laser
int b=1;
int nivo = 800;

void setup() { //nastavitev vhodov in izhodov
  pinMode(vhodni, INPUT);
  pinMode(izhodni, INPUT);
  pinMode(posredovano, OUTPUT);
  pinMode(piezo, OUTPUT);
}

void loop() {
  delayMS=6000;
  if(analogRead(vhodni) < nivo) {
    while((0 < delayMS) && b) { 
      if(analogRead(izhodni) < nivo) {
        st++;
        changeOutput();
        b=0;
        digitalWrite(piezo, HIGH);
        delay(10);
        digitalWrite(piezo, LOW);
        break;
      }
      
      delayMS--;
    }
    delay(250); 
  }
  
  if(analogRead(vhodni) >= nivo) b=1;
  
 if(analogRead(izhodni) < nivo) {
    
    while((0 < delayMS) && b) {  //koliko časa je optimalno čakati?
      if(analogRead(vhodni) < nivo) {
        if(st > 0)
          st--;
        else
          st = 0;
        changeOutput();
        b=0;
        digitalWrite(piezo, HIGH);
        delay(10);
        digitalWrite(piezo, LOW);
        break;
      }
      delayMS--;
      //delayMicroseconds(250);
    }
    delay(250); //kakšen delay med osebami?
  }
  
  if(analogRead(izhodni) >= nivo) b=1;
}

void changeOutput() {
  if (st == 0)
    analogWrite(posredovano, 0);
  else if (st == 1)
    analogWrite(posredovano, 62);
  else if (st == 2)
    analogWrite(posredovano, 124);
  else if (st == 3)
    analogWrite(posredovano, 187);
  else if (st > 3)
    analogWrite(posredovano, 255);
}
