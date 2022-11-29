#include <FastLED.h>
#include <JS.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

#define LED_PIN 33
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX


#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// define two tasks for Blink & AnalogRead
void TaskBlink(void *pvParameters);
void TaskAnalogReadA3(void *pvParameters);

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ScrollingMsg;

// Flankenerkennung

JS joystick(25, 26, 27);


TaskHandle_t TaskAusgabeA_Handle;
TaskHandle_t TaskAusgabeB_Handle;
TaskHandle_t TaskAusgabeC_Handle;


// the setup function runs once when you press reset or power the board
void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  FastLED.show();

  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Task Definition

  xTaskCreatePinnedToCore(
    TaskHauptprogramm, "TaskHauptprogramm", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeA, "TaskAusgabeA", 4096, NULL, 2, &TaskAusgabeA_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeB, "TaskAusgabeB", 4096, NULL, 2, &TaskAusgabeB_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeC, "TaskAusgabeC", 4096, NULL, 2, &TaskAusgabeC_Handle, ARDUINO_RUNNING_CORE);
}

void loop() {}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskHauptprogramm(void *pvParameters)  // This is a task.
{
  (void)pvParameters;
  int counter = 0;
  int joystickValue = 0;
  int joystickValueAlt = 0;
  bool steigendeFlanke = false;
  int c2 = 0;

  vTaskDelay(10);
  for (;;) {
    joystickValue = joystick.getSW();
    steigendeFlanke = (!joystickValueAlt && joystickValue);
    joystickValueAlt = joystickValue;
    if (steigendeFlanke) {
      counter = (counter + 1) % 3;
      c2++;
    }
    //Serial.println(c2);
    switch (counter) {
      case 0:
        //Serial.println("A");
        vTaskSuspend(TaskAusgabeB_Handle);
        vTaskSuspend(TaskAusgabeC_Handle);
        vTaskResume(TaskAusgabeA_Handle);
        break;
      case 1:
        //Serial.println("B");
        vTaskSuspend(TaskAusgabeA_Handle);
        vTaskSuspend(TaskAusgabeC_Handle);
        vTaskResume(TaskAusgabeB_Handle);
        break;
      case 2:
        //Serial.println("C");
        vTaskSuspend(TaskAusgabeA_Handle);
        vTaskSuspend(TaskAusgabeB_Handle);
        vTaskResume(TaskAusgabeC_Handle);
        break;
      default:
        break;
    }
    vTaskDelay(100);
  }
}

void TaskAusgabeA(void *pvParameters)
{
  (void)pvParameters;

  unsigned char TxtDemo[] = { "A" };

  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  for (;;) {
    vTaskDelay(100);
    ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg.UpdateText();
    FastLED.show();
  }
}

void TaskAusgabeB(void *pvParameters)
{
  (void)pvParameters;

  unsigned char TxtDemo[] = { "B" };

  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  for (;;)
  {
    vTaskDelay(100);
    ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg.UpdateText();
    FastLED.show();
  }
}

void TaskAusgabeC(void *pvParameters)
{
  (void)pvParameters;

  unsigned char TxtDemo[] = { "C" };

  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  for (;;)
  {
    vTaskDelay(100);
    ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg.UpdateText();
    FastLED.show();
  }
}