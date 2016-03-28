#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

const int calibratePin = D5;
 
// Raw Ranges:
// initialize to first reading and allow calibration to
// find the minimum and maximum for each axis
int xRawMin,xRawMax,yRawMin,yRawMax,zRawMin,zRawMax;
 
// Take multiple samples to reduce noise when calibrating
const int sampleSize = 10;

const int intervalms = 2000;

void setup() 
{
  Serial.begin(9600);

  ads.setGain(GAIN_ONE); 
  Serial.println(String("gain = ") + ads.getGain());
  ads.begin();
  delay(1000);

// initialize  to saved limits
   if (digitalRead(calibratePin) == LOW)
        setInitialLimits();
   else setStoredLimits();
}
 
void loop() 
{
  int xRaw = readAxis(0);
  int yRaw = readAxis(1);
  int zRaw = readAxis(2);
  
  if (digitalRead(calibratePin) == LOW) {
    autoCalibrate(xRaw, yRaw, zRaw);
   
    Serial.print("Raw Ranges: X: ");
    Serial.print(xRawMin);
    Serial.print("-");
    Serial.print(xRawMax);
    
    Serial.print(", Y: ");
    Serial.print(yRawMin);
    Serial.print("-");
    Serial.print(yRawMax);
    
    Serial.print(", Z: ");
    Serial.print(zRawMin);
    Serial.print("-");
    Serial.print(zRawMax);
    Serial.println();
    
    Serial.print(xRaw);
    Serial.print(", ");
    Serial.print(yRaw);
    Serial.print(", ");
    Serial.print(zRaw);
  }
  
  // Convert raw values to 'milli-Gs"
    
  long xMilliG = map(xRaw, xRawMin, xRawMax, -1000, 1000);
  long yMilliG = map(yRaw, yRawMin, yRawMax, -1000, 1000);
  long zMilliG = map(zRaw, zRawMin, zRawMax, -1000, 1000);
  
  // re-scale to gs and negated  
  float xG = xMilliG / 1000.0;
  float yG = yMilliG / 1000.0;
  float zG = zMilliG / 1000.0; 

  Serial.println(String("Accelerations :")+xG+"G, "+yG+ "G, "+ zG +"G");
  // convert to pitch and roll 

  float pitch = getPitch(xG,yG,zG);
  float roll = getRoll(xG,yG,zG);
  Serial.println(String("roll=")+roll+" pitch= "+pitch);
    
  delay(intervalms);

}

float asDegree(float radians) {
    return radians * 180.0 / 3.141592;
}

int signum(float x) {
   if (x >= 0) return 1; else return -1;
}

float getPitch(float xG, float yG,float zG) {
   return asDegree(atan2(xG, sqrt(yG*yG + zG*zG))); 
}

float getRoll(float xG, float yG,float zG) {
    float mu= 0.01;
    return asDegree(atan2(yG, signum(zG) * sqrt(zG*zG + mu*xG*xG))); 
}

int readAxis(int i) {
  return ads.readADC_SingleEnded(i);
}

//
// Read "sampleSize" samples and return the average
//
int sampleAxis(int axisPin)
{ 
    long sum=0;
    for (int i = 0; i<sampleSize;i++) {
       int reading = readAxis(axisPin);
       sum += reading;
       delay(20);
    }
    return sum/sampleSize;
}
 
//
// Find the extreme raw readings from each axis
//

void setInitialLimits() {
// initialize to first reading
// equal min and max in map causes a fault
    xRawMin = sampleAxis(0); xRawMax=xRawMin+1;
    yRawMin = sampleAxis(1); yRawMax=yRawMin+1;
    zRawMin = sampleAxis(2); zRawMax=zRawMin+1;
}
void setStoredLimits() {
// use previously calibrated values
  xRawMin = 10521; xRawMax = 15735;
  yRawMin = 10352; yRawMax = 15589;
  zRawMin = 10616; zRawMax = 15813; 
}
void autoCalibrate(int xRaw, int yRaw, int zRaw)
{
  Serial.println("Calibrate");
  if (xRaw < xRawMin)
  {
    xRawMin = xRaw;
  }
  if (xRaw > xRawMax)
  {
    xRawMax = xRaw;
  }
  
  if (yRaw < yRawMin)
  {
    yRawMin = yRaw;
  }
  if (yRaw > yRawMax)
  {
    yRawMax = yRaw;
  }
 
  if (zRaw < zRawMin)
  {
    zRawMin = zRaw;
  }
  if (zRaw > zRawMax)
  {
    zRawMax = zRaw;
  }
}
