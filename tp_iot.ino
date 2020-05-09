//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "iotproj-af41e.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "0C23dYUtEBd6RkYsfFsgaCnwDFCDsZHNJb5nahl8"
#define WIFI_SSID "Livebox-33b9"
#define WIFI_PASSWORD "hygst6rtY8rE"

//Define Firebase Data object and paths
FirebaseData firebaseData;
const String room202Ref = "/Rooms/Rm202";
const String classTurnedOnRef = "/Rooms/Rm202/listLighting/Classe";
const String tableauTurnedOnRef = "/Rooms/Rm202/listLighting/Tableau";

// variables pour gérer la lumière de la classe
const int ledClasse = 2;
const int btnLedClasse = 15;
bool btnLedClasseState = LOW;
bool ledClasseState = false;
bool btnLedClassePressed = false;

// variables pour gérer la présence
const int sensor = 14;
bool sensorState = LOW; // we start, assuming no motion detected
bool val = 0; // variable for reading the pin status

// variables pour gérer le timer
unsigned long ts1;
unsigned long ts2;

void setup() {
  // setup firebase
  Serial.begin(9600);  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.print("connecting"); 
  while (WiFi.status() != WL_CONNECTED) { 
   Serial.print("."); 
   delay(500); 
  } 
  Serial.println(); 
  Serial.print("connected: "); 
  Serial.println(WiFi.localIP()); 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 

  // initialisation de la broche 2 comme étant une sortie
  pinMode(ledClasse, OUTPUT);
  pinMode(btnLedClasse, INPUT);
  pinMode(sensor, INPUT);
}

void loop() {
  //observeFirebaseChanges();
  observeSensor();
  observeBtnClasse();
}

void observeSensor() {
  val = digitalRead(sensor);  // read input value
  if (val == HIGH) { // check if the input is HIGH
    ts1 = millis();
    if (sensorState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      sensorState = HIGH;
      
      FirebaseJson json;
      json.set("presence", sensorState);
      Firebase.updateNode(firebaseData, room202Ref, json);
    }
  } else {
    ts2 = millis();
    unsigned long elapsedTime = ts2 - ts1;
    if (sensorState == HIGH && elapsedTime >= 10000){
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      sensorState = LOW;

      FirebaseJson json;
      json.set("presence", sensorState);
      Firebase.updateNode(firebaseData, room202Ref, json);
    }
  }
}

void observeBtnClasse() {  
  btnLedClasseState = digitalRead(btnLedClasse);
  if (btnLedClasseState == HIGH) {
       if (!btnLedClassePressed) {
          ledClasseState = !ledClasseState;
          FirebaseJson json;
          json.set("turnedOn", ledClasseState);
          Firebase.updateNode(firebaseData, classTurnedOnRef, json);
          btnLedClassePressed = true;
       }
  } else {
       btnLedClassePressed = false;
       observeFirebaseChanges();
  } 
}

void observeFirebaseChanges() {

  bool firebasevalue;
  bool res = Firebase.getBool(firebaseData, "/Rooms/Rm202/listLighting/Classe/turnedOn", firebasevalue);
  
  if (res) {
      ledClasseState = firebasevalue;
      digitalWrite(ledClasse, !ledClasseState);
  }
}
