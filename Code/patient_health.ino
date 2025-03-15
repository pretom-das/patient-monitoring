#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DHT.h>

#define DHT11_PIN 18
#define MQ7_PIN 34 // Analog pin for MQ-7
#define MQ135_PIN 35 // Analog pin for MQ-135
#define REPORTING_PERIOD_MS 2000
#define SDA_PIN 21              // Use GPIO 21 for SDA
#define SCL_PIN 22              // Use GPIO 22 for SCL
#define BUTTON_PIN 25           // GPIO pin for the button
#define BUZZER_PIN 26           // GPIO pin for the buzzer
#define FAN_PIN 33              // GPIO pin for the fan
#define HUMIDIFIER 32            // humidifier
#define Fire 19

unsigned long lastFanUpdate = 0;
unsigned long lastHumidifierUpdate = 0;
const int FAN_UPDATE_INTERVAL = 2000;         // Check fan every 2 seconds
const int HUMIDIFIER_UPDATE_INTERVAL = 2000;  // Check humidifier every 2 seconds

float temperature, humidity, BPM, SpO2, bodytemperature, coLevel, airQuality;
String emergencyStatus = "Normal";  // Default status
String fanStatus = "Off";            // Default fan status
String humidifier_status = "Off" ;
String fireStatus = "Normal";


const char* ssid = "MIST";
const char* password = "Il0veMIST";

// const char* ssid = "PIAL";
// const char* password = "Pi@ldas164";

// Initialize the MAX30100 Pulse Oximeter sensor
PulseOximeter pox;
MAX30100 sensor;


// Initialize DHT sensor
DHT dht(DHT11_PIN, DHT11);

WebServer server(80);

uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat detected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(MQ7_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with pull-up
  pinMode(BUZZER_PIN, OUTPUT);       // Set buzzer pin as output
  pinMode(FAN_PIN, OUTPUT);          // Set fan pin as output
  pinMode(Fire, INPUT); 
  pinMode(HUMIDIFIER, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);     // Ensure buzzer is off initially
  digitalWrite(FAN_PIN, HIGH);        // Ensure fan is off initially
  digitalWrite(HUMIDIFIER, HIGH);
  delay(100);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Initializing pulse oximeter...");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_50MA);

  // Initialize DHT sensor
  dht.begin();

}

void loop() {
    server.handleClient();
    pox.update();

    // Read DHT sensor
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read heart rate and SpO2 from MAX30100
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    // Read MQ-7 and MQ-135 sensor values
    coLevel = analogRead(MQ7_PIN);       // MQ-7 raw value
    airQuality = analogRead(MQ135_PIN);  // MQ-135 raw value

    // Check emergency button state
    if (digitalRead(BUTTON_PIN) == HIGH) {  
        digitalWrite(BUZZER_PIN, HIGH);
        emergencyStatus = "EMERGENCY!";
        //Serial.println("EMERGENCY! Button Pressed!");
    } else {
        digitalWrite(BUZZER_PIN, LOW);
        emergencyStatus = "Normal";
    }

    if (digitalRead(Fire) == LOW) {  
        fireStatus = "Fire!!!";
        //Serial.println("Fire! Fire!");
    } else {
        fireStatus = "Normal";
    }
    

    // **Non-blocking Fan Control**
    if (millis() - lastFanUpdate >= FAN_UPDATE_INTERVAL) {
        lastFanUpdate = millis();
        if (airQuality > 500) {  // Threshold for bad air quality
            digitalWrite(FAN_PIN, LOW);  // Turn on fan
            fanStatus = "On";           
        } else {
            digitalWrite(FAN_PIN, HIGH); // Turn off fan
            fanStatus = "Off";           
        }
    }

    // **Non-blocking Humidifier Control**
    if (millis() - lastHumidifierUpdate >= HUMIDIFIER_UPDATE_INTERVAL) {
        lastHumidifierUpdate = millis();
        if (humidity < 55) {  
            digitalWrite(HUMIDIFIER, LOW);  // Turn on humidifier
            humidifier_status = "On";
        } else {
            digitalWrite(HUMIDIFIER, HIGH); // Turn off humidifier
            humidifier_status = "Off";
        }
    }

    // Report every REPORTING_PERIOD_MS
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Room Temperature: "); Serial.print(temperature); Serial.println("°C");
        Serial.print("Room Humidity: "); Serial.print(humidity); Serial.println("%");
        Serial.print("BPM: "); Serial.println(BPM);
        Serial.print("SpO2: "); Serial.println(SpO2);
        Serial.print("CO Level (Raw): "); Serial.println(coLevel);
        Serial.print("Air Quality (Raw): "); Serial.println(airQuality);
        Serial.print("Emergency Status: "); Serial.println(emergencyStatus);
        Serial.print("Fan Status: "); Serial.println(fanStatus);
        Serial.print("Humidifier Status: "); Serial.println(humidifier_status);
        Serial.print("Fire Status: "); Serial.println(fireStatus);
        Serial.println("");
        tsLastReport = millis();
    }
}


void handle_OnConnect() {
  String airQualityStatus = (airQuality < 1000) ? "Good" : "Bad";  // Threshold for air quality
  String coStatus = (coLevel < 800) ? "Good" : "Bad";  // Threshold for CO level

  server.send(200, "text/html", SendHTML(temperature, humidity, BPM, SpO2, bodytemperature, coLevel, airQuality, airQualityStatus, coStatus, emergencyStatus, fanStatus, fireStatus));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float humidity, float BPM, float SpO2, float bodytemperature, float coLevel, float airQuality, String airQualityStatus, String coStatus, String emergencyStatus, String fanStatus, String fireStatus) {
  
  String ptr = "<!DOCTYPE html><html>";
  ptr += "<head><meta charset='UTF-8'><title>Patient Health Monitoring</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<meta http-equiv='refresh' content='6'>"; // Auto-refresh the page every 10 seconds
  ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr += "<style>";

  ptr += "html { font-family: 'Open Sans', sans-serif; background-color: #121212; color: #f4f4f4; text-align: center; }";
  ptr += "h1 { color: #F29C1F; font-size: 36px; font-weight: 600; }";
  ptr += ".container { display: flex; flex-wrap: wrap; justify-content: center; padding: 20px; }";
  ptr += ".data { background-color: #1e1e1e; border-radius: 10px; width: 250px; margin: 15px; padding: 20px; }";
  ptr += ".text { font-size: 20px; font-weight: 600; color: #ffffff; }";
  ptr += ".reading { font-size: 40px; font-weight: 300; color: #F29C1F; }";
  ptr += ".emergency { color: red; font-size: 28px; font-weight: bold; margin-top: 20px; }";
  ptr += "</style></head>";

  ptr += "<body>";
  ptr += "<img src='https://afd.gov.bd/sites/default/files/inline-images/MIST%20Logo_0.png' class='header-logo' alt='<MIST Logo'>";
  ptr += "<h1>Patient Health and Room Monitoring</h1>";
  ptr += "<h3>Pretom-Farhan-Shabab</h3>";
  ptr += "<div class='container'>";

  ptr += "<div class='data'><div class='text'>Room Temperature</div>";
  ptr += "<div class='reading'>" + String((int)temperature) + "\u00B0C</div></div>";

  ptr += "<div class='data'><div class='text'>Room Humidity</div>";
  ptr += "<div class='reading'>" + String((int)humidity) + "%</div></div>";

  ptr += "<div class='data'><div class='text'>Heart Rate</div>";
  ptr += "<div class='reading'>" + String((int)BPM) + " BPM</div></div>";

  ptr += "<div class='data'><div class='text'>Blood Oxygen</div>";
  ptr += "<div class='reading'>" + String((int)SpO2) + "%</div></div>";


  ptr += "<div class='data'><div class='text'>CO Level</div>";
  ptr += "<div class='reading'>" + String((int)coLevel) + " ppm</div>";
  ptr += "<div class='text'>CO Status: " + coStatus + "</div></div>";

  ptr += "<div class='data'><div class='text'>Air Quality</div>";
  ptr += "<div class='reading'>" + String((int)airQuality) + " ppm</div>";
  ptr += "<div class='text'>Air Quality Status: " + airQualityStatus + "</div></div>";

  ptr += "<div class='data'><div class='text'>Fan Status</div>";
  ptr += "<div class='reading'>" + fanStatus + "</div></div>";

    ptr += "<div class='data'><div class='text'>humidifier Status</div>";
  ptr += "<div class='reading'>" + humidifier_status + "</div></div>";

  ptr += "<div class='data emergency'><div class='text'>Emergency Status</div>";
  ptr += "<div class='reading'>" + emergencyStatus + "</div></div>";

  ptr += "<div class='data emergency'><div class='text'>Fire Status</div>";
  ptr += "<div class='reading'>" + fireStatus + "</div></div>";

  ptr += "</div></body></html>";

  return ptr;
}