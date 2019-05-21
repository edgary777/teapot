// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include <EEPROM.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

/*
 * Setup function. Here we do the basics
 */

int relayPin = 3;

int ledLow = 6;
int ledMedium = 7;
int ledHigh = 8;
int button = 9;
int touch = 10;

float currentTemp = 0;
float previousTemp = -1824.00;
float setpoint = EEPROM.read(1);
int tolerance = 2;

long long timer = 0;
long btnTimer = 0;
long timeDelay = 1000;

int activeLed = EEPROM.read(2);


boolean buttonActive = false;
boolean changeTemperature = false;
long buttonTimer = 0;
long longPressTime = 1000;


boolean blinker = false;
boolean ledBlinkerState;
long long ledBlinkerTimer;
long blinkerDelay = 50;


void setup(void) {
  pinMode(ledLow, OUTPUT);
  pinMode(ledMedium, OUTPUT);
  pinMode(ledHigh, OUTPUT);

  digitalWrite(ledLow, HIGH);
  digitalWrite(ledMedium, HIGH);
  digitalWrite(ledHigh, HIGH);

  pinMode(button, INPUT);
  pinMode(touch, INPUT);
  pinMode(relayPin, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 12);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}

void relayOn() {
  digitalWrite(relayPin, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("RELAY ON");
}

void relayOff() {
  digitalWrite(relayPin, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("RELAY OFF");
}

// function to print the temperature for a device
float printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.println(tempC);
  Serial.print("Setpoint: ");
  Serial.println(setpoint);
  return tempC;
}

void touchState() {
//  if (millis() >= btnTimer){
//    if (digitalRead(touch) == HIGH) {
  if (changeTemperature == true){
    changeTemperature = false;
    if (setpoint == 120) {
      setpoint = 0;
      activeLed = 4;
      EEPROM.write(1, 0);
      EEPROM.write(2, 4);
    } else if (setpoint == 90) {
      setpoint = 120;
      activeLed = 3;
      EEPROM.write(1, 120);
      EEPROM.write(2, 3);
    } else if (setpoint == 80) {
      setpoint = 90;
      activeLed = 2;
      EEPROM.write(1, 90);
      EEPROM.write(2, 2);
    } else if (setpoint == 0){
      setpoint = 80;
      activeLed = 1;
      EEPROM.write(1, 80);
      EEPROM.write(2, 1);        
    } else {
      setpoint = 80;
      activeLed = 1;
      EEPROM.write(1, 80);
      EEPROM.write(2, 1);
    }
  }
}
//  }
//}

void touchPress(){
  if (digitalRead(touch) == HIGH) {
    if (millis() >= btnTimer){
      Serial.println("buttonPressed");
      btnTimer = millis() + 1500;
      blinker = true;
      if (buttonActive == false) {
        buttonActive = true;
        buttonTimer = millis();
      }
      if ((millis() - buttonTimer > longPressTime) && (changeTemperature == false)) {
        Serial.println("TemperatureChanged");
        changeTemperature = true;
      }
    }
  } else if (millis() < btnTimer || digitalRead(touch) == LOW) {
    blinker = false;
    if (buttonActive == true) {
      if (changeTemperature == true) {
        changeTemperature = false;
      }
      buttonActive = false;
    }
  }
}


void ledSwitcher(){

  if(activeLed == 0){
   digitalWrite(ledLow, LOW);
   digitalWrite(ledMedium, LOW);
   digitalWrite(ledHigh, LOW);
  } else {
    if(activeLed == 1){
      if (blinker == true){
        if (millis() >= ledBlinkerTimer){
          if (ledBlinkerState == false){
            ledBlinkerState = true;
            digitalWrite(ledLow, LOW);
            ledBlinkerTimer += blinkerDelay * 3;
          } else {
            ledBlinkerState = false;
            digitalWrite(ledLow, HIGH);
            ledBlinkerTimer += blinkerDelay;
          }
        }
      } else {
        digitalWrite(ledLow, LOW);
      }
      digitalWrite(ledMedium, HIGH);
      digitalWrite(ledHigh, HIGH);
    } else if(activeLed == 2){
      if (blinker == true){
        if (millis() >= ledBlinkerTimer){
          if (ledBlinkerState == false){
            ledBlinkerState = true;
            digitalWrite(ledMedium, LOW);
            ledBlinkerTimer += blinkerDelay * 2;
          } else {
            ledBlinkerState = false;
            digitalWrite(ledMedium, HIGH);
            ledBlinkerTimer += blinkerDelay;
          }
        }
      } else {
        digitalWrite(ledMedium, LOW);
      }
      digitalWrite(ledLow, HIGH);
      digitalWrite(ledHigh, HIGH);
    } else if(activeLed == 3){
      if (blinker == true){
        if (millis() >= ledBlinkerTimer){
          if (ledBlinkerState == false){
            ledBlinkerState = true;
            digitalWrite(ledHigh, LOW);
            ledBlinkerTimer += blinkerDelay * 2;
          } else {
            ledBlinkerState = false;
            digitalWrite(ledHigh, HIGH);
            ledBlinkerTimer += blinkerDelay;
          }
        }
      } else {
        digitalWrite(ledHigh, LOW);
      }
      digitalWrite(ledLow, HIGH);
      digitalWrite(ledMedium, HIGH);
    } else if(activeLed == 4){
      if (blinker == true){
        if (millis() >= ledBlinkerTimer){
          if (ledBlinkerState == false){
            ledBlinkerState = true;
            digitalWrite(ledLow, LOW);
            digitalWrite(ledMedium, LOW);
            digitalWrite(ledHigh, LOW);
            ledBlinkerTimer += blinkerDelay * 2;
          } else {
            ledBlinkerState = false;
            digitalWrite(ledLow, HIGH);
            digitalWrite(ledMedium, HIGH);
            digitalWrite(ledHigh, HIGH);
            ledBlinkerTimer += blinkerDelay;
          }
        }
      } else {
        digitalWrite(ledLow, HIGH);
        digitalWrite(ledMedium, HIGH);
        digitalWrite(ledHigh, HIGH);
      }
    }
  }
}


void updateLed(){
  if (setpoint == 120) {
    activeLed = 3;
  } else if (setpoint == 90) {
    activeLed = 2;
  } else if (setpoint == 80) {
    activeLed = 1;
  } else if (setpoint == 0) {
    activeLed = 4;
  }

}


void loop(void) {
  touchPress();
  touchState();
  ledSwitcher();
  if (millis() >= timer) {
    // call sensors.requestTemperatures() to issue a global temperature 
    // request to all devices on the bus
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");

    // It responds almost immediately. Let's print out the data
    currentTemp = printTemperature(insideThermometer); // Use a simple function to print out the data
    if (currentTemp != -127.00){
      if (previousTemp == -1824.00){
        previousTemp = currentTemp;      
      }
      float result = currentTemp - previousTemp;
      if (abs(result) >= 10){
        currentTemp = -127.00;
      } else {
        previousTemp = currentTemp;
      }
    }
    if (currentTemp <= -127.00){
      Serial.println("ERROR READING SENSOR");
      activeLed = 0;
      ledSwitcher();      
      timer = millis() + timeDelay;
      relayOff();
    } else {
      if(activeLed == 0){
        updateLed();
      }
      if (currentTemp <= setpoint - tolerance) {
        relayOn();
      } else if (currentTemp >= setpoint) {
        relayOff();
      }
      timer = millis() + timeDelay;
    }
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
