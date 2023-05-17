#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

#define LED_PIN 27
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define NUM_LEDS 256

#define MATRIX_WIDTH -32
#define MATRIX_HEIGHT -8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX

cLEDMatrix < -MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE > leds;
cLEDText ScrollingMsg;

TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;
TaskHandle_t admin_Handle;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(60);
  FastLED.clear(true);
  delay(500);
  FastLED.show();
  xTaskCreatePinnedToCore(
    admin, "admin", 4096, NULL, 1, &admin_Handle, 0);

  xTaskCreatePinnedToCore(
    Task1, "Task1", 4096, NULL, 2, &Task1_Handle, 0);

  xTaskCreatePinnedToCore(
    Task2, "Task2", 4096, NULL, 2, &Task2_Handle, 1);

  vTaskSuspend(Task1_Handle);
  vTaskSuspend(Task2_Handle);
  vTaskResume(admin_Handle);
}

void loop() {
}

void admin(void *pvParameters) {
  (void)pvParameters;

  vTaskDelay(10);
  for (;;) {
    vTaskResume(Task1_Handle);
    vTaskSuspend(Task2_Handle);
    vTaskDelay(3000);
    FastLED.clear(true);
    vTaskResume(Task2_Handle);
    vTaskSuspend(Task1_Handle);
    vTaskDelay(3000);
    FastLED.clear(true);
  }
}

void Task1(void *pvParameters) {
  (void)pvParameters;

  unsigned char TxtDemo4[] = { "Hallo" };

  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);

  ScrollingMsg.SetText((unsigned char *)TxtDemo4, sizeof(TxtDemo4) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x00, 0x00, 0xff);

  vTaskDelay(20);
  for (;;) {
    Serial.print("A");
    ScrollingMsg.SetText((unsigned char *)TxtDemo4, sizeof(TxtDemo4) - 1);
    ScrollingMsg.UpdateText();
    FastLED.show();
    vTaskDelay(100);
  }
}
void Task2(void *pvParameters) {
  (void)pvParameters;
  vTaskDelay(20);
  for (;;) {
    FastLED.clear();
    Serial.print("B");
    leds[0][20] = CRGB::Red;
    leds[0][25] = CRGB::Red;
    FastLED.show();
    vTaskDelay(100);
  }
}
