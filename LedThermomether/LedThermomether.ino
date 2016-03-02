#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//#include <OneWire.h>
#include <math.h>
#include <EEPROM.h>
#include <dht.h>

#define LED_STRIP_TEMP_PIN 9
#define LED_STRIP_HUM_PIN 10
//#define DS18B20_PIN 8
#define DHT22_PIN 11
#define TEMP_RANGE_START 15
#define LEDS_PER_DEGREE 2
#define PERCENTAGE_PER_LED 3.33333f //humidity
#define STYLE_NUMBER 5
#define BUTTON_DELAY 200
#define NEXT_PIN 2 //unused
#define PREVIOUS_PIN 3 //unused
#define SET_PIN 4
#define RED_LED A5
#define GREEN_LED A4
#define BLUE_LED A3

//Define all stylesTemp as RGB values (gradients)
byte stylesTemp[STYLE_NUMBER][2][3][3] = {{{{255, 255, 0}, {255, 0, 0}, {255, 0, 0}}    //Yellow to Red, leading with Red
                                       ,{{0, 0, 255}, {11, 165, 203}}}              //Blue to Light blue
                                       ,
                                       {{{148, 235, 0}, {229, 15, 0}, {0, 0, 0}}    //Green to Red
                                       ,{{0, 0, 0}, {0, 0, 0}}}                     //Off
                                       ,
                                       {{{0, 0, 0}, {255, 255, 255}, {255, 255, 255}} //White to White, leading with White
                                       ,{{0, 0, 0}, {0, 0, 0}}}                       //Off
                                       ,
                                       {{{255, 0, 0}, {255, 0, 0}, {0, 255, 0}}     //Red, leading with Green
                                       ,{{0, 0, 255}, {0, 0, 255}}}                 //Blue
                                       ,
                                       {{{0, 255, 0}, {0, 255, 0}, {0, 0, 0}}     //Custom setting
                                       ,{{0, 0, 255}, {0, 0, 255}}}};

byte stylesHum[STYLE_NUMBER][2][3][3] = {{{{11, 165, 203}, {0, 0, 255}, {0, 0, 255}}    //Yellow to Red, leading with Red
                                       ,{{255, 0, 0}, {255, 255, 0}}}              //Blue to Light blue
                                       ,
                                       {{{148, 235, 0}, {229, 15, 0}, {0, 0, 0}}    //Green to Red
                                       ,{{0, 0, 0}, {0, 0, 0}}}                     //Off
                                       ,
                                       {{{0, 0, 0}, {255, 255, 255}, {255, 255, 255}} //White to White, leading with White
                                       ,{{0, 0, 0}, {0, 0, 0}}}                       //Off
                                       ,
                                       {{{255, 0, 0}, {255, 0, 0}, {0, 255, 0}}     //Red, leading with Green
                                       ,{{0, 0, 255}, {0, 0, 255}}}                 //Blue
                                       ,
                                       {{{0, 255, 0}, {0, 255, 0}, {0, 0, 0}}     //Custom setting
                                       ,{{0, 0, 255}, {0, 0, 255}}}};

                                       
byte currentStyle = 0;

//Create strips
// number of pixels in strip, Arduino pin number, pixel type flags
Adafruit_NeoPixel stripTemp = Adafruit_NeoPixel(30, LED_STRIP_TEMP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripHum = Adafruit_NeoPixel(30, LED_STRIP_HUM_PIN, NEO_GRB + NEO_KHZ800);

// 1000 uF capacitor across power leads, add 300 - 500 Ohm resistor on first pixel's data input

// DS18B20 Temperature chip i/o
//OneWire ds(DS18B20_PIN);
//DHT22 sensor
dht DHT;  

float temp; //For storing current temperature
float humidity;

void setup() {
  pinMode(SET_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  // Initialize and set all pixels to 'off'
  stripTemp.begin();
  stripHum.begin();
  stripTemp.show(); 
  stripHum.show();

  //Get first data from sensor
  startUpAndTest(); //Test the strip, cycle RGB VALUES
  
  //setTempResolution(); //set DS18B20 to 10-bit resolution

  //Get saved stylesTemp in EEPROM
  readCustomStyle();

  //Attach interrupts
  attachInterrupt(0, nextStyle, RISING); //interrupt on pin 2
  attachInterrupt(1, previousStyle, RISING); //interrupt on pin 3
}

void loop() {
  getDataDHT22(); //Get data from sensor
  setColorForTemp(); //Set LED strip color
  setColorForHum();

  //Update strips
  stripTemp.show();
  stripHum.show();
  
  delay(300); //wait before reading the data again

  //Check for button press
  if(digitalRead(SET_PIN))
    setCostumStyle();
}

//Sets colors on strip for current temperature
void setColorForTemp(){
  //Calculate RGB incremets for each LED
  float increment[3] = {(stylesTemp[currentStyle][0][1][0]-stylesTemp[currentStyle][0][0][0])/30.0, (stylesTemp[currentStyle][0][1][1]-stylesTemp[currentStyle][0][0][1])/30.0, (stylesTemp[currentStyle][0][1][2]-stylesTemp[currentStyle][0][0][2])/30.0};
  
  uint16_t i;
  //Set the appropriate portion of the strip to first RGB gradient
  for(i=0; i < ((temp-TEMP_RANGE_START)*LEDS_PER_DEGREE)-1; i++) {
    stripTemp.setPixelColor(i, stripTemp.Color(stylesTemp[currentStyle][0][0][0]+round(i*increment[0]), stylesTemp[currentStyle][0][0][1]+round(i*increment[1]), stylesTemp[currentStyle][0][0][2]+round(i*increment[2])));
  }

  //Set leading LED according to current Style. If the RGB value is set to 0, it will not be set any differently than other LEDs
  if(stylesTemp[currentStyle][0][2][0] == 0 && stylesTemp[currentStyle][0][2][1] == 0 && stylesTemp[currentStyle][0][2][2] == 0)
    stripTemp.setPixelColor(i, stripTemp.Color(stylesTemp[currentStyle][0][0][0]+round(i*increment[0]), stylesTemp[currentStyle][0][0][1]+round(i*increment[1]), stylesTemp[currentStyle][0][0][2]+round(i*increment[2])));
  else
    stripTemp.setPixelColor(i, stripTemp.Color(stylesTemp[currentStyle][0][2][0], stylesTemp[currentStyle][0][2][1], stylesTemp[currentStyle][0][2][2]));
  i++;

  //Calculate RGB increments for second portion of strip
  increment[0] = (stylesTemp[currentStyle][1][1][0]-stylesTemp[currentStyle][1][0][0])/30.0;
  increment[1] = (stylesTemp[currentStyle][1][1][1]-stylesTemp[currentStyle][1][0][1])/30.0;
  increment[2] = (stylesTemp[currentStyle][1][1][2]-stylesTemp[currentStyle][1][0][2])/30.0;

  //Set the rest of the strip
  for(; i < stripTemp.numPixels(); i++) {
    stripTemp.setPixelColor(i, stripTemp.Color(stylesTemp[currentStyle][1][0][0]+round(i*increment[0]), stylesTemp[currentStyle][1][0][1]+round(i*increment[1]), stylesTemp[currentStyle][1][0][2]+round(i*increment[2])));
  }
}

//Sets colors on strip for current temperature
void setColorForHum(){
  //Calculate RGB incremets for each LED
  float increment[3] = {(stylesHum[currentStyle][0][1][0]-stylesHum[currentStyle][0][0][0])/30.0, (stylesHum[currentStyle][0][1][1]-stylesHum[currentStyle][0][0][1])/30.0, (stylesHum[currentStyle][0][1][2]-stylesHum[currentStyle][0][0][2])/30.0};
  
  uint16_t i;
  //Set the appropriate portion of the strip to first RGB gradient
  for(i=0; i < (humidity/PERCENTAGE_PER_LED)-1; i++) {
    stripHum.setPixelColor(i, stripHum.Color(stylesHum[currentStyle][0][0][0]+round(i*increment[0]), stylesHum[currentStyle][0][0][1]+round(i*increment[1]), stylesHum[currentStyle][0][0][2]+round(i*increment[2])));
  }

  //Set leading LED according to current Style. If the RGB value is set to 0, it will not be set any differently than other LEDs
  if(stylesHum[currentStyle][0][2][0] == 0 && stylesHum[currentStyle][0][2][1] == 0 && stylesHum[currentStyle][0][2][2] == 0)
    stripHum.setPixelColor(i, stripHum.Color(stylesHum[currentStyle][0][0][0]+round(i*increment[0]), stylesHum[currentStyle][0][0][1]+round(i*increment[1]), stylesHum[currentStyle][0][0][2]+round(i*increment[2])));
  else
    stripHum.setPixelColor(i, stripHum.Color(stylesHum[currentStyle][0][2][0], stylesHum[currentStyle][0][2][1], stylesHum[currentStyle][0][2][2]));
  i++;

  //Calculate RGB increments for second portion of strip
  increment[0] = (stylesHum[currentStyle][1][1][0]-stylesHum[currentStyle][1][0][0])/30.0;
  increment[1] = (stylesHum[currentStyle][1][1][1]-stylesHum[currentStyle][1][0][1])/30.0;
  increment[2] = (stylesHum[currentStyle][1][1][2]-stylesHum[currentStyle][1][0][2])/30.0;

  //Set the rest of the strip
  for(; i < stripHum.numPixels(); i++) {
    stripHum.setPixelColor(i, stripHum.Color(stylesHum[currentStyle][1][0][0]+round(i*increment[0]), stylesHum[currentStyle][1][0][1]+round(i*increment[1]), stylesHum[currentStyle][1][0][2]+round(i*increment[2])));
  }
}

//Read humidity and temperature from DHT22 sensor
void getDataDHT22(){
  //Get data
  int chk = DHT.read22(DHT22_PIN);

  //Check for errors
  switch (chk){
    case DHTLIB_OK:
        break;
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    case DHTLIB_ERROR_CONNECT:
    case DHTLIB_ERROR_ACK_L:
    case DHTLIB_ERROR_ACK_H:
        return;
        break;
  }

  //Update the newly read data
  temp = DHT.temperature;
  humidity = DHT.humidity;
}

/*//Read temperature from DS18B20 sensor
boolean getTemperature(){
  byte i;
  byte data[12];
  
  ds.reset();
  ds.skip();
  // Start conversion
  ds.write(0x44, 1);
  // Wait some time...
  delay(200);
  ds.reset();
  ds.skip();
  // Issue Read scratchpad command
  ds.write(0xBE);
  // Receive 9 bytes
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Calculate temperature value
  temp = ( (data[1] << 8) + data[0] )*0.0625;
  return true;
}*/

/*void setTempResolution(){
  ds.reset();
  ds.skip();
  ds.write(0x4E);         // Write scratchpad
  ds.write(0);            // TL
  ds.write(0);            // TH
  ds.write(0x3F);         // Configuration Register, Set resolution to 10-bit
  
  ds.write(0x48);         // Copy Scratchpad
  ds.reset();
}*/

//Test all colors
void startUpAndTest(){
  for(int j = 0; j < 110; j++){
    int i = 0;
    
    for (; (i < j && i < 30); i++)
      stripTemp.setPixelColor(i, stripTemp.Color(sin(0.3*(j-i)) * 127 + 128, sin(0.3*(j-i) + 2) * 127 + 128, sin(0.3*(j-i) + 4) * 127 + 128));
    
    for(; (i < 60 && i < j);i++)
      stripHum.setPixelColor(60-i, stripHum.Color(sin(0.3*(j-i)) * 127 + 128, sin(0.3*(j-i) + 2) * 127 + 128, sin(0.3*(j-i) + 4) * 127 + 128));
    
    delay(50);
    stripTemp.show();
    stripHum.show();
  }
}

byte a, b, c;
bool editMode = false;
void nextStyle(){
  //increase RGB component if in edit style mode
  if(editMode){
    while(digitalRead(NEXT_PIN) && stylesTemp[STYLE_NUMBER-1][a][b][c] != 255){
      stylesTemp[STYLE_NUMBER-1][a][b][c] += 1;
      setColorForTemp();
      stripTemp.show();
    }
  }     //change to next style
  else if(!editMode && currentStyle != STYLE_NUMBER-1){
    currentStyle += 1;
    setColorForTemp();
    stripTemp.show();
    while(digitalRead(NEXT_PIN)){}
    delay(BUTTON_DELAY);
  }
}

void previousStyle(){
  //decrease RGB component if currently in edit mode
  if(editMode){
    while(digitalRead(PREVIOUS_PIN) && stylesTemp[STYLE_NUMBER-1][a][b][c] != 0){
      stylesTemp[STYLE_NUMBER-1][a][b][c] -= 1;
      setColorForTemp();
      stripTemp.show();
    }
  }     //change to previous style
  else if(!editMode && currentStyle != 0){
    currentStyle -= 1;
    setColorForTemp();
    stripTemp.show();
    while(digitalRead(PREVIOUS_PIN)){}
    delay(BUTTON_DELAY);
  }
}

void setCostumStyle(){
  noInterrupts();
  editMode = true;  

  //set strip to full brightness to indicate edit mode has been entered
  for(int i = 0; i < 30; i++)
    stripTemp.setPixelColor(i, stripTemp.Color(255, 255, 255));
  stripTemp.show();

  //wait till the button is depressed
  while(digitalRead(SET_PIN)){}

  //set custom style and display it, enable interrupts
  currentStyle = STYLE_NUMBER-1;
  setColorForTemp();
  stripTemp.show();
  interrupts();

  //Set all the values for custom style
  setCustomColor(0, 0);
  setCustomColor(0, 1);
  setCustomColor(0, 2);
  setCustomColor(1, 0);
  setCustomColor(1, 1);

  //exit edit mode
  editMode = false;
}

//Set RGB value for custom style at location x,y in array stylesTemp
void setCustomColor(byte x, byte y){
  a = x;
  b = y;
  for(byte i=0; i < 3; i++){
    switch(i){
      case 0:
        digitalWrite(RED_LED, HIGH);
        break;
      case 1:
        digitalWrite(GREEN_LED, HIGH);
        break;
      case 2:
        digitalWrite(BLUE_LED, HIGH);
    }
    
    c = i;
    while(!digitalRead(SET_PIN)){
      //wait, interrupts will set the value
    }
    while(digitalRead(SET_PIN)){}
    delay(BUTTON_DELAY);
    EEPROM.write((a*3+b)*3+i, stylesTemp[STYLE_NUMBER-1][a][b][i]);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
  }

  for(int i = 0; i < 30; i++)
    stripTemp.setPixelColor(i, stripTemp.Color(255, 255, 255));
  stripTemp.show();
  delay(500);
  setColorForTemp();
  stripTemp.show();
}

//Read the custom style stored in EEPROM from last sesion
void readCustomStyle(){
  readCustomRGBValue(0, 0);
  readCustomRGBValue(0, 1);
  readCustomRGBValue(0, 2);
  readCustomRGBValue(1, 0);
  readCustomRGBValue(1, 1);   
}

//read RGB values from EEPROM
void readCustomRGBValue(byte a, byte b){
  stylesTemp[STYLE_NUMBER-1][a][b][0] = EEPROM.read((a*3+b)*3); 
  stylesTemp[STYLE_NUMBER-1][a][b][1] = EEPROM.read((a*3+b)*3+1); 
  stylesTemp[STYLE_NUMBER-1][a][b][2] = EEPROM.read((a*3+b)*3+2); 
}
