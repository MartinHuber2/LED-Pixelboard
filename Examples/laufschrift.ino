#include <FastLED.h>

#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

#define LED_PIN1       32
#define LED_PIN2       26
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;
cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> leds2;

cLEDText ScrollingMsg;
cLEDText ScrollingMsg2;

const unsigned char TxtDemo[] = { EFFECT_SCROLL_LEFT "      Text oben "};
const unsigned char TxtDemo2[] = { EFFECT_SCROLL_LEFT "      Text unten "};
                                  

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  
  FastLED.addLeds<CHIPSET, LED_PIN2, COLOR_ORDER>(leds2[0], leds2.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);

  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  ScrollingMsg2.SetFont(MatriseFontData);
  ScrollingMsg2.Init(&leds2, leds2.Width(), ScrollingMsg2.FontHeight() + 1, 0, 0);
  ScrollingMsg2.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
  ScrollingMsg2.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}


void loop()
{
  
  if (ScrollingMsg.UpdateText() == -1){
    ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  }

  
  if (ScrollingMsg2.UpdateText() == -1){
    ScrollingMsg2.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
  }
  
  FastLED.show();
  delay(200);
  
  
}
