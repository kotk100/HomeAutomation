#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

float desiredTemp = 22;
float maxTempDif = 0.5;
int lightFloor = 600;
int lightDesired = 750;
int mainPWM = 240; //skoraj minimum
int deskPWM = 240;  //skoraj minimum
boolean clap = false;
boolean menu = false;
int lightMode=3;
char* lightSelect[] = {" OFF", "DESK", "MAIN", " ALL"};
int tempMode = 1;
char* tempSelect[] = {" OFF", "AUTO", "  ON"};
int const down = A3;
int const up = A2; 
int const set = A4;
int const delayGumbi = 200;
int const lightSensor = A0;
int const soundSensor = 2;  
int const occupancy = A1; 
int const bedSensor = 0;
int const tempSensor = 1;
int const heating = 6;
int const mainLight = 5;
int const mainLightPWM = 11;
int const deskLight = 4;
int const deskLightPWM = 3;
int const menuB = A5;
int const forceLight = 2; //clapper 

#define ONE_WIRE_BUS tempSensor
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer = { 0x28, 0xBD, 0xB3, 0x57, 0x04, 0x00, 0x00, 0x92 };

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 7, 9, 10, 12, 13); //RS, E, D4, D5, D6, D7



void setup() {
  pinMode(down, INPUT);
  pinMode(up, INPUT);
  pinMode(set, INPUT);
  pinMode(tempSensor, INPUT);
  pinMode(deskLight, OUTPUT);
  pinMode(deskLightPWM, OUTPUT);
  pinMode(mainLight, OUTPUT);
  pinMode(mainLightPWM, OUTPUT);
  pinMode(heating, OUTPUT);
  pinMode(occupancy, INPUT);
  pinMode(bedSensor, INPUT);
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  sensors.setResolution(insideThermometer, 11);
  //attachInterrupt(0, clapChange, RISING); //menu gumb na pin 2
  
  lcd.clear();
  lcd.print("Temperatura:");
  lcd.setCursor(0,1);
  lcd.print("Osvetljenost:");
  lcd.setCursor(0,2);
  lcd.print("St. Oseb:");
  lcd.setCursor(18,0);
  lcd.print((char)223);
  lcd.print("C");
  
  
  noInterrupts();
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); 
  TCCR2B = _BV(CS22) | _BV(CS21);
  interrupts();
}

void loop() {
  if(digitalRead(menuB))
    changeMenu();
    
  if (menu) {
    menuGlavni(); 
  }
  
  else {
    sensors.requestTemperatures();
    float temp = sensors.getTempC(insideThermometer);
    
    lcd.setCursor(13,0); 
    lcd.print(temp);
    lcd.setCursor(17,1);
    
    (void)analogRead(lightSensor);
    lcd.print(analogRead(lightSensor));
    
    lcd.setCursor(18,2);
    int o=analogRead(occupancy);
    if(o > 950)
      lcd.print(">3");
    else if(o > 700)
      lcd.print(" 3");
    else if(o > 450)
      lcd.print(" 2");
    else if(o > 200)
      lcd.print(" 1");
    else
      lcd.print(" 0");
          
    if(o > 200 && !digitalRead(bedSensor)) { //nekdo je v sobi
      if(digitalRead(forceLight)) {
        tempControl(temp);
        digitalWrite(mainLight, HIGH);
        digitalWrite(deskLight, HIGH);
        OCR2A = 255;  //deskPWM
        OCR2B = 0;    //mainPWM
      }
      else {
        tempControl(temp);
        lightControl();
      }
    }
    else if (digitalRead(bedSensor)){ //če nekdo leži na pojstelji
      digitalWrite(mainLight, LOW);
      digitalWrite(deskLight, LOW);
      digitalWrite(heating, LOW);
      while(digitalRead(bedSensor)){
        //DO NOTHING
      }
      deskPWM = 240;
      mainPWM = 240;
      delay(2000); //počakaj dve sekundi, nato preveri če je nekdo ponovno na postelji
    }
    else { //nikogar ni v sobi
      digitalWrite(mainLight, LOW);
      digitalWrite(deskLight, LOW);
      digitalWrite(heating, LOW);
    }
  }
}

void changeMenu() {
  menu = !menu;
  delay(delayGumbi);
} 

/* KONTROLIRANJE TEMPERATURE IN OSVETLITVE*/
void lightControl() {
  if(analogRead(lightSensor) < lightFloor) {
      switch (lightMode) {
        case 0:
          digitalWrite(mainLight, LOW);
          digitalWrite(deskLight, LOW);
          break;
        case 1:
          digitalWrite(mainLight, LOW);
          digitalWrite(deskLight, HIGH);
          break;
        case 2:
          digitalWrite(mainLight, HIGH);
          digitalWrite(deskLight, LOW);
          break;
        case 3:
          digitalWrite(mainLight, HIGH);
          digitalWrite(deskLight, HIGH);
      }
    }
    if ((lightDesired - analogRead(lightSensor)) > 15) {  // POVEČANJE SVETILNOSTI
      deskPWM =  increaseDecrease(false, false, 255, deskPWM, 2);
      mainPWM =  increaseDecrease(false, false, 255, mainPWM, 2);
    }
    else if ((analogRead(lightSensor) - lightDesired) > 15) { //ZMANJŠEVANJE SVETILNOSTI
      deskPWM = increaseDecrease(true, false, 255, deskPWM, 2);
      mainPWM = increaseDecrease(true, false, 255, mainPWM, 2); 
    }    
    else if (deskPWM > 245 || mainPWM > 245){ //IZKLOP LUČI KO JE SVETILNOST DOVOLJ MAJHNA
      digitalWrite(mainLight, LOW);
      digitalWrite(deskLight, LOW);
    }
    OCR2A = deskPWM;
    OCR2B = (255 - mainPWM); 
}

void tempControl(float temp) {
  switch(tempMode) {
    case 0:
      digitalWrite(heating, LOW);
      break;
      
    case 1:
      if((desiredTemp-temp) >= maxTempDif)
        digitalWrite(heating, HIGH);
      else if((temp-desiredTemp) >= maxTempDif)
        digitalWrite(heating, LOW);
      break;
      
    case 2:
      digitalWrite(heating, HIGH);
  }
}

/* MENIJI ZA LCD*/
void menuGlavni() {
  int i=0;
  lcd.clear();
  lcd.print(" Temperatura");
  lcd.setCursor(0,0);
  lcd.print((char)0x7E);
  lcd.setCursor(0,1);
  lcd.print(" Osvetlitev");
  
  while (menu) {
    if(digitalRead(down)) {
      lcd.setCursor(0,i);
      lcd.write(' ');
      i = increaseDecrease(false, true, 1, i, 1);
      lcd.setCursor(0,i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if(digitalRead(up)) {
      lcd.setCursor(0,i);
      lcd.write(' ');
      i = increaseDecrease(true, true, 1, i, 1);
      lcd.setCursor(0,i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if (digitalRead(set)) {
      delay(delayGumbi); 
      if(i == 0) {
        menuTemperatura();
      }
      else if (i == 1) 
        menuOsvetlitev();
    }
    
    if(digitalRead(menuB))
      changeMenu();
  }
  
  /* NAPIŠI ZA EKRAN, KO NISI V MENIJU, RAZEN UPDATANJA SPREM.*/
  lcd.clear();
  lcd.print("Temperatura:");
  lcd.setCursor(0,1);
  lcd.print("Osvetljenost:");
  lcd.setCursor(0,2);
  lcd.print("St. Oseb:");
  lcd.setCursor(18,0);
  lcd.print((char)223);
  lcd.print("C");
}

void menuTemperatura() {
  int i = 0;
  lcd.clear();
  lcd.write((char)0x7E);
  lcd.print("Zeljena temp.:");
  lcd.setCursor(15,0);
  lcd.print(desiredTemp);
  lcd.setCursor(0,1);
  lcd.print(" Max razlika:");
  lcd.setCursor(17,1);
  lcd.print(maxTempDif);
  lcd.setCursor(0,2);
  lcd.print(" Temp. Mode:");
  lcd.setCursor(16,2);
  lcd.print(tempSelect[tempMode]);
  
  
  while(menu) {
    if(digitalRead(down)) {
      lcd.setCursor(0,i);
      lcd.write(' ');
      i = increaseDecrease(true, true, 2, i, 1);
      lcd.setCursor(0,i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if(digitalRead(up)) {
      lcd.setCursor(0,i);
      lcd.write(' ');
      i = increaseDecrease(false, true, 2, i, 1);
      lcd.setCursor(0,i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if(digitalRead(set)) {
      delay(delayGumbi); 
      if(i == 0) {
        changeDesiredTemperature();
      }
      else if (i == 1) {
        changeMaxTempDiference();
      }
      else if (i == 2) {
        changeTempMode();
      }
    } 
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeDesiredTemperature() {
  lcd.clear();
  lcd.print("Zeljena temperatura:");
  lcd.setCursor(12,2);
  lcd.print((char)223);
  lcd.print("C");
  
  while(menu) {
    lcd.setCursor(7,2);
    lcd.print(desiredTemp);
    
    if(digitalRead(down)) {
      desiredTemp -= 0.5;
      delay(delayGumbi);
    }  
    
    else if(digitalRead(up)) {
      desiredTemp += 0.5;
      delay(delayGumbi);
    }
    
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeMaxTempDiference() {
  lcd.clear();
  lcd.print("Max temp. razlika:");
  
  while(menu) {
    lcd.setCursor(8,2);
    lcd.print(maxTempDif);
    
    if(digitalRead(down)) {
      maxTempDif -= 0.1;
      delay(delayGumbi);
    }  
    
    else if(digitalRead(up)) {
      maxTempDif += 0.1;
      delay(delayGumbi);
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeTempMode() {
  lcd.clear();
  lcd.print("Tempearture mode:");
  
  while(menu) {
    lcd.setCursor(8,2);
    lcd.print(tempSelect[tempMode]);
    
    if(digitalRead(down)) {
        tempMode = increaseDecrease(false, true, 2, tempMode, 1);
        delay(delayGumbi);
      } 
    else if(digitalRead(up)) {
        lightMode = increaseDecrease(true, true, 2, tempMode, 1);
        delay(delayGumbi);
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

void menuOsvetlitev() {
  int i=0;
  lcd.clear();
  lcd.write((char)0x7E);
  lcd.print("Light mode:");
  lcd.setCursor(16,0); //Nov lcd = lcd.setCursor(16,0);
  lcd.print(lightSelect[lightMode]);
  lcd.setCursor(0,1);
  lcd.print(" Light floor:");
  lcd.setCursor(17,1);
  lcd.print(lightFloor);
  lcd.setCursor(0,2);
  lcd.print(" Light desired:");
  lcd.setCursor(17,2);
  lcd.print(lightDesired);
  
  while(menu) {
    if(digitalRead(down)) {
      lcd.setCursor(0, i);
      lcd.write(' ');
      i = increaseDecrease(true, true, 2, i, 1); 
      lcd.setCursor(0, i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if(digitalRead(up)) {
      lcd.setCursor(0, i);
      lcd.write(' ');
      i = increaseDecrease(false, true, 2, i, 1); 
      lcd.setCursor(0, i);
      lcd.write((char)0x7E);
      delay(delayGumbi);
    }
    else if(digitalRead(set)) {
      delay(delayGumbi);
      switch(i) {
        case 0:
          changeLightMode();
          break;
        case 1:
          changeLightFloor();
          break;
        case 2:
          changeLightDesired();
          break;
      }
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeLightMode() {
  while(menu) {
    lcd.setCursor(16,0); //Nov lcd = lcd.setCursor(16,0); //je potrebno vsakič nastavi cursor?
    lcd.print(lightSelect[lightMode]);
    
    if(digitalRead(down)) {
      lightMode = increaseDecrease(false, true, 3, lightMode, 1);
      delay(delayGumbi);
    } 
    else if(digitalRead(up)) {
      lightMode = increaseDecrease(true, true, 3, lightMode, 1);
      delay(delayGumbi);
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeLightFloor() {
   while(menu) {
    lcd.setCursor(17,1);
    lcd.print(lightFloor);
    
    if(digitalRead(down)) {
      lightFloor = increaseDecrease(false, false, lightDesired, lightFloor, 10);
      delay(delayGumbi);
    } 
    else if(digitalRead(up)) {
      lightFloor = increaseDecrease(true, false, lightDesired, lightFloor, 10);
      delay(delayGumbi);
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

void changeLightDesired() {
   while(menu) {
    lcd.setCursor(17,2);
    lcd.print(lightDesired);
    
    if(digitalRead(down)) {
      lightDesired = increaseDecrease(false, true, 1023, lightDesired, 10);
      delay(delayGumbi);
    } 
    else if(digitalRead(up)) {
      lightDesired = increaseDecrease(true, true, 1023, lightDesired, 10);
      delay(delayGumbi);
    }
    if(digitalRead(menuB))
      changeMenu();
  }
}

int increaseDecrease(boolean plus, boolean circle, int maxn, int sprem, int stepn) {
  if(circle) {
    if(plus) {
      if(sprem + stepn <= maxn)
        return (sprem + stepn);
      else
        return sprem = 0;
    }
    else {
      if(sprem - stepn < 0)
        return maxn;
      else
        return (sprem - stepn);  
    }
  }
  else {
    if(plus) {
      if(sprem + stepn <= maxn)
        return (sprem + stepn);
      else
        return maxn;
    }
    else {
      if(sprem - stepn <0)
        return 0;
      else
        return (sprem - stepn);  
    }
  }
}
