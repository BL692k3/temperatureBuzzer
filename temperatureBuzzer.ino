#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6Sw7PhL4h"
#define BLYNK_TEMPLATE_NAME "temperatureBuzzer"
#define BLYNK_AUTH_TOKEN "rRjuKU_ZNu7DC7Fq_7XMcPCizwAKdWf8"
// Database information
#define API_KEY "AIzaSyDOpapodHt8TPLi9lDIE4w6Ow59H-6hQSI"
#define DATABASE_URL "https://temperaturebuzzer-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* Define the user Email and password that alreadey registerd or added in the project */
#define USER_EMAIL "thisishell@gmail.com"
#define USER_PASSWORD "bruhhh"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <Blynk.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

#define ONE_WIRE_BUS 17
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int buzzerPin = 16;
const float temperatureThreshold = 35.0;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Cherry Coffee";
char pass[] = "23456789";
unsigned long sendDataPrevMillis = 0;


void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);
  sensors.begin();
  pinMode(buzzerPin, OUTPUT);  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
 }

void loop()
{
  Blynk.run();
  Firebase.ready();
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    Serial.printf("Set temperature... %s\n", Firebase.setFloat(fbdo, F("/Temp"), temperature) ? "ok" : fbdo.errorReason().c_str());
    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse.ino
    FirebaseJson json;
  }
  Serial.println();

  Blynk.virtualWrite(V1, temperature);
  if (temperature == -127.00) {
    Serial.println("Failed to read temperature from Dallas 18B20 sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  if (temperature > temperatureThreshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    digitalWrite(buzzerPin, LOW);
    delay(5000);
  }
}