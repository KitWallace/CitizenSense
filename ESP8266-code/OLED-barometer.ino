/*********************************************************************
A barometer using an  OLED and libraries from Adafruit


*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#undef BMP280_ADDRESS         // Undef BMP280_ADDRESS from the BMP280 library to easily override I2C address
#define BMP280_ADDRESS (0x76) // Low = 0x76 , High = 0x77 (default on adafruit and sparkfun BME280 modules, default for library)

// OLED 
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
# error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
// barometer
Adafruit_BMP280 bmp; // I2C
int elevation = 60;  // height of station in m

// baro queue
float scale  = 10.0;
int period = 24*60;
int interval = 6;
int entries = period / interval;

// queue - to be class
int qmax = entries;
int qi=0;
int q[1000];

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C(for the 128x64)

  // BMP280 setup
  Serial.println(F("BMP280 test"));
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
 // q setup;
   qinit(0);
   Serial.println(entries);
 
  // init done
  // display splash screen 
  display.display();
  delay(2000);
}

void loop() {
  float temp = bmp.readTemperature();
  float pressure= bmp.readPressure()/ 100.0F; 
  float slpressure = pressure + elevation/9.2F;
  float past3 = bget(180);
  float trend = slpressure - past3;

  display.clearDisplay();
  Serial.println(slpressure);
  bput(slpressure);
 
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print(slpressure);
  display.print("mb ");
  display.print(temp);
  display.print(" C");
  chart();
  display.display();
  delay(interval * 60 * 1000); 
}

void chart() {
  for (int i = 0;i < entries; i=i+2) {
     float b = (bgeti(i) +bgeti(i+1))/2;
     int k = 30 + (b - 1010.0);
     int l = (entries - i)/2;
     if (k > 0) bar(l,63,k,1);
  }
  Serial.println();
}
void bar(int x,int y,int height,int width) {
   display.fillRect(x,y-height,width,height,WHITE);
}


void bput(float baro) {
   qput((int)round(baro*scale));
}

float bget(int t) {
   int k = t / interval; 
   float val = qget(k)/scale;
   return val;
}

float bgeti(int k) {
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

