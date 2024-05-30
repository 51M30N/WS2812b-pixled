/*
ADAPTED BY 51M30N
contact@lacon.dev
*/


#include <Wire.h>
///////////////////////////////////////////////////////////////////////////////////////////////////PAR LÀ LE NOM DU RECEPTEUR
const char* DEVICE_ID = "StuART_WS2812B/RING12/8";                                  ////note ici le nom du code et de la version ESP8266_$capteur-$version
const char* PREFIX = "/CTRL/";
String completeAddress = String(PREFIX) + String(DEVICE_ID);
const char* DEVICE_NAME = completeAddress.c_str();
//OSCErrorCode error;
#include "secret_ssidlocal.h" //
#include "StuART_OSClocal.h" //


// DEFS DE LA LEDSTRIP
#include <Adafruit_NeoPixel.h>
#define NUMPIXELSTAB 64 // Nombre de LEDs dans la bande//////////////////////////////////////////////// ICI LE NOMBRE DE PIXELS PREVUS DANS LE PROG
// Connexions pour l'ESP8266 LEDSTRIP
#define DATAPIN D5   // ou un autre pin approprié
Adafruit_NeoPixel PIX_LEDs(NUMPIXELSTAB, DATAPIN, NEO_GRB + NEO_KHZ800);


int NUMPIXELS = 64; // Nombre de LEDs dans la bande//////////////////////////////////////////////// ICI LE NOMBRE DE PIXELS
// VARIABLES DE LEDSTRIP
//int pix; // 
//boolean buttonState = 0;
//int colormax = 255; //max 

  byte toogle_program = 0;
  int program = 1; // numéro de programm (0 = no program)                                        ///// numero du programme au demarage
  int programSELECTEDPIX = 0; // pixel selectionné pour program
  int programTIME_COUNT;  // compteur temps de program
  int programTIME_DELAY = 100; //temps de boucle de program
  unsigned int programcurrentMillis = 0; //variable time de  program
  unsigned int programpreviousMillis = 0; //variable time de  program

int freq = 25; // en ms, vitesse de rafraîchissement
//byte decrement_table[NUMPIXELS]; // chaîne de valeur de color[0] = 0 à <=colormax
int valeursfixes_table[NUMPIXELSTAB][3]; //
float fade_time[NUMPIXELSTAB]; //
int fade_value[NUMPIXELSTAB][3]; //
float fade_value_calcul[NUMPIXELSTAB][3]; //
float fade_delta[NUMPIXELSTAB][3]; //
float detla[3];
//int lenght_decrement_table; // 
//int lenght_snake = 10; // longueur de la chaîne
byte dimmer = 200;

#include "OneButton.h"
#define PIN_INPUT D1
#define PIN_GND D2
OneButton button(PIN_INPUT, true);
//unsigned long pressStartTime;// save the millis when a press has started.
ICACHE_RAM_ATTR void checkTicks() {
  // include all buttons here to be checked
  button.tick(); // just call tick() to check the state.
}
// INIT interrupt routine ONEBUTTON
// this function will be called when the button was pressed 1 time only.
void singleClick() {
  program += 1;
  program_change(program);
  //Serial.println("singleClick() detected.");
} // singleClick
// this function will be called when the button was pressed 2 times in a short timeframe.
void doubleClick() {
  program = 0;
  program_change(program);
  //Serial.println("doubleClick() detected.");
} // doubleClick
// this function will be called when the button was held down for 1 second or more.
void pressStart() {
  program = 1;
  program_change(program);
  //Serial.println("pressStart()");
  //pressStartTime = millis() - 1000; // as set in setPressMs()
} // pressStart()



//#include "program-eliwa.h" //
#include "program.h" //
//#include "program-duck.h" //
//#include "program-1024.h" //
//#include "program-8x8.h" //


void setup() { ///////////////////////////////SETUP///////////////////////
  
// SETUP BUILTINLED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led
  
/// SERIAL INITIALISATION
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
   
  wificonnection();
  HelloWorld();
  openOSCport();
  
  
////SETUP DE LEDSTRIP
  //pinMode(TOOGLEPIN, INPUT_PULLUP); /// PEUT ETRE PLUS UTILE
  //calcul_decrement_table(colormax, lenght_snake);
  PIX_LEDs.setBrightness(dimmer); 
  PIX_LEDs.begin();
  PIX_LEDs.show();
  set_valeursfixes_table(0, (NUMPIXELS - 1), 0, 0, 0);  // INITIALISATION DES LEDS A ZERO


  // setup interrupt routine ONEBUTTON
  // enable the led output.
  pinMode(PIN_GND, OUTPUT); // sets the digital pin as output
  digitalWrite(PIN_GND, LOW);
  // when not registering to the interrupt the sketch also works when the tick is called frequently.
  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), checkTicks, CHANGE);
  // link the xxxclick functions to be called on xxxclick event.
  button.attachClick(singleClick);
  button.attachDoubleClick(doubleClick);
  button.setPressMs(1000); // that is the time when LongPressStart is called
  button.attachLongPressStart(pressStart);
  //button.attachLongPressStop(pressStop);

}
///////////////////////////////END SETUP///////////////////////






///////////////////////////////LOOP///////////////////////
void loop() {  
oscreceiver(); // écoute du port OSC d'entree
//count_time(); // calcul du temps écoulé depuis la dernière mesure
//program_count_time(); // calcul du temps écoulé depuis le dernir tour dans le program
// WAIT A MOMENT  
    //delay(TIME_LOOP);
    calcul_fade_value();
    send_valeursfixes_table();  
    PIX_LEDs.show();
    button.tick();  // controle du bouton 
    delay(freq); 

    if (program == 0){      // programm == 0 => pas de program
    }
    else{
      send_program();
    }
} 
///////////////////////////////END LOOP///////////////////////



void oscreceiver(){                                 //RECEPTION OSC
  OSCMessage bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {                                 // ROUTE OSC
      bundle.dispatch("/led", led);                           // /led 0-1 : turn on-off ledbuildin
      bundle.dispatch("/IPCIBLE", ipcible);                   // /IPCIBLE $1 $2 $3 $4
      bundle.dispatch("/POOL_REQUEST", PoolREQUEST);          // reply osc information
      bundle.dispatch("/helloworld", reply_helloworld);       // reply osc information
      bundle.dispatch("/DIMMER", set_dimmer);                 // /DIMMER 0-255 : set brightness
      bundle.dispatch("/ALUM",set_valeursfixes_tableOSC);     // direct value /ALUM numPixMin numPixMax time R G B 
      bundle.dispatch("/FADE_SERIE",set_fade_serie);          //deprecated // fade une série de pixel 
      bundle.dispatch("/PROGRAM",select_program);             // /PROGAM $1 : programmes internes 
      bundle.dispatch("/PIX",set_pix_serie);                  //deprecated// reçoit une série de pixels 
      bundle.dispatch("/NUMPIXEL",set_numpixel);              // defini le bombre de pixels (use by PROGRAM)
      bundle.dispatch("/SETPROGRAM/color",set_program_color); // defini les paramètres du programme pers "/SETPROGRAM/color RGBup RGBdown"
      bundle.dispatch("/SETPROGRAM/speed",set_program_speed); // defini les paramètres du programme pers "/SETPROGRAM/speed up_time down_time programTIME_COUNT deltaSELECTEDPIX"
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

void set_pix_serie(OSCMessage &msg) {       // reçoit une série de pixels 
   byte nbPix = msg.getInt(0);
   int timePix = msg.getInt(1);
    for(byte i = 0; i < nbPix; i++){
      int ioffset = i*4;
      set_color_1pix(msg.getInt(2 + ioffset), timePix, msg.getInt(3 + ioffset), msg.getInt(4 + ioffset), msg.getInt(5 + ioffset));
    }
}

void set_valeursfixes_tableOSC(OSCMessage &msg) {
  /*int pixmin = msg.getInt(0);
  int pixmax = msg.getInt(1);
  byte value[3];
  value[0] = msg.getInt(3);
  value[1] = msg.getInt(4);
  value[2] = msg.getInt(5);
  float fade_time_serie = msg.getInt(2); //Serial.println(fade_time_serie);
  for(int i = pixmin; i <= pixmax; i++){
    fade_time[1] = fade_time_serie;
    for(int ii = 0; ii < 3; ii++){
      fade_value[i][ii] = color_serie[ii]; //
      fade_delta[i][ii] = fade_value[i][ii] - valeursfixes_table[i][ii];
    }*/
  //int LONGUEUR_SERIE = msg.getInt(0);
  //int serie[LONGUEUR_SERIE];
  // Serial.println(LONGUEUR_SERIE);
  byte color_serie[3];
  int pixmin = msg.getInt(0);
  int pixmax = msg.getInt(1);
  /*for(int i = 0; i < LONGUEUR_SERIE; i++){
    serie[i] = msg.getInt(i +1);
    //Serial.println(serie[i]);
  }*/
  float fade_time_serie = msg.getInt(2); //Serial.println(fade_time_serie);
  color_serie[0] = msg.getInt(3); //Serial.println(color_serie[0]);
  color_serie[1] = msg.getInt(4); //Serial.println(color_serie[1]);
  color_serie[2] = msg.getInt(5); //Serial.println(color_serie[2]);
  for(int i = pixmin; i <= pixmax; i++){
    fade_time[i] = fade_time_serie;
    for(byte ii = 0; ii < 3; ii++){
      fade_value[i][ii] = color_serie[ii]; //
      fade_delta[i][ii] = fade_value[i][ii] - valeursfixes_table[i][ii];
    }
  } 
  //set_valeursfixes_table(pixmin, pixmax, value[0], value[1], value[2]);
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
    for(byte ii = 0; ii < 3; ii++){
      float DELTA_LOOP = fade_delta[i][ii] * freq / fade_time[i];
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
  byte color_serie[3];
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
    for(byte ii = 0; ii < 3; ii++){
      fade_value[serie[i]][ii] = color_serie[ii]; //
      fade_delta[serie[i]][ii] = fade_value[serie[i]][ii] - valeursfixes_table[serie[i]][ii];
    }
  } 
}

void set_dimmer(OSCMessage &msg) {         // CONTROL DU DIMMER OSCmessage /DIMMER 0-255
  dimmer = msg.getInt(0);
  PIX_LEDs.setBrightness(dimmer); 
  //Serial.print("/DIMMER: ");
  //Serial.println(dimmer);
  //PIX_LEDs.show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_numpixel(OSCMessage &msg) {         // CONTROL D
  if (msg.isInt(0)){
    NUMPIXELS = msg.getInt(0);
  }
  //Serial.print("/NUMPIXELS: ");
  //Serial.println(NUMPIXELS);
  //PIX_LEDs.show();
}///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_program_color(OSCMessage &msg) {         // set la color des programm -1 -2 -3 ....
  for (int i = 0; i < 3; i++){
    if (msg.isInt(i)){
      color_program_up[0][i] = msg.getInt(i);
    }
    if (msg.isInt(i+3)){
      color_program_down[0][i] = msg.getInt(i+3);
    } else {color_program_down[0][i] = 0;}
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_program_speed(OSCMessage &msg) {         // set les time des programm -1 -2 -3 ....
  if (msg.isInt(0)){
    up_time = msg.getInt(0);
  }
  if (msg.isInt(1)){
    down_time = msg.getInt(1);
  }
  if (msg.isInt(2)){
    programTIME_DELAY = msg.getInt(2);
  }
  if (msg.isInt(3)){
    deltaSELECTEDPIX = msg.getInt(3);
    if (deltaSELECTEDPIX == 0){
      deltaSELECTEDPIX = 1;
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void select_program(OSCMessage &msg) {         // changement DU NUMERO DE PROGRAMME
  char receivedString[2];
  if (msg.isInt(0)){
    program_change(msg.getInt(0));
  }
  else if (msg.getString(0, receivedString, sizeof(receivedString))){
    //Serial.println("msgIsString");
    if (strcmp(receivedString, "+") == 0) {
      program_change(program += 1);
    }
    else if (strcmp(receivedString, "-") == 0) {
      program_change(program -= 1);      
    }
  }
}

void set_color_1pix(int pix, long int fadetime, byte red, byte green, byte blue){
  byte color_serie[3] = {red, green, blue};
  // fade value sur une série de pixel   // OSCmessage /FADE_SERIE nombreDePixels pix1 pix2 ... pixn time(ms) R G B
  //byte LONGUEUR_SERIE = 1;
  fade_time[pix] = fadetime;
  for(byte ii = 0; ii < 3; ii++){
    fade_value[pix][ii] = color_serie[ii]; //
    fade_delta[pix][ii] = fade_value[pix][ii] - valeursfixes_table[pix][ii];
  }
 }

 void program_change(int num){
  program = num;
  //if (program <= 0) {program = 0;}
  for (byte i = 0; i<NUMPIXELS; i++){
    set_color_1pix(i, 1000, 0, 0, 0);
  }
  Serial.print("Start program ");  Serial.println(program);
  //delay(2000);
 }

//////////////////////////////////////////////////////////////// END OSC receiver et fonctions associés
