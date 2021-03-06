/*
MKR1000 connecting to IBM Watson IoT Platform

Based on documentation and "recipes" on IBM Bluemix
https://www.ibm.com/cloud-computing/bluemix/watson

Please connect a potentiometer on MKR1000 GND - A1 - Vcc.
Please connect LEDs and 220 Ohm resistors on MKR1000 6 and 7.

Timo Karppinen 9.2.2018
 */

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiSSLClient.h>
#include <MQTTClient.h>   // The Gaehwiler mqtt library

// WLAN 
char ssid[] = "HAMKvisitor"; //  your network SSID (name)
char pass[] = "xxxxxxxxxxx";    // your network password (use for WPA)

// IBM Watson
// Your organization and device needs to be registered in IBM Watson IoT Platform.
// Instruction for registering on page
// https://internetofthings.ibmcloud.com/#

//char *client_id = "d:<your Organization ID>:<your Device Type>:<your Device ID>"; 
char *client_id = "d:yyyyyy:A_MKR1000:xxxxxxxxxxxx";
char *user_id = "use-token-auth";   // telling that authentication will be done with token
char *authToken = "zzzzzzzzzzzzzzzzz"; // Your IBM Watson Authentication Token

//char *ibm_hostname = "<your-org-id.messaging.internetofthings.ibmcloud.com>";
char *ibm_hostname = "yyyyyy.messaging.internetofthings.ibmcloud.com";

const int ainputPin = A1;
const int blinkPin = 7;        // Pin 6 is the on-board LED on MKR1000
const int fadePin = 6;          // must be a pin that supports PWM. 0...8 on MKR1000

// PWM steps per fade step.  More fades faster; less fades slower.
const int pwmStepsPerFade = 14;

int blinkState = 0;
int fadePWM;

/*use this class if you connect using SSL
 * WiFiSSLClient net;
*/
WiFiClient net;
MQTTClient MQTTc;

unsigned long lastMillis = 0;


void setup() 
{
  Serial.begin(9600);
  delay(5000);    // waiting to let you open the serial monitor
  WiFi.begin(ssid, pass);
  Serial.println("Connected to WLAN");
  printWiFiStatus();
  
  /*
    client.begin("<Address Watson IOT>", 1883, net);
    Address Watson IOT: <WatsonIOTOrganizationID>.messaging.internetofthings.ibmcloud.com
    Example:
    client.begin("iqwckl.messaging.internetofthings.ibmcloud.com", 1883, net);
  */
  MQTTc.begin(ibm_hostname, 1883, net);

  connect();
  // Set up the I/O pins
 
  pinMode(blinkPin, OUTPUT);
  digitalWrite(blinkPin, LOW);
  pinMode(fadePin, OUTPUT);
  fadePWM = 255;
  analogWrite(fadePin, fadePWM);   // sets PWM duty cycle

}

void loop() {
   MQTTc.loop();  // Cut for testing without Watson
 
  
  if (blinkState == LOW) 
  {
    digitalWrite(blinkPin, HIGH);
    blinkState = HIGH;
  }
  else
  {
    digitalWrite(blinkPin, LOW);
    blinkState = LOW;
  }
  if (fadePWM > 2*pwmStepsPerFade)
  {
    fadePWM = fadePWM - pwmStepsPerFade;
  }
  analogWrite(fadePin, fadePWM);
  
     // publish a message roughly every  30 second.
     if(millis() - lastMillis > 30000) 
     {
      Serial.println("Publishing to Watson...");
        if(!MQTTc.connected()) {    // Cut for testing without Watson
         connect();                 // Cut for testing without Watson
        }                           // Cut for testing without Watson
        lastMillis = millis();
         //Cut for testing without Watson
         MQTTc.publish("iot-2/evt/SensorsFromLab/fmt/json", "{\"name\":\"Analog input value\",\"bpm\":" + String(analogRead(ainputPin))+"}");  
        fadePWM = 255;
     }
    Serial.print("Voltage  ");
    Serial.println(analogRead(ainputPin));
    delay(1000);
    
// end of loop
}

void connect() 
{
  Serial.print("checking WLAN...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Connecting WLAN...  ");
    Serial.print(".");
    delay(500);
  }
  /*
    Example:
    MQTTc.connect("d:iqwckl:arduino:oxigenarbpm","use-token-auth","90wT2?a*1WAMVJStb1")
    
    Documentation: 
    https://console.ng.bluemix.net/docs/services/IoT/iotplatform_task.html#iotplatform_task
  */
  Serial.print("\nconnecting Watson with MQTT....");
  while (!MQTTc.connect(client_id,user_id,authToken)) 
  {
    Serial.println("Connecting mqtt broker...  ");
    Serial.print(".");
    delay(10000);
  }
  Serial.println("\nconnected!");
}

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
