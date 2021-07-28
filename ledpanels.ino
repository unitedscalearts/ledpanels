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

// Variables Globales
volatile uint8_t state = ESTATICO;
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
void setPixel(int Pixel, byte red, byte green, byte blue);
void setPixel_notHue(int Pixel, byte red, byte green, byte blue);
void setAll(byte red, byte green, byte blue);
void setAll_notHue(byte red, byte green, byte blue);
void update_bt();
void update_state();
void update_leds();
boolean timeout(uint16_t timeout);
boolean transition(byte initR, byte initG, byte initB, byte finalR, byte finalG, byte finalB, uint16_t tiempo);
void flash(byte red, byte green, byte blue);
void party_1();
void party_2(byte red, byte green, byte blue);
void posTime(uint16_t *timeout, uint8_t buff, uint8_t *bt_state, uint8_t tam);

void setup() {
  Serial.begin(9600);
  leds_init();
  timer2_init();
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
      break;
    
    case ESTATICO:
      setAll(actRGB[0], actRGB[1], actRGB[2]);
      break;
    
    case CIRCUITO:
      setAll_notHue(diaR, diaG, diaB);
      state = DIA;
      break;
    
    case DIA:
      if (timeout(diaTimeout)) state = TRANS_TARDE;
      break;
    
    case TRANS_TARDE:
      if (transition(diaR, diaG, diaB, tardeR, tardeG, tardeB, transTardeTimeout)) {
        setAll_notHue(tardeR, tardeG, tardeB);
        state = TARDE;
      }
      break;
    
    case TARDE:
      if (timeout(tardeTimeout)) state = TRANS_NOCHE1;
      break;
    
    case TRANS_NOCHE1:
      if (transition(tardeR, tardeG, tardeB, noche1R, noche1G, noche1B, transNoche1Timeout)) {
        setAll_notHue(noche1R, noche1G, noche1B);
        state = NOCHE1;
      }
      break;
    
    case NOCHE1:
      if (timeout(noche1Timeout)) state = TRANS_NOCHE2;
      break;
    
    case TRANS_NOCHE2:
      if (transition(noche1R, noche1G, noche1B, noche2R, noche2G, noche2B, transNoche2Timeout)) {
        setAll_notHue(noche2R, noche2G, noche2B);
        state = NOCHE2;
      }
      break;
    
    case NOCHE2:
      if(timeout(noche2Timeout)) state = TRANS_DIA;
      break;
    
    case TRANS_DIA:
      if (transition(noche2R, noche2G, noche2B, diaR, diaG, diaB, transDiaTimeout)) {
        setAll_notHue(diaR, diaG, diaB);
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
      party_2(actRGB[0], actRGB[1], actRGB[2]);
      break;

    default:
      Serial.println("ERROR");
      state = REPOSO;
      break;
   }
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
    setPixel_notHue(i, colorCircuito[0], colorCircuito[1], colorCircuito[2]); 
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
  if (count == 1) setAll(red, green, blue);
  else if (count == fOn) setAll(0,0,0);
  else if (count == (fOn+fOff)) count = 0;
}

void party_1() {
  static uint16_t count = 0;
  if (gFlag) {
    count = 0;
    gFlag = false;
  }
  count++;
  if (count == 1) setAll(255,0,0);
  else if (count == 500) setAll(0, 255, 0);
  else if (count == 1000) setAll(0, 0, 255);
  else if (count == 1500) count = 0;
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
  setAll(0,0,0);
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if ( (i >= 31 && i <=35) || i == 38 || i == 42 || (i >= 45 && i <=49) || (i >= 52 && i <=56) ) setPixel(i, red, green, blue);
    else if ( i == 61 || i == 65 || i == 68 || i == 72 || i == 77 || i == 82 || i == 86  ) setPixel(i, red, green, blue);
    else if ( i == 91 || i == 95 || i == 98 || i == 102 || i == 107 || i == 112 || i == 116  ) setPixel(i, red, green, blue);
    else if ( (i >= 121 && i <=125) || i == 128 || i == 132 || i == 137 || i == 142 || i == 146  ) setPixel(i, red, green, blue);
    else if ( i == 151 || i == 158 || i == 162 || i == 167 || i == 172 || i == 176  ) setPixel(i, red, green, blue);
    else if ( i == 181 || i == 188 || i == 192 || i == 197 || i == 202 || i == 206  ) setPixel(i, red, green, blue);
    else if ( i == 211 || i == 218 || i == 222 || i == 227 || i == 232 || i == 236  ) setPixel(i, red, green, blue);
    else if ( i == 241 || i == 257 ) setPixel(i, red, green, blue);
    else if ( (i >= 248 && i <=252) || (i >= 262 && i <=266)) setPixel(i, red, green, blue);
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
  TIMSK2 = TIMSK2 | 0b00000001;            // Habilitación de la bandera 0 del registro que habilita la interrupción por sobre flujo o desbordamiento del TIMER2.
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

void setPixel(int Pixel, byte red, byte green, byte blue) {
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

void setPixel_notHue(int Pixel, byte red, byte green, byte blue) {
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

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
}


void setAll_notHue(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel_notHue(i, red, green, blue);
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
