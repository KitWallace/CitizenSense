#define ADC_REF 3.3  // Reference voltage
#define ADC A0   // Analog input

#define STATSON D7 // take this pin high to turn on stats 

// Sensor 
String sensorName = "Angle";

// sampling
int samples = 25;
int delayms = 1000;

void setup() 
{
  Serial.begin(9600); 

  delay(100);  // sensor startup delay
  
  Serial.println(); 
  Serial.println(sensorName);
}


void loop() {
  if (digitalRead(STATSON))
       gatherStats();
  else getSample();
  delay(delayms);
}

void gatherStats() {
  float sum=0;
  float sum1=0;  
  float sum2=0;
  float value, k;
  k = asValue(getVoltage()); 
  Serial.println(String("k = ") + k);
  delay(delayms); 
  for(int i =0; i<samples; i++) {
      float voltage = getVoltage();
      value =asValue(voltage);
      Serial.println(String("value = ") + value);
      sum += value;
      sum1 += (value - k);
      sum2 += (value - k)* (value - k);
      delay(delayms); 
  }
  float mean = sum / samples ;
  float stddev = sqrt((sum2 - (sum1 * sum1) / samples) / (samples - 1));
  Serial.println(String("Statistics for ") + samples + " samples");
  Serial.println(String("mean =")+ mean + " StdDev= " + stddev + " 95% interval = " + (mean - 2 * stddev) + "," + (mean+ 2 * stddev));
  if (digitalRead(LOGON) ==1 )
      log_data(String("average=")+mean+"&stddev="+stddev);

}

void getSample() {
      float voltage = getVoltage();
      float value =asValue(voltage);
      Serial.println(String("voltage = ") + voltage + " value=" +value);
}

float getVoltage()
{
  int sensor_value;
  sensor_value = analogRead(ADC);   
  float voltage = (float)sensor_value*ADC_REF/1024;
  return voltage;
}

float asValue(float voltage) {
    return voltage*270/ADC_REF;  // 270 degrees full turn
}

