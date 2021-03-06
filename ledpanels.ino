#include <Adafruit_NeoPixel.h>

extern const uint8_t gamma8[];

#define LED_PIN     5
#define NUM_LEDS    300
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
#define BRIGHTNESS 100

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Inicializaciones
void leds_init();
void timer2_init();


// Estados
#define REPOSO 0
#define ESTATICO 1
#define CIRCUITO 2
#define DIA 3
#define TRANS_TARDE 4
#define TARDE 5
#define TRANS_NOCHE1 6
#define NOCHE1 7
#define TRANS_NOCHE2 8
#define NOCHE2 9
#define TRANS_DIA 10
#define FLASH 11
#define PARTY_1 12
#define PARTY_2 13
#define PARTY_3 14
#define POLICE 15
#define RAINBOW 16
#define USA 17

// Variables Globales
volatile uint8_t state = REPOSO;
uint8_t timer2_ticks = 5;                  // Frecuencia de actualizacion (1 ~ 1ms)
uint16_t timer2_count = 0;
boolean timer2_flag = false;
boolean gFlag = false;

uint16_t auxR, auxG, auxB;
uint16_t dR, dG, dB;


#define VEL_TRANS 1
#define DEL_TRANS 2
uint16_t diaTimeout = 36;
uint16_t transTardeTimeout = 30;
uint16_t tardeTimeout = 60;
uint16_t transNoche1Timeout = 30;
uint16_t noche1Timeout = 60;
uint16_t transNoche2Timeout = 30;
uint16_t noche2Timeout = 60;
uint16_t transDiaTimeout = 60;

byte diaR = 244, diaG = 164, diaB = 50;
byte tardeR = 255, tardeG = 100, tardeB = 0;
byte noche1R = 50, noche1G = 50, noche1B = 70;
byte noche2R = 5, noche2G = 5, noche2B = 7;

//byte diaR = 251, diaG = 218, diaB = 127;
//byte tardeR = 255, tardeG = 182, tardeB = 0;
//byte noche1R = 127, noche1G = 127, noche1B = 161;
//byte noche2R = 64, noche2G = 64, noche2B = 70;

byte posRGB[3] = {255, 0, 0};
byte actRGB[3] = {255, 0, 0};
byte posBrightness = BRIGHTNESS;
byte actBrightness = BRIGHTNESS;
byte colorCircuito[3] = {diaR, diaG, diaB};

void showStrip();
void setPixelGammaHue(int Pixel, byte red, byte green, byte blue);
void setPixel(int Pixel, byte red, byte green, byte blue);
void setAllGammaHue(byte red, byte green, byte blue);
void setAll(byte red, byte green, byte blue);
void update_bt();
void update_state();
void update_leds();
boolean timeout(uint16_t timeout);
boolean transition(byte initR, byte initG, byte initB, byte finalR, byte finalG, byte finalB, uint16_t tiempo);
void flash(byte red, byte green, byte blue);
void party_1();
void party_2(byte red, byte green, byte blue);
void party_3(byte red, byte green, byte blue);
void usa();
void flash_text(byte red, byte green, byte blue);
void update_rainbow(byte *red, byte *green, byte *blue);
void rainbow(byte vel);
void rainbow_cycle(byte colorVel = 1, byte moveVel = 0);
void text(byte red, byte green, byte blue);
void text_ready(byte red, byte green, byte blue);
void police();
void police2();
void posTime(uint16_t *timeout, uint8_t buff, uint8_t *bt_state, uint8_t tam);

void setup() {
  Serial.begin(9600);
  leds_init();
  timer2_init();
  randomSeed(analogRead(0));
}

void loop() {
  update_bt();
  update_leds();
  //Serial.print("State: ");
  //Serial.print(state);
  //Serial.print(" - BT State: ");
  //Serial.print(bt_state);
  //Serial.println("");
}


#define RED 1
#define GREEN 2
#define BLUE 3
#define BRIGHT 4
#define C_DIA 5
#define C_TARDE 6
#define C_NOCHE1 7
#define C_NOCHE2 8
#define CT_TARDE 9
#define CT_NOCHE1 10
#define CT_NOCHE2 11
#define CT_DIA 12


void update_bt () {
  static char buff = 0;
  static uint8_t bt_state = REPOSO;
  if (Serial.available()) {
    buff = Serial.read();
    //Serial.print(" // BUFF : ");
    //Serial.print(buff);
    if (bt_state == REPOSO) {
      if (buff == 'r') bt_state = RED;
      else if (buff == 'g') bt_state = GREEN;
      else if (buff == 'b') bt_state = BLUE;
      else if (buff == 'f') bt_state = BRIGHT;
      else if (buff == 'q') bt_state = C_DIA;
      else if (buff == 'w') bt_state = C_TARDE;
      else if (buff == 't') bt_state = C_NOCHE1;
      else if (buff == 'y') bt_state = C_NOCHE2;
      else if (buff == 'u') bt_state = CT_TARDE;
      else if (buff == 'i') bt_state = CT_NOCHE1;
      else if (buff == 'o') bt_state = CT_NOCHE2;
      else if (buff == 'p') bt_state = CT_DIA;
      else if (buff == 'c') {
        state = CIRCUITO;
        colorCircuito[0] = diaR;
        colorCircuito[1] = diaG;
        colorCircuito[2] = diaB;
      }
      else if (buff == 'e') state = ESTATICO;
      else if (buff == 'x') state = FLASH;
      else if (buff == 'z') state = PARTY_1;
      else if (buff == 'v') state = PARTY_2;
      else if (buff == 's') state = PARTY_3;
      else if (buff == 'n') state = POLICE;
      else if (buff == 'm') state = RAINBOW;
      else if (buff == 'a') state = USA;
    }
    else if (bt_state == RED) {
      posRGB[0] = buff;
      bt_state = REPOSO;
    }
    else if (bt_state == GREEN) {
      posRGB[1] = buff;
      bt_state = REPOSO;
    }
    else if (bt_state == BLUE) {
      posRGB[2] = buff;
      bt_state = REPOSO;
    }
    else if (bt_state == BRIGHT) {
      posBrightness = buff;
      bt_state = REPOSO;
    }
    else if (bt_state == C_DIA) posTime(&diaTimeout, buff, &bt_state, 2);
    else if (bt_state == C_TARDE) posTime(&tardeTimeout, buff, &bt_state, 2);
    else if (bt_state == C_NOCHE1) posTime(&noche1Timeout, buff, &bt_state, 2);
    else if (bt_state == C_NOCHE2) posTime(&noche2Timeout, buff, &bt_state, 2);
    else if (bt_state == CT_TARDE) posTime(&transTardeTimeout, buff, &bt_state, 2);
    else if (bt_state == CT_NOCHE1) posTime(&transNoche1Timeout, buff, &bt_state, 2);
    else if (bt_state == CT_NOCHE2) posTime(&transNoche2Timeout, buff, &bt_state, 2);
    else if (bt_state == CT_DIA) posTime(&transDiaTimeout, buff, &bt_state, 2);
  }
}

void posTime(uint16_t *timeout, uint8_t buff, uint8_t *bt_state, uint8_t tam) {
  static uint8_t bytes[8] = {};
  static uint8_t aux = 0;
  bytes[aux] = (uint8_t) buff;
  aux++;
  if(aux == tam) {
    aux = 0;
    *bt_state = REPOSO;
    *timeout = 0;
    for (uint8_t i = 0; i < tam; i++) {
      *timeout |= (uint16_t) bytes[i] << 8*i;
    }
  }
}

void update_leds() {
   if(!timer2_flag) return;
   timer2_flag = false;
   switch (state) {
    
    case REPOSO:
      text_ready(actRGB[0], actRGB[1], actRGB[2]);
      break;
    
    case ESTATICO:
      setAllGammaHue(actRGB[0], actRGB[1], actRGB[2]);
      break;
    
    case CIRCUITO:
      setAll(diaR, diaG, diaB);
      state = DIA;
      break;
    
    case DIA:
      if (timeout(diaTimeout)) state = TRANS_TARDE;
      break;
    
    case TRANS_TARDE:
      if (transition(diaR, diaG, diaB, tardeR, tardeG, tardeB, transTardeTimeout)) {
        setAll(tardeR, tardeG, tardeB);
        state = TARDE;
      }
      break;
    
    case TARDE:
      if (timeout(tardeTimeout)) state = TRANS_NOCHE1;
      break;
    
    case TRANS_NOCHE1:
      if (transition(tardeR, tardeG, tardeB, noche1R, noche1G, noche1B, transNoche1Timeout)) {
        setAll(noche1R, noche1G, noche1B);
        state = NOCHE1;
      }
      break;
    
    case NOCHE1:
      if (timeout(noche1Timeout)) state = TRANS_NOCHE2;
      break;
    
    case TRANS_NOCHE2:
      if (transition(noche1R, noche1G, noche1B, noche2R, noche2G, noche2B, transNoche2Timeout)) {
        setAll(noche2R, noche2G, noche2B);
        state = NOCHE2;
      }
      break;
    
    case NOCHE2:
      if(timeout(noche2Timeout)) state = TRANS_DIA;
      break;
    
    case TRANS_DIA:
      if (transition(noche2R, noche2G, noche2B, diaR, diaG, diaB, transDiaTimeout)) {
        setAll(diaR, diaG, diaB);
        state = DIA;
      }
      break;

    case FLASH:
      flash(actRGB[0], actRGB[1], actRGB[2]);
      break;

    case PARTY_1:
      party_1();
      break;

    case PARTY_2:
      //party_2(actRGB[0], actRGB[1], actRGB[2]);
      text(actRGB[0], actRGB[1], actRGB[2]);
      break;

    case PARTY_3:
      party_3(actRGB[0], actRGB[1], actRGB[2]);
      break;

    case POLICE:
      police();
      break;

    case RAINBOW:
      //rainbow_cycle(10, 1);
      rainbow(1);
      break;

    case USA:
      usa();
      break;

    default:
      Serial.println("ERROR");
      state = REPOSO;
      break;
   }
   for(uint8_t i = 0; i < 2; i++) {
    if (actRGB[0] < posRGB[0]) actRGB[0]++;
    else if (actRGB[0] > posRGB[0]) actRGB[0]--;
    if (actRGB[1] < posRGB[1]) actRGB[1]++;
    else if (actRGB[1] > posRGB[1]) actRGB[1]--;
    if (actRGB[2] < posRGB[2]) actRGB[2]++;
    else if (actRGB[2] > posRGB[2]) actRGB[2]--;
   
    if (actBrightness != posBrightness) {
      if(actBrightness > posBrightness) actBrightness--;
      else if(actBrightness < posBrightness) actBrightness++;    
      strip.setBrightness(actBrightness);
    }
   }
   showStrip();
}

boolean timeout(uint16_t tiempo) {
  static uint16_t count = 0;
  count++;
  if (count == (1000/timer2_ticks)*tiempo) {
    count = 0;
    return true;
  }
  return false;
}

boolean transition(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t fR, uint8_t fG, uint8_t fB, uint16_t tiempo) {
  ///////////////////////////////////////////////
  /// Tratando de ahorrar los 3 bytes por led ///
  ///////////////////////////////////////////////
  /*static uint16_t count = 0;
  boolean res = false;
  count++;
  for(uint16_t i = 0; i <NUM_LEDS; i++) {
   setPixel(i, colorCircuito[0], colorCircuito[1], colorCircuito[2]);
   res = update_color(iR, iG, iB, fR, fG, fB, tiempo);
  }*/
  ///////////////////////////////////////////
  /// Solucion utilizando 3 bytes por led ///
  ///////////////////////////////////////////
  /*
  //static uint16_t count = 0;
  static uint16_t count2 = 0;
  static byte r[NUM_LEDS] = {};
  static byte g[NUM_LEDS] = {};
  static byte b[NUM_LEDS] = {};
  boolean res = false;
  //count++;
  setPixel(0, colorCircuito[0], colorCircuito[1], colorCircuito[2]);
  r[0] = colorCircuito[0];
  g[0] = colorCircuito[1];
  b[0] = colorCircuito[2];
  res = update_color(iR, iG, iB, fR, fG, fB, tiempo);
  if(count2 == DEL_TRANS) {
    count2 = 0;
    for(uint16_t j = 0; j < VEL_TRANS; j++) {
      for(uint16_t i = NUM_LEDS - 1; i > 0; i--) {
        setPixel(i, r[i], g[i], b[i]);
        r[i] = r[i - 1];
        g[i] = g[i - 1];
        b[i] = b[i - 1];
      }
    }
  }
  else count2++;
  if (r[0] != r[NUM_LEDS]) res = false;
  */
  ///////////////////////////////////////////
  /// Solucion all transition not Hue     ///
  ///////////////////////////////////////////
  boolean res = false;
  for(uint16_t i = 0; i < NUM_LEDS; i++) {
    setPixel(i, colorCircuito[0], colorCircuito[1], colorCircuito[2]); 
  }
  res = update_color(iR, iG, iB, fR, fG, fB, tiempo);

  ////////////////////////////////////////////
  ////////////////////////////////////////////
  ////////////////////////////////////////////
  return res;
  /*if (count == (1000/timer2_ticks)*tiempo) {
    count = 0;
    return true;
  }
  return false;*/
}

boolean update_color(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t fR, uint8_t fG, uint8_t fB, uint16_t tiempo) {
  /*Serial.print(" dR: ");
  Serial.print(dR);
  Serial.print(" dG: ");
  Serial.print(dG);
  Serial.print(" dB: ");
  Serial.print(dB);
  Serial.print(" auxR: ");
  Serial.print(auxR);
  Serial.print(" auxG: ");
  Serial.print(auxG);
  Serial.print(" auxB: ");
  Serial.print(auxB);
  Serial.print(" R: ");
  Serial.print(colorCircuito[0]);
  Serial.print(" G: ");
  Serial.print(colorCircuito[1]);
  Serial.print(" B: ");
  Serial.print(colorCircuito[2]);
  Serial.println("");*/
  if (fR - iR > 0) dR = (uint16_t)((tiempo * (1000/timer2_ticks)) / (fR - iR));
  else dR = (uint16_t)((tiempo * (1000/timer2_ticks)) / (iR - fR));

  if (fG - iG > 0) dG = (uint16_t)((tiempo * (1000/timer2_ticks)) / (fG - iG));
  else dG = (uint16_t)((tiempo * (1000/timer2_ticks)) / (iG - fG));

  if (fB - iB > 0) dB = (uint16_t)((tiempo * (1000/timer2_ticks)) / (fB - iB));
  else dB = (uint16_t)((tiempo * (1000/timer2_ticks)) / (iB - fB));

  if (auxR == (dR - 1)) {
    auxR = 0;
    if (colorCircuito[0] > fR) colorCircuito[0]--;
    else if (colorCircuito[0] < fR) colorCircuito[0]++;
  }
  else {
    auxR++;
  }

  if (auxG == (dG - 1)) {
    auxG = 0;
    if (colorCircuito[1] > fG) colorCircuito[1]--;
    else if (colorCircuito[1] < fG) colorCircuito[1]++;
  }
  else {
    auxG++;
  }

  if (auxB == (dB - 1)) {
    auxB = 0;
    if (colorCircuito[2] > fB) colorCircuito[2]--;
    else if (colorCircuito[2] < fB) colorCircuito[2]++;
  }
  else {
    auxB++;
  }
  // Prueba
  if (colorCircuito[0] == fR &&
      colorCircuito[1] == fG &&
      colorCircuito[2] == fB) return true;
  return false;
}

void flash (byte red, byte green, byte blue) {
  static uint16_t fOn = 5;
  static uint16_t fOff = 15;
  static uint16_t count = 0;
  count++;
  if (count == 1) setAllGammaHue(red, green, blue);
  else if (count == fOn) setAllGammaHue(0,0,0);
  else if (count == (fOn+fOff)) count = 0;
}

void party_1() {
  static uint16_t count = 0;
  if (gFlag) {
    count = 0;
    gFlag = false;
  }
  count++;
  if (count == 1) setAllGammaHue(255,0,0);
  else if (count == 300) setAllGammaHue(0, 255, 0);
  else if (count == 600) setAllGammaHue(0, 0, 255);
  else if (count == 900) count = 0;
}

///////////
// Texto //
///////////

const bool a[25] = {  0, 0, 1, 0, 0,
                      0, 1, 0, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 1, 1, 1, 1,
                      1, 0, 0, 0, 1 };

const bool d[25] = {  1, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      1, 1, 1, 1, 0 };

const bool e[25] = {  1, 1, 1, 1, 1,
                      1, 0, 0, 0, 0,
                      1, 1, 1, 0, 0,
                      1, 0, 0, 0, 0,
                      1, 1, 1, 1, 1 };

const bool g[25] = {  0, 1, 1, 1, 1,
                      1, 0, 0, 0, 0,
                      1, 0, 0, 1, 1,
                      1, 0, 0, 0, 1,
                      0, 1, 1, 1, 1 };

const bool i[25] = {  1, 1, 1, 1, 1,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0,
                      1, 1, 1, 1, 1 };

const bool j[25] = {  1, 1, 1, 1, 1,
                      0, 0, 0, 1, 0,
                      0, 0, 0, 1, 0,
                      1, 0, 0, 1, 0,
                      0, 1, 1, 0, 0 };

const bool l[25] = {  1, 0, 0, 0, 0,
                      1, 0, 0, 0, 0,
                      1, 0, 0, 0, 0,
                      1, 0, 0, 0, 0,
                      1, 1, 1, 1, 1 };

const bool o[25] = {  0, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      0, 1, 1, 1, 0 };

const bool p[25] = {  1, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 1, 1, 1, 0,
                      1, 0, 0, 0, 0,
                      1, 0, 0, 0, 0 };

const bool q[25] = {  0, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 0, 1, 0, 1,
                      1, 0, 0, 1, 0,
                      0, 1, 1, 0, 1 };

const bool r[25] = {  1, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 1, 1, 1, 0,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1 };

const bool s[25] = {  0, 1, 1, 1, 1,
                      1, 0, 0, 0, 0,
                      0, 1, 1, 1, 0,
                      0, 0, 0, 0, 1,
                      1, 1, 1, 1, 0 };

const bool t[25] = {  1, 1, 1, 1, 1,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0 };

const bool u[25] = {  1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      1, 0, 0, 0, 1,
                      0, 1, 1, 1, 0 };

const bool y[25] = {  1, 0, 0, 0, 1,
                      0, 1, 0, 1, 0,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0,
                      0, 0, 1, 0, 0 };


void text(byte red, byte green, byte blue) {
  setAll(0, 0, 0);
  uint16_t des = 0;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, p[pos]*red, p[pos]*green, p[pos]*blue);
  }
  des = 6;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, u[pos]*red, u[pos]*green, u[pos]*blue);
  }
  des = 12;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, t[pos]*red, t[pos]*green, t[pos]*blue);
  }
  des = 18;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, o[pos]*red, o[pos]*green, o[pos]*blue);
  }
  des = 150;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(q[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 159;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(l[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 165;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(e[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 171;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(e[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  /*
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, j[pos]*red, j[pos]*green, j[pos]*blue);
  }
  des = 6;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, o[pos]*red, o[pos]*green, o[pos]*blue);
  }
  des = 12;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, r[pos]*red, r[pos]*green, r[pos]*blue);
  }
  des = 18;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, g[pos]*red, g[pos]*green, g[pos]*blue);
  }
  des = 24;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, e[pos]*red, e[pos]*green, e[pos]*blue);
  }
  
  des = 156;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, u[pos]*red, u[pos]*green, u[pos]*blue);
  }
  des = 162;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, s[pos]*red, s[pos]*green, s[pos]*blue);
  }
  des = 168;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, a[pos]*red, a[pos]*green, a[pos]*blue);
  }*/
}



/*
               5         10        15        20        25      29
000  o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o
030  o x x x x x o o x o o o x o o x x x x x o o x x x x x o o o
060  o x o o o x o o x o o o x o o o o x o o o o x o o o x o o o
090  o x o o o x o o x o o o x o o o o x o o o o x o o o x o o o
120  o x x x x x o o x o o o x o o o o x o o o o x o o o x o o o
150  o x o o o o o o x o o o x o o o o x o o o o x o o o x o o o
180  o x o o o o o o x o o o x o o o o x o o o o x o o o x o o o
210  o x o o o o o o x o o o x o o o o x o o o o x o o o x o o o
240  o x o o o o o o x x x x x o o o o x o o o o x x x x x o o o
270  o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o

*/

void party_2(byte red, byte green, byte blue) {
  setAllGammaHue(0,0,0);
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if ( (i >= 31 && i <=35) || i == 38 || i == 42 || (i >= 45 && i <=49) || (i >= 52 && i <=56) ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 61 || i == 65 || i == 68 || i == 72 || i == 77 || i == 82 || i == 86  ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 91 || i == 95 || i == 98 || i == 102 || i == 107 || i == 112 || i == 116  ) setPixelGammaHue(i, red, green, blue);
    else if ( (i >= 121 && i <=125) || i == 128 || i == 132 || i == 137 || i == 142 || i == 146  ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 151 || i == 158 || i == 162 || i == 167 || i == 172 || i == 176  ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 181 || i == 188 || i == 192 || i == 197 || i == 202 || i == 206  ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 211 || i == 218 || i == 222 || i == 227 || i == 232 || i == 236  ) setPixelGammaHue(i, red, green, blue);
    else if ( i == 241 || i == 257 ) setPixelGammaHue(i, red, green, blue);
    else if ( (i >= 248 && i <=252) || (i >= 262 && i <=266)) setPixelGammaHue(i, red, green, blue);
  }
}

// UPDATE_RAINBOW
void update_rainbow(byte *red, byte *green, byte *blue) {
  #define RED     0
  #define D_RED   1
  #define GREEN   2
  #define D_GREEN 3
  #define BLUE    4
  #define D_BLUE  5
  static uint8_t r_estado = RED;
  if (r_estado == RED) {
    if ((*red) == 255) r_estado = D_BLUE;
    else (*red)++;
  }
  if (r_estado == D_BLUE) {
    if (!(*blue)) r_estado = GREEN;
    else (*blue)--;
  }
  if (r_estado == GREEN) {
    if ((*green) == 255) r_estado = D_RED;
    else (*green)++;
  }
  if (r_estado == D_RED) {
    if (!(*red)) r_estado = BLUE;
    else (*red)--;
  }
  if (r_estado == BLUE) {
    if ((*blue) == 255) r_estado = D_GREEN;
    else (*blue)++;
  }
  if (r_estado == D_GREEN) {
    if (!(*green)) r_estado = RED;
    else (*green)--;
  }
}

// RAINBOW
void rainbow(byte vel) {
  static byte red = 0, green = 0, blue = 0;
  for (uint16_t i = 0; i < vel; i++) {
     update_rainbow(&red, &green, &blue);
  }
  setAllGammaHue(red, green, blue);
}

// Rainbow Cycle
byte r_red[NUM_LEDS/10] = {};
byte r_green[NUM_LEDS/10] = {};
byte r_blue[NUM_LEDS/10] = {};
void rainbow_cycle(byte colorVel, byte moveVel) {
  static byte red = 255, green = 0, blue = 0;
  static uint16_t cycleCount = 0;
  if (cycleCount == moveVel) {
    cycleCount = 0;
    for (uint16_t i = 0; i < colorVel; i++) {
      update_rainbow(&red, &green, &blue);
    }
    r_red[0] = red;
    r_green[0] = green;
    r_blue[0] = blue;
    for (uint16_t j = 0; j < NUM_LEDS/10; j++) {
      r_red[j+1] = r_red[j];
      r_green[j+1] = r_green[j];
      r_blue[j+1] = r_blue[j];
      for(uint16_t x = 0; x < 10; x++) {
        setPixel(j+x*30, r_red[j], r_green[j], r_blue[j]); 
      }
    }
  }
  else {
    cycleCount++;
  }
}

const boolean barra[130] = {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  };
void barra_f (uint16_t des, byte red, byte green, byte blue) {
  setAll(0, 0, 0);
  for(uint16_t pos = 0; pos < 180; pos++) {
    if (!(pos%17) && pos) des+= 13;
    setPixelGammaHue(pos+des, red, green, blue);
  }
}

void barra_azul (uint16_t des) {
  for(uint16_t pos = 0; pos < 130; pos++) {
    if (!(pos%13) && pos) des+= 17;
    setPixelGammaHue(pos+des, barra[pos]*0, barra[pos]*0, barra[pos]*255);
  }
}

void barra_roja (uint16_t des) {
  for(uint16_t pos = 0; pos < 130; pos++) {
    if (!(pos%13) && pos) des+= 17;
    setPixelGammaHue(pos+des, barra[pos]*255, barra[pos]*0, barra[pos]*0);
  }
}

void barra_blanca (uint16_t des) {
  for(uint16_t pos = 0; pos < 180; pos++) {
    if (!(pos%10) && pos) des+= 20;
    setPixelGammaHue(pos+des, barra[pos]*255, barra[pos]*255, barra[pos]*255);
  }
}

boolean p_delay(uint16_t del) {
  static uint16_t count = 0;
  if(count == del) {
    count = 0;
    return true;
  }
  count++;
  return false;
}

void police() {
  #define P_BLUE 0
  #define P_RED 1
  #define P_STROBE 2
  static uint8_t pol_state = P_BLUE;
  static uint16_t count = 0;
  static uint16_t flash_count = 0;
  static uint8_t rep_count = 0;
  uint16_t des = 0;
  count++;
  if (pol_state == P_BLUE) {
    if (flash_count < 6) {
      if(count == 1) barra_azul(0);
      else if(count == 2) setAll(0, 0, 0);
      else if(count == 5) {
       count = 0;
       flash_count++;
      }
    }
    else if (flash_count == 6) {
      pol_state = P_RED;
      flash_count = 0; 
      count = 0;
    }
  }
  else if (pol_state == P_RED) {
    if (flash_count < 6) {
      if(count == 1) barra_roja(17);
      else if(count == 2) setAll(0, 0, 0);
      else if(count == 4) {
        count = 0;
        flash_count++;
      }
    }
    else if (flash_count == 6) {
      pol_state = P_BLUE;
      rep_count++;
      flash_count = 0;
      count = 0;
      if(rep_count == 4) {
        rep_count = 0;
        pol_state = P_STROBE;
        flash_count = 0;
        count = 0;
      }
    }
  }
  else if (pol_state == P_STROBE) {
    static boolean dir = 0; 
    if(count == 1) {
      /*//barra_f(dir*13, 255, 255, 255);
      barra_f(dir*13, dir*255, 0, (!dir)*255);
      barra_blanca(10);
      //fix//
      setPixel(288, 255, 255, 255);
      setPixel(289, 255, 255, 255);
      */
      setAll(255, 255, 255);
    }
    else if(count == 2) setAll(0, 0, 0);
    else if(count == 5) {
      count = 0;
      flash_count++;
      dir = !dir;
    }
    if (flash_count == 12) {
      pol_state = P_BLUE;
      flash_count = 0;
    }
  }
}

const boolean barra2[50] = { 1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1 };

void barra_2 (uint16_t des, byte red, byte green, byte blue) {
  for(uint16_t pos = 0; pos < 50; pos++) {
    if (!(pos%5) && pos) des += 25;
    setPixelGammaHue(pos+des, barra2[pos]*red, barra2[pos]*green, barra2[pos]*blue);
  }
}

void police2() {
  #define P_BLUE 0
  #define P_RED 1
  #define P_STROBE 2
  static uint8_t pol_state = P_BLUE;
  static uint16_t count = 0;
  count++;
  if(pol_state == P_BLUE) {
    if(count == 1) {
      barra_2(0, 0, 0, 255);
      barra_2(10, 0, 0, 255);
      barra_2(20, 255, 0, 0); 
    }
    else if (count == 4) setAll(0, 0, 0);
    else if(count == 10) {
      pol_state = P_RED;
      count = 0;
    }
  }
  else if (pol_state == P_RED) {
    if(count == 1) {
      barra_2(5, 0, 0, 255);
      barra_2(15, 255, 0, 0);
      barra_2(25, 255, 0, 0); 
    }
    else if(count == 4) setAll(0, 0, 0);
    else if(count == 10) {
      pol_state = P_BLUE;
      count = 0;
    }
  }
}

/*const boolean barra[130] = {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  };
*/
void police3() {
  #define P_BLUE 0
  #define P_RED 1
  #define P_STROBE 2
  static uint8_t pol_state = P_BLUE;
  static uint16_t count = 0;
  static uint16_t flash_count = 0;
  static uint8_t rep_count = 0;
  uint16_t des = 0;
  count++;
  if (pol_state == P_BLUE) {
    if (flash_count < 6) {
      if(count == 1) barra_azul(0);
      else if(count == 2) setAll(0, 0, 0);
      else if(count == 5) {
       count = 0;
       flash_count++;
      }
    }
    else if (flash_count == 6) {
      pol_state = P_RED;
      flash_count = 0; 
      count = 0;
    }
  }
  else if (pol_state == P_RED) {
    if (flash_count < 6) {
      if(count == 1) barra_roja(17);
      else if(count == 2) setAll(0, 0, 0);
      else if(count == 5) {
        count = 0;
        flash_count++;
      }
    }
    else if (flash_count == 6) {
      pol_state = P_BLUE;
      rep_count++;
      flash_count = 0;
      count = 0;
      if(rep_count == 4) {
        rep_count = 0;
        pol_state = P_STROBE;
        flash_count = 0;
        count = 0;
      }
    }
  }
  else if (pol_state == P_STROBE) {
    if(count == 1) setAll(255, 255, 255);
    else if(count == 2) setAll(0, 0, 0);
    else if(count == 4) {
      count = 0;
      flash_count++;
    }
    if (flash_count == 18) {
      pol_state = P_BLUE;
      flash_count = 0;
    }    
  }
}

/*
               5         10        15        20        25      29
000  o o o o o o o o o o o o o o o o o o r r r r r r r r r r r r
030  o b o o o b o o o b o o o b o o o o o o o o o o o o o o o o
060  o o o o o o o o o o o o o o o o o o r r r r r r r r r r r r
090  o o o b o o o b o o o b o o o b o o o o o o o o o o o o o o
120  o o o o o o o o o o o o o o o o o o r r r r r r r r r r r r
150  o b o o o b o o o b o o o b o o o o o o o o o o o o o o o o
180  a a a a a a a a a a a a a a a a a a r r r r r r r r r r r r
210  o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o
240  r r r r r r r r r r r r r r r r r r r r r r r r r r r r r r
270  r r r r r r r r r r r r r r r r r r r r r r r r r r r r r r
*/

void usa() {
  setAll(0, 0, 255);
  boolean color = true;
  for (uint16_t i = 31; i < 100; i+=2) {
    if(i == 41 || i == 72) i+=19;
    setPixel(i, 255, 255, 255);
  }
  setPixel(60, 0, 0, 255);
  setPixel(90, 0, 0, 255);
  //setPixel(120, 0, 0, 255);
  for (uint16_t i = 12; i < 150; i++) {
    if(i==30 || i==60 || i==90 || i==120) {
      i+=12;
      color = !color;
    }
    if(color) setPixel(i, 255, 0, 0);
    else setPixel(i, 255, 255, 255);
    
  }
  for (uint16_t i = 150; i < 180; i++) {
    setPixel(i, 255, 255, 255);
  }
  for (uint16_t i = 180; i < 210; i++) {
    setPixel(i, 255, 0, 0);
  }
  for (uint16_t i = 210; i < 240; i++) {
    setPixel(i, 255, 255, 255);
  }
  for (uint16_t i = 240; i < NUM_LEDS; i++) {
    setPixel(i, 255, 0, 0);
  }
}

void party_3 (byte red, byte green, byte blue) {
  static bool f_random = false;
  static uint16_t rand_count = 0;
  static uint16_t rand_flash_count = 0;
  if( !(((uint32_t)random(0,3000))%500) ) f_random = true;
  if(f_random) {
    rand_count++;
    if(rand_count == 1) setAll(255, 255, 255);
    else if(rand_count == 2) setAll(0, 0, 0);
    else if(rand_count == 4) {
      rand_count = 0;
      rand_flash_count++;
    }
    if (rand_flash_count == 18) {
      f_random = false;
      rand_count = 0;
      rand_flash_count = 0;
    }
    return;
  }
  // Flash
  static uint16_t fOn = 4;
  static uint16_t fOff = 12;
  static uint16_t count = 0;
  count++;
  if (count == 1) {
    for(uint16_t i = 0; i < 90; i++) {
      setPixelGammaHue(i, red, green, blue);
      setPixelGammaHue(i+21, red, green, blue);
      setPixelGammaHue(i+210, red, green, blue);
      setPixelGammaHue(i+231, red, green, blue);
      if(i==8 || i == 38 || i == 68) i+=21;
    }
  }
  else if (count == fOn) {
    for(uint16_t i = 0; i < 90; i++) {
      setPixel(i, 0, 0, 0);
      setPixel(i+21, 0, 0, 0);
      setPixel(i+210, 0, 0, 0);
      setPixel(i+231, 0, 0, 0);
      if(i==8 || i == 38 || i == 68) i+=21;
    }
  }
  else if (count == (fOn+fOff)) {
    fOn = (uint16_t) random(1, 5);
    fOff = (uint16_t) random(1, 13);
    count = 0;
  }

  // Flash-Fade
  static byte ff_r = 0;
  static byte ff_g = 0;
  static byte ff_b = 255;
  for(uint16_t j = 9; j < 21; j++) {
    setPixel(j, ff_r, ff_g, ff_b);
    setPixel(30+j, ff_r, ff_g, ff_b);
    setPixel(60+j, ff_r, ff_g, ff_b);
    setPixel(210+j, ff_r, ff_g, ff_b);
    setPixel(240+j, ff_r, ff_g, ff_b);
    setPixel(270+j, ff_r, ff_g, ff_b);
  }
  if(ff_r) ff_r--;
  else ff_r = (byte) random(0,256);
  if(ff_g) ff_g--;
  else ff_g = (byte) random(0,256);
  if(ff_b) ff_b--;
  else ff_b = (byte) random(0,256);

  // Lineas colores
  static byte col_r[7] = {255,  255,  255,  255,  0,    0,    0   };
  static byte col_g[7] = {0,    255,  0,    255,  255,  255,  0   };
  static byte col_b[7] = {0,    0,    255,  255,  0,    255,  255 };
  static byte aux = 0;
  static bool col_dir = true;
  static uint16_t col_pos = 150;
  if(col_dir) {
    col_pos++;
    if(col_pos == 179) {
      aux++;
      aux%=7;
      col_dir = !col_dir;
    }
  }
  else {
    col_pos--;
    if(col_pos == 150) {
      aux++;
      aux%=7;
      col_dir = !col_dir;
    }
  }
  setPixel(col_pos, col_r[aux], col_b[aux], col_g[aux]);
  setPixel(30+col_pos, col_r[aux], col_b[aux], col_g[aux]); // Linea extra

  // Lineas tipo KITT
  #define RIGHT 0
  #define LEFT 1
  #define MIDDLE 2
  #define LEN 6
  #define REP 1
  static uint8_t p3_state = RIGHT;
  static int16_t l_count = 0;
  static uint16_t i = 90;
  static uint8_t kitt_del = 0;
  if(kitt_del < 1) {
    kitt_del++;
    return;
  }
  else kitt_del = 0;
  if(p3_state == RIGHT) {
    if(i < (120)) {
      for(uint8_t j = 0; j < LEN; j++) {
        if((i-j)>=90) {
          setPixel(i-j, 255, 0, 0);
          setPixel(30+i-j, 255, 0, 0); // Linea extra          
        }
        if((i-j-1)>=90) {
          setPixel(i-j-1, 0, 0, 0);
          setPixel(30+i-j-1, 0, 0, 0); // Linea extra
        }
      }
      i++;
    }
    else {
      p3_state = LEFT;
      i-=LEN;
    }
  }
  else if (p3_state == LEFT) {
    if(i >= 90) {
      for(uint8_t j = 0; j < LEN; j++) {
        if((i+j)<120) {
          setPixel(i+j, 255, 0, 0);
          setPixel(30+i+j, 255, 0, 0); // Linea extra
        }
        if((i+j+1)<120) {
          setPixel(i+j+1, 0, 0, 0);
          setPixel(30+i+j+1, 0, 0, 0); // Linea extra
        }
      }
      i--;
    }
    else {
      p3_state = RIGHT;
      i+=LEN;
      l_count++;
      if(l_count == REP) {
        l_count = 0;
        p3_state = MIDDLE;
        i-=LEN;
      }
    }
  }
  else if (p3_state == MIDDLE) {
    static bool mid_dir = true;
    static bool turn = true;
    if(mid_dir) {
      if(i<105) {
        for(uint8_t j = 0; j < LEN/2; j++) {
          if((i-j)>=90) {
            setPixel(i-j, 255, 0, 0);
            setPixel(209-(i-j), 255, 0, 0);
            // Linea extra
            setPixel(30+i-j, 255, 0, 0);
            setPixel(30+209-(i-j), 255, 0, 0);
          }
          if((i-j-1)>=90) {
            setPixel(i-j-1, 0, 0, 0);
            setPixel(209-(i-j-1), 0, 0, 0);
            // Linea extra
            setPixel(30+i-j-1, 0, 0, 0);
            setPixel(30+209-(i-j-1), 0, 0, 0);
          }
         }
        i++;
      }
      else mid_dir = false;  
    }
    else {
      if(i>=90) {
        for(uint8_t j = 0; j < LEN/2; j++) {
          if((i+j)<120) {
            setPixel(i+j, 255, 0, 0);
            setPixel(209-(i+j), 255, 0, 0);
            // Linea extra
            setPixel(30+i+j, 255, 0, 0);
            setPixel(30+209-(i+j), 255, 0, 0);
          }
          if((i+j+1)<120) {
            setPixel(i+j+1, 0, 0, 0);
            setPixel(209-(i+j+1), 0, 0, 0);
            // Linea extra
            setPixel(30+i+j+1, 0, 0, 0);
            setPixel(30+209-(i+j+1), 0, 0, 0);            
          }
        }
        i--;
      }
      else {
        mid_dir = true;
        turn = !turn;
        if(turn) {
          p3_state = LEFT;
          for(uint16_t j = 0; j<LEN/2; j++) { // Fix
            setPixel(90+j, 0, 0, 0);
            setPixel(30+90+j, 0, 0, 0); // Linea extra
          }
          i = 119;
          l_count = -1;
        }
        else {
          p3_state = RIGHT;
          for(uint16_t j = 0; j<LEN/2; j++) { // Fix
            setPixel(119-j, 0, 0, 0);
            setPixel(30+119-j, 0, 0, 0); // Linea extra
          }
          i = 90;
        }
      }
    }
  }
  // No escribir nada abajo de KITT (por el return)
}

void flash_text(byte red, byte green, byte blue) {
  // Flash
  static uint16_t fOn = 4;
  static uint16_t fOff = 12;
  static uint16_t count = 0;
  count++;
  if (count == 1) setAllGammaHue(red, green, blue);
  else if (count == fOn) setAllGammaHue(0,0,0);
  else if (count == (fOn+fOff)) count = 0;

  // Texto
  uint16_t des = 63;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if (p[pos]) setPixel(pos+des, 0, 0, 0);
  }
  des = 69;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if (u[pos]) setPixel(pos+des, 0, 0, 0);
  }
  des = 75;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if (t[pos]) setPixel(pos+des, 0, 0, 0);
  }
  des = 81;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if (o[pos]) setPixel(pos+des, 0, 0, 0);
  }
}

void text_ready(byte red, byte green, byte blue) {
  setAll(0, 0, 0);
  uint16_t des = 0;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, r[pos]*red, r[pos]*green, r[pos]*blue);
  }
  des = 6;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, e[pos]*red, e[pos]*green, e[pos]*blue);
  }
  des = 12;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, a[pos]*red, a[pos]*green, a[pos]*blue);
  }
  des = 18;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, d[pos]*red, d[pos]*green, d[pos]*blue);
  }
  des = 24;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    setPixelGammaHue(pos+des, y[pos]*red, y[pos]*green, y[pos]*blue);
  }
  des = 150;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(l[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 156;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(i[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 162;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(s[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 168;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(t[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
  des = 174;
  for(uint16_t pos = 0; pos < 25; pos++) {
    if (!(pos%5) && pos) des+= 25;
    if(o[pos]) setPixelGammaHue(pos+des, 255-red, 255-green, 255-blue);
  }
}


void leds_init() {
  strip.begin();
  strip.clear();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
}

void timer2_init() {
  SREG = (SREG & 0b01111111);              // Deshabilitar interrupciones globales
  TCNT2 = 0;                               // Limpiar el registro que guarda la cuenta del Timer-2.
  TIMSK2 = TIMSK2 | 0b00000001;            // Habilitaci??n de la bandera 0 del registro que habilita la interrupci??n por sobre flujo o desbordamiento del TIMER2.
  TCCR2B = 0b00000011;                     // ft2 = 250Khz => 1ms ( (1/250.000) * 255 )
  SREG = (SREG & 0b01111111) | 0b10000000; // Habilitar interrupciones
}

ISR(TIMER2_OVF_vect) {
  timer2_count++;
  if (timer2_count == timer2_ticks) {
    timer2_flag = true;
    timer2_count = 0;
  }
}


/////////////
// DRIVERS //
/////////////

void showStrip() {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  FastLED.show();
#endif
}

void setPixelGammaHue(int Pixel, byte red, byte green, byte blue) {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(pgm_read_byte(&gamma8[red]), pgm_read_byte(&gamma8[green]), pgm_read_byte(&gamma8[blue])));
  //strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = pgm_read_byte(&gamma8[red]);
  leds[Pixel].g = pgm_read_byte(&gamma8[green]);
  leds[Pixel].b = pgm_read_byte(&gamma8[blue]);
  //leds[Pixel].r = red;
  //leds[Pixel].g = green;
  //leds[Pixel].b = blue;
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
  //strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
#endif
}

void setAllGammaHue(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixelGammaHue(i, red, green, blue);
  }
}


void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
}

///////////////
// Gamma Hue //
///////////////

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
