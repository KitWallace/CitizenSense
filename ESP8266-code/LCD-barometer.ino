#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#undef BMP280_ADDRESS         // Undef BMP280_ADDRESS from the BMP280 library to easily override I2C address
#define BMP280_ADDRESS (0x76) // Low = 0x76 , High = 0x77 (default on adafruit and sparkfun BME280 modules, default for library)

// display
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

//  custom characters

byte downright[8] = {
  B00000,
  B10000,
  B01000,
  B00101,
  B00011,
  B00111,
  B00000,
  B00000
};  

byte upright[8] = {
  B00000,
  B00111,
  B00011,
  B00101,
  B01000,
  B10000,
  B00000
};  

byte backslash[8] = {
  B00000,
  B10000,
  B01000,
  B00100,
  B00010,
  B00001,
  B00000,
  B00000,
};
byte slopedown[8] = {
  B00000,
  B00000,
  B11000,
  B00100,
  B00011,
  B00000,
  B00000,
  B00000
};  

byte slopeup[8] = {
  B00000,
  B00000,
  B00011,
  B00100,
  B11000,
  B00000,
  B00000,
  B00000
};  
byte uparrow[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000,
  B00000
};

byte downarrow[8] = {
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000,
  B00000
};

// barometer
Adafruit_BMP280 bmp; // I2C
int elevation = 60;  // height of station in m

// baro queue
float scale  = 10.0;
int period = 24*60;
int interval = 5;
int entries = period / interval;

// queue - to be class
int qmax = entries;
int qi=0;
int q[1000];

void setup()   
{
  Serial.begin(9600);
// display setup
  lcd.begin(16,2);         // initialize the lcd for 16 chars 2 lines and turn on backlight
  lcd.createChar(0, slopeup);
  lcd.createChar(1, slopedown);
  lcd.createChar(2, upright);
  lcd.createChar(3, downright);
  lcd.createChar(4, uparrow);
  lcd.createChar(5, downarrow);
  lcd.createChar(6, backslash);

// BMP280 setup
  Serial.println(F("BMP280 test"));
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  delay(5000); //let BMP settle
// q setup;
   qinit(0);
   Serial.println(entries);
}/*--(end setup )---*/

void loop()  
{  lcd.clear();
   float temp = bmp.readTemperature();
   lcd.setCursor(0,0); 
   lcd.print(temp);
   lcd.print(" C ");
   lcd.setCursor(0,1);

   float pressure= bmp.readPressure()/ 100.0F; 
   float slpressure = pressure + elevation/9.2F;
   float past3 = bget(180);
   float trend = slpressure - past3;
   lcd.print(slpressure);

// store this reading
   bput(slpressure);

   printgraph(3*60, 8);
   delay(interval * 60 * 1000); 
}

void bput(float baro) {
   qput((int)round(baro*scale));
}

float bget(int t) {
   int k = t / interval; 
   float val = qget(k)/scale;
   return val;
}

void qinit(int val) {
    for (int i=0; i < qmax;i++)
       q[i] = val;
}
void qput(int val) {
    q[qi]=val;
    qi = (qi + 1) % qmax;
}
int qget(int i) {
    int j = (qi-i-1+qmax)% qmax;
    return q[j];
}

byte trendaschar(float trend) {
  byte d;
  if (trend <=-4.5) d=byte(5); else     // down arrow
  if (trend <=-3.0) d=byte(3); else     // down right
  if (trend <=-1.5) d=byte(6); else     // backslash
  if (trend <-0.2)  d=byte(1); else     // slopedown
  if (trend <=0.2)  d=0x2D; else        // dash
  if (trend <1.5)   d=byte(0); else     // slope up
  if (trend <3.0)   d=0x2F; else        // slash
  if (trend <4.5)   d=byte(2); else     // upright
  d=byte(4);                            // uparrow
  return d;
}

void printgraph(int period, int points) {
   int k = points - 1;
   for (int i =k;i>0;i--) {
     float a = bget((i-1)*period);
     float b = bget(i*period);
     if (a > 0 && b > 0) {  // both are actual readings 
       float trend = a - b;
       lcd.write(trendaschar(trend));
     }
     else lcd.print(" ");
   }
}
