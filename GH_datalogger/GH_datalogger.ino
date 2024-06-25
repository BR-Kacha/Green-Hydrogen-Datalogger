
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#define RX 2
#define TX 3
#define ledPin 6
//#define sensorPin A0
#define currentpin A1  //for hydrogen
const int CURRENT_SENSOR_PIN = A0;  //for current

float voltage = 0.0;
const float MAX_CURRENT = 5.0; // Amps
String AP = "";       // AP NAME
String PASS = ""; // AP PASSWORD
String API = "";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
float valSensor = 1.000;
SoftwareSerial esp8266(RX,TX); 

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  pinMode(ledPin, OUTPUT);
  pinMode(currentpin,INPUT);
  digitalWrite(ledPin, LOW);
  pinMode(CURRENT_SENSOR_PIN, INPUT);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Hydrogen Gas:");
}

void loop() {
 valSensor = readSensorVoltage(); //if greater than 550, than hydrogen present

 String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor)+"&field2="+String(getCurrentValue());
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);delay(750);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}

// function of hydrogen sensor
float readSensorVoltage() {

  unsigned int sensorValue = analogRead(currentpin);  // Read the analog value from sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Convert analog reading to voltage (assuming 5V reference)
  unsigned int outputValue = map(sensorValue, 0, 1023, 0, 255); // map the 10-bit data to 8-bit data
  
  if (outputValue > 65) {
    analogWrite(ledPin, outputValue); // generate PWM signal
    lcd.setCursor(0, 1);
    lcd.print("                "); //16 spaces
    lcd.setCursor(0, 1);
    lcd.print("Detected");
  }
  else {
    digitalWrite(ledPin, LOW);
    lcd.setCursor(0, 1);
    lcd.print("                "); //16 spaces
    lcd.setCursor(0, 1);
    lcd.print("Absent");
  }
  Serial.println(sensorValue);
  Serial.println(voltage);
  return voltage;  // Return voltage value
}

float getCurrentValue(){
  float analogvalue = analogRead(CURRENT_SENSOR_PIN);
 /* for (int i=0; i<=10000; i++)
  {
      voltage = voltage + ((analogvalue / 1023.0) * MAX_CURRENT);
  }
  voltage = voltage/10000;
  delay(100);*/
  voltage =  ((analogvalue / 1023.0) * MAX_CURRENT);
  float current = (voltage - 2.48) / 0.185 *1000.0;
  Serial.print("Current in mA: ");
  Serial.println(current);
  return (current);
  //delay(500);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
