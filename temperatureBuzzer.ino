#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6Sw7PhL4h"
#define BLYNK_TEMPLATE_NAME "temperatureBuzzer"
#define BLYNK_AUTH_TOKEN "rRjuKU_ZNu7DC7Fq_7XMcPCizwAKdWf8"
// Database information
#define API_KEY "AIzaSyDOpapodHt8TPLi9lDIE4w6Ow59H-6hQSI"
#define DATABASE_URL "https://temperaturebuzzer-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* Define the user Email and password that already registered or added to the project */
#define USER_EMAIL "thisishell@gmail.com"
#define USER_PASSWORD "bruhhh"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <Blynk.h>
#include <NTPClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//  Use the NTP (Network Time Protocol) to get the time and date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

#define ONE_WIRE_BUS 14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//define path in the database
String prPath = "/temperatureSensor";
String timePath = "/temperatureSensor";
String databasePath = "/asm2";
String parentPath;

const int buzzerPin = 27;
const float temperatureThreshold = 32;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Redmi 9T";
char pass[] = "20033002";
unsigned long sendDataPrevMillis = 0;

void setup()
{ 
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);
  sensors.begin();
  timeClient.begin();
	timeClient.setTimeOffset(25200);
  pinMode(buzzerPin, OUTPUT);  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  
  //Assign the callback function for the long running token generation task
	config.token_status_callback = tokenStatusCallback;
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
  String datetime = getDatetime();
  float temperature = sensors.getTempCByIndex(0);
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
    parentPath= databasePath + "/" + datetime;
    sendDataPrevMillis = millis();
    //set the JSON string
 		json.set(timePath.c_str(), temperature);
    if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json)){
 			Serial.println("PASSED");
 			Serial.println("PATH: " + fbdo.dataPath());
			Serial.println("TYPE: " + fbdo.dataType());
 		}
 		else {
 			Serial.println("FAILED");
 			Serial.println("REASON: " + fbdo.errorReason());
 		}
  }
  Serial.println();
  if(Blynk.connected()){Blynk.virtualWrite(V1, temperature);}
  if (temperature == -127.00) {
    Serial.println("Failed to read temperature from Dallas 18B20 sensor!");
    return;
  }
  Serial.print("time: ");
  Serial.println(datetime);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  if (temperature > temperatureThreshold) {
    Serial.print("Your house is burning!!!");
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

//define the function to get the datetime
String getDatetime(){
 	timeClient.update();
 	time_t epochTime = timeClient.getEpochTime();
 	struct tm *ptm = gmtime ((time_t *)&epochTime);
 	int monthDay = ptm->tm_mday;
 	int currentMonth = ptm->tm_mon+1;
 	int currentYear = ptm->tm_year+1900;
 	String formattedTime = timeClient.getFormattedTime();
 	return String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear) + " " + formattedTime;
}