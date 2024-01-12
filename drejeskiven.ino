/*
kode til thing dev kortet i drejeskivetelefonen.
*/

// Diverse konstanter og andet - læsning af drejeskivetelefon
int needToPrint = 0;
int count;
int in = 5; // den pin der læser drejeskiven
int handleset = 13; // den pin der læser håndsættet
String resultat = "";    // Den variabel jeg bruger til at gemme resultatet

int lastState = LOW;
int trueState = LOW;
long lastStateChangeTime = 0;
int cleared = 0;

int dialHasFinishedRotatingAfterMs = 100;
int debounceDelay = 10;

// vi skal også have noget der signalerer liv
unsigned long previousMillis = 0;
int interval = 5000;

#include "EspMQTTClient.h"

EspMQTTClient client(
  "navn på trådløst net",
  "password til trådløst net",
  //"IP-adresse til MQTT brokeren",  // MQTT Broker server ip
  "URL til MQTT broker",  // MQTT Broker server ip
  "Klientens brugernavn på MQTT serveren",   // Can be omitted if not needed
  "Klientens password",   // Can be omitted if not needed
  "Klientens navn",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void setup()
{
  Serial.begin(115200);
  pinMode(in, INPUT);
  pinMode(handleset, INPUT);
  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("drejeskiven/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Publish a message to "homeassistant/test"
  client.publish("drejeskiven/status", "Drejeskiven er online"); // You can activate the retain flag by setting the third parameter to true
}

void enableMQTTPersistence();


void loop(){
  client.loop();
  unsigned long currentMillis = millis();
  
  if(digitalRead(handleset)){
    int reading = digitalRead(in);

    if((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs){
    // the dial isn’t being dialed, or has just finished being dialed.
      if (needToPrint) {
        // if it’s only just finished being dialed, we need to send the number down the serial
        // line and reset the count. We mod the count by 10 because ‘0’ will send 10 pulses.
        resultat = resultat + (String) (count % 10);
        Serial.println(resultat);
        //Serial.print(count % 10, DEC);
        needToPrint = 0;
        count = 0;
        cleared = 0;
      }
    }
    if (reading != lastState) {
      lastStateChangeTime = millis();
    }
    if ((millis() - lastStateChangeTime) > debounceDelay) {
      // debounce – this happens once it’s stablized
      if (reading != trueState) {
        // this means that the switch has either just gone from closed->open or vice versa.
        trueState = reading;
        if (trueState == HIGH) {
          // increment the count of pulses if it’s gone high.
          count++;
          needToPrint = 1; // we’ll need to print this number (once the dial has finished rotating)
        }
      }
    }
    lastState = reading;
  }
  
  // herunder, hvis røret er lagt på, og der er et resultat - print det.
  // Det er også hvad vi skal have sendt pr mqtt
  if(!digitalRead(handleset) & resultat != ""){
    Serial.print("resultatet: ");
    Serial.println(resultat);
    client.publish("drejeskiven", resultat);
  
    resultat  = "";
    }
  
  if((unsigned long)(currentMillis - previousMillis) >= interval){
    // der er gået interval tid siden vi sidst gjorde det her
    // lad os gøre det!
    Serial.println("Drejeskiven er i live");
    client.publish("drejeskiven/status", "1");
    previousMillis = currentMillis;
  }
   
 

}
