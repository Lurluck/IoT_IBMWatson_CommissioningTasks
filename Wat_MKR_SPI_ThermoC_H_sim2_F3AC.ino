﻿
/*
MKR1000 connecting to IBM Watson IoT Platform

Based on documentation and "recipes" on IBM Bluemix
https://www.ibm.com/cloud-computing/bluemix/watson
Timo Karppinen 19.2.2017

Modified for testing SPI thermocouple board Digilent PmodTC1
Please connect
MKR1000 - PmodTC1
GND - 5 GND
Vcc - 6 Vcc
9 SCK - 4 SCK
10 MISO - 3 MISO
2 - 1 SS

Thermocouple data on the SPI
D31 - sign
D30 ....D18 - 13 bits of temperature data
D16 - normally FALSE. TRUE if thermocouple input is open or shorted to GND or VCC
D15 ... D0 - reference junction temperature

The reference junction compensation is calculted in the IC. no need to calculate here.

Timo Karppinen 25.1.2018
 */

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiSSLClient.h>
#include <MQTTClient.h>   // The Gaehwiler mqtt client

// WLAN 
//char ssid[] = "Moto_Z2_TK"; //  your network SSID (name)
//char pass[] = "xxxxxxxxxx";    // your network password (use for WPA)

char ssid[] = "HAMKvisitor"; //  your network SSID (name)
char pass[] = "xxxxxxxxx";    // your network password (use for WPA)

//char ssid[] = "Nelli";
//char pass[] = "xxxxxxxxxx";

// IBM Watson
// Your organization and device needs to be registered in IBM Watson IoT Platform.
// Instruction for registering on page
// https://internetofthings.ibmcloud.com/#

//char *client_id = "d:<your Organization ID>:<your Device Type>:<your Device ID>"; 
char *client_id = "d:yyyyyyyy:A_MKR1000:sim2_F3AC";
char *user_id = "use-token-auth";   // telling that authentication will be done with token
char *authToken = "xxxxxxxxxxxxx"; // Your IBM Watson Authentication Token

//char *ibm_hostname = “your-org-id.messaging.internetofthings.ibmcloud.com”;
char *ibm_hostname = "yyyyyyyyyy.messaging.internetofthings.ibmcloud.com";

// sensors and LEDS
const int LEDPin = LED_BUILTIN;     // must be a pin that supports PWM. 0...8 on MKR1000
// PModTC1
const int thermoCS = 3;         // chip select for MIC3 SPI communication
int thermoByte0 = 0;           // 8 bit data from TC1 board
int thermoByte1 = 0;           // 8 bit data from TC1 board
int thermoByte2 = 0;           // 8 bit data from TC1 board
int thermoByte3 = 0;           // 8 bit data from TC1 board
int temp14bit = 0;             // 14 most significant bits on a 32 bit integer
int tempRaw = 0;
float tempScaledF = 0;


int blinkState = 0;

/*use this class if you connect using SSL
 * WiFiSSLClient net;
*/
WiFiClient net;
MQTTClient MQTTc;

unsigned long lastSampleMillis = 0;
unsigned long previousWiFiBeginMillis = 0;
unsigned long lastWatsonMillis = 0;
unsigned long lastPrintMillis = 0;


void setup() 
{
  pinMode(thermoCS, OUTPUT);
  digitalWrite(thermoCS, HIGH);   // for not communicating with MIC3 at the moment
  Serial.begin(9600);
  delay(2000); // Wait for wifi unit to power up
  WiFi.begin(ssid, pass);
  delay(5000); // Wait for WiFi to connect
  Serial.println("Connected to WLAN");
  printWiFiStatus();
  
  /*
    client.begin("<Address Watson IOT>", 1883, net);
    Address Watson IOT: <WatsonIOTOrganizationID>.messaging.internetofthings.ibmcloud.com
    Example:
    client.begin("iqwckl.messaging.internetofthings.ibmcloud.com", 1883, net);
  */
  MQTTc.begin(ibm_hostname, 1883, net);  // Cut for testing without Watson

  connect();

  SPI.begin();
  // Set up the I/O pins
 
  pinMode(thermoCS, OUTPUT);
  pinMode(LEDPin, OUTPUT);


}

void loop() {
   MQTTc.loop();  // Cut for testing without Watson
 

  // opening and closing SPI communication for reading TC1
  if(millis() - lastSampleMillis > 500)
  { 
    lastSampleMillis = millis();
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    digitalWrite(thermoCS, LOW);
  
    thermoByte0 = SPI.transfer(0x00);
    thermoByte1 = SPI.transfer(0x00);
    thermoByte2 = SPI.transfer(0x00);
    thermoByte3 = SPI.transfer(0x00);
  
    digitalWrite(thermoCS, HIGH);
    SPI.endTransaction();


    thermoByte0 = thermoByte0 << 24;
    thermoByte1 = thermoByte1 << 16;
    
    
    temp14bit =( thermoByte0 | thermoByte1 );
    
    tempRaw = temp14bit/262144; // shifting 18 bits to right gives multiply of 0,25 degree C.
    tempScaledF = float(temp14bit/262144)/4;

  }

  // Print on serial monitor once in 1000 millisecond
  if(millis() - lastPrintMillis > 1000)
  {
    Serial.print("temp14bit   ");
    Serial.println(temp14bit, BIN);
    Serial.print("  tempRaw  ");
    Serial.println(tempRaw, BIN);
    Serial.print("  tempScaledF  ");
    Serial.println(tempScaledF);
 
    lastPrintMillis = millis();
  }
  
     // publish a message every  30 second.
     if(millis() - lastWatsonMillis > 30000) 
     {
      Serial.println("Publishing to Watson...");
        if(!MQTTc.connected()) {    // Cut for testing without Watson
         connect();                 // Cut for testing without Watson
        }                           // Cut for testing without Watson
        lastWatsonMillis = millis();
         //Cut for testing without Watson
   //variable values as string!// MQTTc.publish("iot-2/evt/TemperatureTC1/fmt/json", "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":\"" + String(tempScaledF)+"\", \"TempStreightDF48\": \"" + String(temp14bit)+"\"}}");  
    //ok    MQTTc.publish("iot-2/evt/TemperatureTC1/fmt/json", "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":12345, \"TempStreightDF48\": \"" + String(temp14bit)+"\"}}");  
    //not   MQTTc.publish("iot-2/evt/TemperatureTC1/fmt/json", "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":float(tempScaledF), \"TempStreightDF48\": \"" + String(temp14bit)+"\"}}");  
    //ok     MQTTc.publish("iot-2/evt/TemperatureTC1/fmt/json", "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":34.567, \"TempStreightDF48\": \"" + String(temp14bit)+"\"}}");  
   
    //ok    String wpayload = "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":1234, \"TempStreightDF48\":5678}}";
    //ok    String wpayload = "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":123.4, \"TempStreightDF48\":567.8}}";
            String wpayload = "{\"d\":{\"TemperatureSensor\":\"TC1 \",\"TempScaledF3AC\":" + String(tempScaledF)+ ", \"TempStreightDF48\":" + String(temp14bit)+"}}";
         
          MQTTc.publish("iot-2/evt/TemperatureTC1/fmt/json", wpayload);
     
     }
   
    delay(1);
    
// end of loop
}

void connect() 
{
  Serial.print("checking WLAN...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");       // printing a dot every half second
    if ( millis() - previousWiFiBeginMillis > 5000) // reconnecting
    {
      previousWiFiBeginMillis = millis();
      WiFi.begin(ssid, pass);
      delay(5000); // Wait for WiFi to connect
      Serial.println("Connected to WLAN");
      printWiFiStatus();
    }
    delay(500);
    
  }
  /*
    Example:
    MQTTc.connect("d:iqwckl:arduino:oxigenarbpm","use-token-auth","90wT2?a*1WAMVJStb1")
    
    Documentation: 
    https://console.ng.bluemix.net/docs/services/IoT/iotplatform_task.html#iotplatform_task
  */
  
  Serial.print("\nconnecting Watson with MQTT....");
  // Cut for testing without Watson
  while (!MQTTc.connect(client_id,user_id,authToken)) 
  {
    Serial.print(".");
    delay(3000);
  }
  Serial.println("\nconnected!");
}

// messageReceived subroutine needs to be here. MQTT client is calling it.
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
