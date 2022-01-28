
#include <ESP8266WiFi.h>//esp8266 library
#include <DHT.h> //temp and humidity sensor library
unsigned long prevTime_T1=millis();//using millis function for the pump so that it wont block the sensor readings
long interval_T1=720000;//Running time interval for pump
int pumpstate=LOW;//initial state of pump
unsigned long delayStart =0;
bool delayRunning=false;
 
String apiKey = "WP3NBX8QM1XW8NTC";     //  Enter your Write API key from ThingSpeak
 
const char *ssid =  "Galaxy A30s84CE";     // replace with your wifi ssid and wpa2 key
const char *pass =  "12345677";
const char* server = "api.thingspeak.com";
#define DHTPIN 0          //pin where the dht11 is connected
 
DHT dht(DHTPIN, DHT11);//declaring the model of temp and humidity sensor used
 
WiFiClient client;
 
#define TdsSensorPin A0//pin where temp and humidity sensor is connected to the esp8266
#define VREF 3.3 // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;
void setup()
{
pinMode(TdsSensorPin,INPUT);//temp and humidity sensor
pinMode(4,OUTPUT);//pump
Serial.begin(115200);//initilizing serial communication
delay(10);
dht.begin();
Serial.println("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, pass);
while (WiFi.status() != WL_CONNECTED) 
{
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
}
void loop()
{
static unsigned long analogSampleTimepoint = millis();
if(millis()-analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
{
analogSampleTimepoint = millis();
analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
analogBufferIndex++;
if(analogBufferIndex == SCOUNT)
analogBufferIndex = 0;
}
static unsigned long printTimepoint = millis();
if(millis()-printTimepoint > 800U)
{
printTimepoint = millis();
for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF/ 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
float compensationCoefficient=1.0+0.02*(temperature-25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
float compensationVolatge=averageVoltage/compensationCoefficient; //temperature compensation
tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
//Serial.print("voltage:");
//Serial.print(averageVoltage,2);
//Serial.print("V ");
}
float h = dht.readHumidity();
float t = dht.readTemperature();
if (isnan(h) || isnan(t)) 
{
Serial.println("Failed to read from DHT sensor!");
return;
}
if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
{  
String postStr = apiKey;
postStr +="&field1=";//field 1 in thingspeak is temperature
postStr += String(t);//the float t is read as temperature
postStr +="&field2=";//field 2 in thingspeak is humidity
postStr += String(h);//the float h is read as humidity
postStr +="&field3=";//field 3 in thingspeak is tds(total dissolved solids measured in ppm)
postStr += String(tdsValue,0);//tdsValue is read as tds
postStr += "\r\n\r\n\r\n";
client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n");
client.print(postStr);
Serial.print("Temperature: ");
Serial.print(t);
Serial.print(" degrees Celcius, Humidity: ");
Serial.print(h);
Serial.println("%. Send to Thingspeak.");
Serial.print("TDS Value: ");
Serial.print(tdsValue,0);
Serial.println("ppm");
Serial.println("%. Send to Thingspeak.");
}
client.stop();
Serial.println("Waiting...");
 // thingspeak needs minimum 15 sec delay between updates
delay(100);
unsigned long currentTime =millis();
if(currentTime - prevTime_T1 >interval_T1)
{
digitalWrite(4,HIGH);
delay(180000);//pump will be running for 3 minutes for every 12 minutes gap
digitalWrite(4,LOW);
prevTime_T1=currentTime;
} 
}
int getMedianNum(int bArray[], int iFilterLen)
{
int bTab[iFilterLen];
for (byte i = 0; i<iFilterLen; i++)
bTab[i] = bArray[i];
int i, j, bTemp;
for (j = 0; j < iFilterLen - 1; j++)
{
for (i = 0; i < iFilterLen - j - 1; i++)
{
if (bTab[i] > bTab[i + 1])
{
bTemp = bTab[i];
bTab[i] = bTab[i + 1];
bTab[i + 1] = bTemp;
}
}
}
if ((iFilterLen & 1) > 0)
bTemp = bTab[(iFilterLen - 1) / 2];
else
bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
return bTemp;
}
