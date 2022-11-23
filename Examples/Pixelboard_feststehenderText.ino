#include <FastLED.h>

#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

// Change the next 6 defines to match your matrix type and size

#define LED_PIN        33
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX

int z = 15;

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ScrollingMsg;

unsigned char TxtDemo[] = { " Z=      "};


void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  FastLED.show();

  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}


void loop() {
  z--;
  if (z == 0) z = 15;
  
  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.UpdateText();

  String zString = String(z);
  for (int i = 0; i < 2; i++) { 
    TxtDemo[i + 3] = ' ';
    TxtDemo[(i + 3)] = zString[i];
  }
  
  FastLED.show();
  delay(1000);
}
