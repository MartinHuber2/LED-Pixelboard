#include <FastLED.h>

#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

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

unsigned char TxtDemo[] = {"Temp:"};
unsigned char TxtDemo2[] = {"_____"};


const char* ssid = "HTLinn3";
const char* password = "ErenBaiter77";
String openWeatherMapApiKey = "22934d95dbf682c1b09b6d89cafc948b";
String city = "Innsbruck";
String countryCode = "AT";
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
String jsonBuffer;



double temperatur;                 

void setup()
{ 
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  
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
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
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



  String messString2 = String(temperatur);
  
  for(int i = 0; i < 5; i++){
      TxtDemo2[i] = ' ';
    }
    
  for(int i = 0; i < messString2.length(); i++){
      TxtDemo2[i] = messString2[i];
    }
  
  

    ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    ScrollingMsg.UpdateText();
  

  

    ScrollingMsg2.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    ScrollingMsg2.UpdateText();
  
  FastLED.show();
  delay(200); 
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
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
