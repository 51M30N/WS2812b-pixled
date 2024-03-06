/*
ADAPTED BY 51M30N
contact@lacon.dev
*/


#include <Wire.h>

char DEVICE_NAME[] = "StuART_WS2812B/StripPixLed";                                   ////note ici le nom du code et de la version ESP8266_$capteur-$version
String completeAddress = String("/WS2812B/") + DEVICE_NAME; 
//OSCErrorCode error;
#include 'secret_ssid.h' //
#include 'StuART_OSC.h' //


// DEFS DE LA LEDSTRIP
#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 96 // Nombre de LEDs dans la bande
// Connexions pour l'ESP8266 LEDSTRIP
#define DATAPIN D5   // ou un autre pin approprié
Adafruit_NeoPixel PIX_LEDs(NUMPIXELS, DATAPIN, NEO_GRB + NEO_KHZ800);

// VARIABLES DE LEDSTRIP
//int pix; // 
//boolean buttonState = 0;
//int colormax = 255; //max 
int freq = 25; // en ms, vitesse de rafraîchissement
//byte decrement_table[NUMPIXELS]; // chaîne de valeur de color[0] = 0 à <=colormax
byte valeursfixes_table[NUMPIXELS][3]; //
float fade_time[NUMPIXELS]; //
float fade_value[NUMPIXELS][3]; //
float fade_value_calcul[NUMPIXELS][3]; //
float fade_delta[NUMPIXELS][3]; //
float detla[3];
//int lenght_decrement_table; // 
//int lenght_snake = 10; // longueur de la chaîne
byte dimmer = 200;

void setup() { ///////////////////////////////SETUP///////////////////////
  
// SETUP BUILTINLED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led
  
/// SERIAL INITIALISATION
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
   
  wificonnection();
  openOSCport();

  
////SETUP DE LEDSTRIP
  PIX_LEDs.setBrightness(dimmer); 
  PIX_LEDs.begin();
  PIX_LEDs.show();
  set_valeursfixes_table(0, (NUMPIXELS - 1), 0, 0, 0);  // INITIALISATION DES LEDS A ZERO
}
///////////////////////////////END SETUP///////////////////////






///////////////////////////////LOOP///////////////////////
void loop() {  
oscreceiver(); // écoute du port OSC d'entree
count_time(); // calcul du temps écoulé depuis la dernière mesure
// WAIT A MOMENT  
    calcul_fade_value();
    send_valeursfixes_table();  
    PIX_LEDs.show();
    delay(freq); 
} 
///////////////////////////////END LOOP///////////////////////



void oscreceiver(){                                 //RECEPTION OSC
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {                               // ROUTE OSC
      bundle.dispatch("/led", led);
      bundle.dispatch("/time_loop", time_loop);
      //bundle.dispatch("/test", test);
      bundle.dispatch("/IPCIBLE", ipcible);
      bundle.dispatch("/POOL_REQUEST", PoolREQUEST);
      bundle.dispatch("/DIMMER", set_dimmer); // /DIMMER 0-255
      bundle.dispatch("/ALUM",set_valeursfixes_tableOSC); // direct value /ALUM numPixMin numPixMax R G B
      bundle.dispatch("/FADE_SERIE",set_fade_serie); // fade une série de pixel 
      // /FADE_SERIE nombreDePixels pix1 pix2 ... pixn time(ms) R G B
    } 
    else {
      /* error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error); */
    }
  }
 }                                                  //END RECEPTION OSC
 



////////////FONCTIONS DE LEDSTRIP

void write_pixel(int index, byte red, byte green, byte blue) {
  PIX_LEDs.setPixelColor(index, PIX_LEDs.Color(red, green, blue));
}

void send_valeursfixes_table(){
  for(int i = 0; i < NUMPIXELS; i++){
      write_pixel(i, valeursfixes_table[i][0], valeursfixes_table[i][1], valeursfixes_table[i][2]);
  }
}

void set_valeursfixes_table(int pixmin, int pixmax, byte red, byte green, byte blue){
    for(int i = pixmin; i <= pixmax; i++){
      valeursfixes_table[i][0] = red;
      valeursfixes_table[i][1] = green;
      valeursfixes_table[i][2] = blue;
    }
}

void set_valeursfixes_tableOSC(OSCMessage &msg) {
  int pixmin = msg.getInt(0);
  int pixmax = msg.getInt(1);
  byte value[3];
  value[0] = msg.getInt(2);
  value[1] = msg.getInt(3);
  value[2] = msg.getInt(4);
  set_valeursfixes_table(pixmin, pixmax, value[0], value[1], value[2]);
  //send_valeursfixes_table();
  //PIX_LEDs.show();
  /*Serial.print("/ALUM: ");
  Serial.println(pixmin);
  Serial.println(pixmax);
  Serial.println(value);
  */
  
}
void set_fade_value(OSCMessage &msg) {
  int adress = msg.getInt(0);
  fade_time[adress] = msg.getInt(1); //
  fade_value[adress][0] = msg.getInt(2); // 
  fade_value[adress][1] = msg.getInt(3); // 
  fade_value[adress][2] = msg.getInt(4); //
  fade_delta[adress][0] = fade_value[adress][0] - valeursfixes_table[adress][0];
  fade_delta[adress][1] = fade_value[adress][1] - valeursfixes_table[adress][1];
  fade_delta[adress][2] = fade_value[adress][2] - valeursfixes_table[adress][2];
  for(int i = 0; i < 3; i++){
    /*Serial.print("fade_delta ");
    Serial.print(adress);
    Serial.print(" ");
    Serial.print(i);
    Serial.print(" ");
    Serial.println(fade_delta[adress][i]);//*/
    }
}

void calcul_fade_value(){
  for(int i = 0; i < NUMPIXELS; i++){
    for(int ii = 0; ii < 3; ii++){
      float DELTA_LOOP = fade_delta[i][ii] * TIME_COUNT / fade_time[i];
      /*Serial.print(DELTA_LOOP);
      Serial.print(" ");
      Serial.println(fade_value[i][ii]);//*/
      if((fade_delta[i][ii] > 0) && (valeursfixes_table[i][ii] < fade_value[i][ii])){
        fade_value_calcul[i][ii] = fade_value_calcul[i][ii] + DELTA_LOOP ;
        if(fade_value_calcul[i][ii] > fade_value[i][ii]){
          fade_value_calcul[i][ii] = fade_value[i][ii];
        /*Serial.print("DELTA_LOOP");
        Serial.print(" ");
        Serial.println(DELTA_LOOP);
        Serial.print("fade_value_calcul+ ");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(ii);
        Serial.print(" :");
        Serial.println(fade_value_calcul[i][ii]);//*/
          }
        valeursfixes_table[i][ii] = fade_value_calcul[i][ii];
        /*Serial.print("valeursfixes_table+ ");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(ii);
        Serial.print(" :");
        Serial.println(valeursfixes_table[i][ii]);//*/
      }
      else if((fade_delta[i][ii] < 0) && (valeursfixes_table[i][ii] > fade_value[i][ii])){
        fade_value_calcul[i][ii] = fade_value_calcul[i][ii] + DELTA_LOOP ;
        if(fade_value_calcul[i][ii] < fade_value[i][ii]){
          fade_value_calcul[i][ii] = fade_value[i][ii];
          }
        valeursfixes_table[i][ii] = fade_value_calcul[i][ii];
        /*Serial.print("valeursfixes_table- ");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(ii);
        Serial.print(" :");
        Serial.println(valeursfixes_table[i][ii]);//*/
      }
    } 
  }
}

void set_fade_serie(OSCMessage &msg){ 
  // fade value sur une série de pixel   // OSCmessage /FADE_SERIE nombreDePixels pix1 pix2 ... pixn time(ms) R G B
  int LONGUEUR_SERIE = msg.getInt(0);
  int serie[LONGUEUR_SERIE];
  // Serial.println(LONGUEUR_SERIE);
  int color_serie[3];
  for(int i = 0; i < LONGUEUR_SERIE; i++){
    serie[i] = msg.getInt(i +1);
    //Serial.println(serie[i]);
  }
  float fade_time_serie = msg.getInt(LONGUEUR_SERIE + 1); //Serial.println(fade_time_serie);
  color_serie[0] = msg.getInt(LONGUEUR_SERIE + 2); //Serial.println(color_serie[0]);
  color_serie[1] = msg.getInt(LONGUEUR_SERIE + 3); //Serial.println(color_serie[1]);
  color_serie[2] = msg.getInt(LONGUEUR_SERIE + 4); //Serial.println(color_serie[2]);
  for(int i = 0; i < LONGUEUR_SERIE; i++){
    fade_time[serie[i]] = fade_time_serie;
    for(int ii = 0; ii < 3; ii++){
      fade_value[serie[i]][ii] = color_serie[ii]; //
      fade_delta[serie[i]][ii] = fade_value[serie[i]][ii] - valeursfixes_table[serie[i]][ii];
    }
  } 
}

void set_dimmer(OSCMessage &msg) {         // CONTROL DU DIMMER OSCmessage /DIMMER 0-255
  dimmer = msg.getInt(0);
  PIX_LEDs.setBrightness(dimmer); 
  Serial.print("/DIMMER: ");
  Serial.println(dimmer);
  //PIX_LEDs.show();
}


//////////////////////////////////////////////////////////////////// END OSC receiver et fonctions associés
