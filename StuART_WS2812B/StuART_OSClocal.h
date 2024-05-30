#include <WiFiUdp.h>
#include <OSCMessage.h>
//#include <OSCBundle.h>
WiFiUDP Udp;
/// DEFINE BUILTIN LED TEST CONTROL  
unsigned int ledState = LOW;              // LOW means led is *on*
#define BUILTIN_LED LED_BUILTIN           // LED de control



/*// VARIABLES TIME
int TIME_LOOP = 50;             // delay de la boucle loop
int TIME_COUNT = 0;             // temp mesur� entre chaque envoi
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;*/

void openOSCport(){//////// SETUP UDP LISTEN
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port listen : ");
/* #ifdef ESP32
  Serial.println(localPort);
#else */
  Serial.println(Udp.localPort());
/* #endif */
}

// FONCTION POOL REQUEST
void PoolREQUEST(OSCMessage &msg){ 
    // vous pouvez récupérer l'adresse IP attribuée en utilisant WiFi.localIP()
    IPAddress myIP = WiFi.localIP();
    // Convertissez l'adresse IP en une chaîne de caractères (char[])
    char ipCharArray[16]; // Une adresse IP v4 est composée de 15 caractères au maximum (par exemple, "192.168.1.100\0")
    myIP.toString().toCharArray(ipCharArray, sizeof(ipCharArray));
    Serial.println("Pool_Request");

    if (msg.getInt(0) == 1) {
      OSCMessage msg("/Pool_REPLY");
      msg.add(DEVICE_NAME);
      msg.add("IN");      msg.add(ipCharArray);      msg.add(localPort);
      msg.add("OUT");      msg.add(IP_CIBLE);      msg.add(OUT_PORT);
      Udp.beginPacket(IP_BROADCAST, OUT_PORT);
      msg.send(Udp);      Udp.endPacket();      msg.empty();
      }
}

void led(OSCMessage &msg) {         // control de la Led BUILTIN pour test CONNECTION
  ledState = msg.getInt(0);
  digitalWrite(BUILTIN_LED, ledState);
  Serial.print("/led: ");
  Serial.println(ledState);
}

/*void time_loop(OSCMessage &msg) {     //REDEFINIT LA DUREE DE LA BOUCLE LOOP
  TIME_LOOP = msg.getInt(0);
  Serial.print("/time_loop: ");
  Serial.println(TIME_LOOP);
}*/

/*void count_time(){ //compteur d'interval de temps REEL DE LOOP
  currentMillis = millis();
  TIME_COUNT = currentMillis - previousMillis; // temp mesur� entre chaque envoi
  previousMillis = currentMillis;
}*/

void ipcible(OSCMessage &msg) {     //REDEFINIT LA cible des messages OSC
  char ipcible[16];
  for(int i = 0; i < 4 ; i++){ipcible[i]=msg.getInt(i);}
  snprintf(IP_CIBLE, sizeof(IP_CIBLE), "%d.%d.%d.%d", ipcible[0], ipcible[1], ipcible[2], ipcible[3]); // 
}

/* //RECEPTION OSC
void oscreceiver(){                                 
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {                    // ROUTE OSC
      bundle.dispatch("/led", led);
      bundle.dispatch("/time_loop", time_loop);
      //bundle.dispatch("/test", test);
      bundle.dispatch("/POOL_REQUEST", PoolREQUEST);
      bundle.dispatch("/DIMMER", set_dimmer); // /DIMMER 0-255
      bundle.dispatch("/IPCIBLE", ipcible);
      bundle.dispatch("/ALUM",set_valeursfixes_tableOSC); // direct value /ALUM numPixMin numPixMax R G B
      //bundle.dispatch("/FADE",set_fade_value); // fade un pixel /FADE numpix time(ms) R G B
      bundle.dispatch("/FADE_SERIE",set_fade_serie); // fade une s�rie de pixel     
} 
    else {
      error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
 }                                          //END RECEPTION OSC
*/
// FONCTION HelloWorld
void HelloWorld(){ 
    delay(random(50, 5000));
    // vous pouvez récupérer l'adresse IP attribuée en utilisant WiFi.localIP()
    IPAddress myIP = WiFi.localIP();
    // Convertissez l'adresse IP en une chaîne de caractères (char[])
    char ipCharArray[16]; // Une adresse IP v4 est composée de 15 caractères au maximum (par exemple, "192.168.1.100\0")
    myIP.toString().toCharArray(ipCharArray, sizeof(ipCharArray));
    //Serial.println("Pool_Request");
      String addressOSC = "/helloworld";
      addressOSC = DEVICE_NAME + addressOSC;
      OSCMessage msg(addressOSC.c_str());
      msg.add(ipCharArray);
      msg.add(localPort);
      Udp.beginPacket(IP_CIBLE, OUT_PORT);
      msg.send(Udp);      Udp.endPacket();      msg.empty();
}
void reply_helloworld(OSCMessage &msg){
   HelloWorld();
   Serial.println("HelloWorld");
}
