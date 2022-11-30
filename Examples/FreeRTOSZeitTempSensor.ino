#include <FastLED.h>
#include <JS.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "time.h"

#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22

#define LED_PIN 32
#define LED_PIN2  14
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

cLEDMatrix < -MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE > leds;
cLEDMatrix < MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE > leds2;


cLEDText ScrollingMsg;
cLEDText ScrollingMsg2;

DHT dht(DHTPIN, DHTTYPE);



const char* ssid = "HOTSPOT";
const char* password = "PASSWORD";

double temperatur;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// Flankenerkennung

JS joystick(25, 26, 27);


TaskHandle_t TaskAusgabeA_Handle;
TaskHandle_t TaskAusgabeB_Handle;
TaskHandle_t TaskAusgabeC_Handle;
TaskHandle_t TaskGetWeatherInfo_Handle;
TaskHandle_t TaskHumidity_Handle;


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(115200);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());







  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  FastLED.show();

  FastLED.addLeds<CHIPSET, LED_PIN2, COLOR_ORDER>(leds2[0], leds2.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);


  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);


  ScrollingMsg2.SetFont(MatriseFontData);
  ScrollingMsg2.Init(&leds2, leds2.Width(), ScrollingMsg2.FontHeight() + 1, 0, 0);


  // Task Definition

  xTaskCreatePinnedToCore(
    TaskHauptprogramm, "TaskHauptprogramm", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskGetWeatherInfo, "TaskWeatherInfo", 4096, NULL, 1, &TaskGetWeatherInfo_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeA, "TaskAusgabeA", 4096, NULL, 2, &TaskAusgabeA_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeB, "TaskAusgabeB", 4096, NULL, 2, &TaskAusgabeB_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskAusgabeC, "TaskAusgabeC", 4096, NULL, 2, &TaskAusgabeC_Handle, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskHumidity, "TaskHumidity", 4096, NULL, 2, &TaskHumidity_Handle, ARDUINO_RUNNING_CORE);
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
      counter = (counter + 1) % 4;
      FastLED.clear();
    }
    //Serial.println(c2);
    switch (counter) {
      case 0:
        //Serial.println("A");
        vTaskSuspend(TaskAusgabeB_Handle);
        vTaskSuspend(TaskAusgabeC_Handle);
        vTaskSuspend(TaskHumidity_Handle);
        vTaskResume(TaskAusgabeA_Handle);
        break;
      case 1:
        //Serial.println("B");
        vTaskSuspend(TaskAusgabeA_Handle);
        vTaskSuspend(TaskAusgabeC_Handle);
        vTaskSuspend(TaskHumidity_Handle);
        vTaskResume(TaskAusgabeB_Handle);
        break;
      case 2:
        //Serial.println("C");
        vTaskSuspend(TaskAusgabeA_Handle);
        vTaskSuspend(TaskAusgabeB_Handle);
        vTaskSuspend(TaskHumidity_Handle);
        vTaskResume(TaskAusgabeC_Handle);
        break;
      case 3:
        //Serial.println("C");
        vTaskSuspend(TaskAusgabeA_Handle);
        vTaskSuspend(TaskAusgabeB_Handle);
        vTaskSuspend(TaskAusgabeC_Handle);
        vTaskResume(TaskHumidity_Handle);
        break;
      default:
        break;
    }
    vTaskDelay(100);
  }
}





void TaskGetWeatherInfo(void *pvParameters)
{
  (void)pvParameters;

  String openWeatherMapApiKey = "22934d95dbf682c1b09b6d89cafc948b";
  String city = "Innsbruck";
  String countryCode = "AT";
  unsigned long lastTime = 0;
  unsigned long timerDelay = 20000;
  String jsonBuffer;


  for (;;)
  {
    vTaskDelay(100);
    if ((millis() - lastTime) > timerDelay) {
      // Check WiFi connection status
      if (WiFi.status() == WL_CONNECTED) {
        String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

        jsonBuffer = httpGETRequest(serverPath.c_str());
        Serial.println(jsonBuffer);
        JSONVar myObject = JSON.parse(jsonBuffer);

        // JSON.typeof(jsonVar) can be used to get the type of the var
        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          return;
        }



        Serial.print("JSON object = ");
        Serial.println(myObject);
        Serial.print("Temperature: ");
        temperatur = (double(myObject["main"]["temp"])) - (273.15);
        Serial.println(temperatur);
      }
      else {
        Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
    }

  }
  vTaskDelay(60000);
}

void TaskAusgabeA(void *pvParameters)
{
  (void)pvParameters;

  unsigned char TxtDemo[] = {  "            Wert =   "};

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);

  FastLED.show();

  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg2.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);


  for (;;) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    const char* s = (&timeinfo, "%M");
    char timeSek[3];
    strftime(timeSek, 3, "%M", &timeinfo);
    String messString = String(timeSek);
    char timeMin[3];
    strftime(timeMin, 3, "%I", &timeinfo);
    String messString2 = String(timeMin);


    String mString = String(messString);
    String zString = String(messString2);
    for (int i = 0; i < 2; i++) {
      TxtDemo[i + 3] = ' ';
      TxtDemo[(i + 3)] = mString[i];
      TxtDemo[i + 1] = ':';
      TxtDemo[i] = ' ';
      TxtDemo[ (i) ] = zString[i];
    }
    ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg.UpdateText();

    const char* d = (&timeinfo, "%d");
    char timed[3];
    strftime(timed, 3, "%d", &timeinfo);
    String messString3 = String(timed);
    char timeA[3];
    strftime(timeA, 3, "%A", &timeinfo);
    String messString4 = String(timeA);


    String mString3 = String(messString3);
    String zString4 = String(messString4);
    for (int i = 0; i < 2; i++) {
      TxtDemo[i + 3] = ' ';
      TxtDemo[(i + 3)] = mString3[i];
      TxtDemo[i + 1] = ':';
      TxtDemo[i] = ' ';
      TxtDemo[ (i) ] = zString4[i];
    }
    ScrollingMsg2.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg2.UpdateText();



    FastLED.show();
    vTaskDelay(100);
  }
}

void TaskAusgabeB(void *pvParameters)
{
  (void)pvParameters;

  unsigned char TxtDemo[] = {"Temp:"};
  unsigned char TxtDemo2[] = {"_____"};



  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg2.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
  ScrollingMsg2.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  for (;;) {
    vTaskDelay(100);
    String messString2 = String(temperatur);

    for (int i = 0; i < 5; i++) {
      TxtDemo2[i] = ' ';
    }

    for (int i = 0; i < messString2.length(); i++) {
      TxtDemo2[i] = messString2[i];
    }



    ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    ScrollingMsg.UpdateText();




    ScrollingMsg2.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg2.UpdateText();

    FastLED.show();
    delay(200);
  }
}






void TaskAusgabeC(void *pvParameters) {
  (void)pvParameters;

  unsigned char TxtDemo[] = {"RaumT"};
  unsigned char TxtDemo2[] = {"_____"};

  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg2.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
  ScrollingMsg2.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  for (;;)
  {
    vTaskDelay(100);

    float t = dht.readTemperature();

    String messString2 = String(t);

    for (int i = 0; i < 5; i++) {
      TxtDemo2[i] = ' ';
    }

    for (int i = 0; i < messString2.length(); i++) {
      TxtDemo2[i] = messString2[i];
    }


    ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    ScrollingMsg.UpdateText();


    ScrollingMsg2.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg2.UpdateText();

    FastLED.show();
    delay(200);
  }

}

void TaskHumidity(void *pvParameters) {
  (void)pvParameters;

  unsigned char TxtDemo[] = {"Humid"};
  unsigned char TxtDemo2[] = {"_____"};

  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg2.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
  ScrollingMsg2.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  for (;;)
  {
    vTaskDelay(100);

    float h = dht.readHumidity();

    String messString2 = String(h);

    for (int i = 0; i < 5; i++) {
      TxtDemo2[i] = ' ';
    }

    for (int i = 0; i < messString2.length(); i++) {
      TxtDemo2[i] = messString2[i];
    }


    ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    ScrollingMsg.UpdateText();


    ScrollingMsg2.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg2.UpdateText();

    FastLED.show();
    delay(200);
  }

}





String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeSek[3];
  strftime(timeSek, 3, "%S", &timeinfo);
  Serial.println(timeSek);

}
