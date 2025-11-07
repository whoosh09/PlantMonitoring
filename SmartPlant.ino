#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Servo.h>
#include "DHT.h"
#include "secrets.h"  

#define SERVO_PIN D2  
#define SOIL_PIN A0
#define RED_PIN D1
#define GREEN_PIN D3
#define BLUE_PIN D5
#define DHTPIN D4
#define DHTTYPE DHT22


WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Servo shadeServo;

// Telegram
String botToken = TELEGRAM_BOT_TOKEN;
String chatID = TELEGRAM_CHAT_ID;

// State Variables
long lastMsg = 0;
char msg[200];
int currentAngle = 180;
int shadeVal = 180;   
bool alertSent = false;

enum MoistureState {
  STATE_DRY,
  STATE_PERFECT,
  STATE_WET,
  STATE_UNKNOWN
};

MoistureState lastState = STATE_UNKNOWN;

// Smooth Servo Movement
void moveServoSmooth(int targetAngle) {
  int step = (targetAngle > currentAngle) ? 1 : -1;
  for (int pos = currentAngle; pos != targetAngle; pos += step) {
    shadeServo.write(pos);
    delay(10);
  }
  currentAngle = targetAngle;
}

// Telegram Alert
void sendTelegram(String text) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    text.replace(" ", "%20");
    text.replace("\n", "%0A");

    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken +
                 "/sendMessage?chat_id=" + chatID +
                 "&parse_mode=Markdown" +
                 "&text=" + text;

    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[Telegram] Sent: %s\n", text.c_str());
    } else {
      Serial.printf("[Telegram] Failed: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  Serial.println(message);

  // Shade Control via NetPie
  if (String(topic) == "@msg/shade") {
    int val = message.toInt();
    moveServoSmooth(val);
    shadeVal = val;
    
    String json = "{\"data\":{\"shade\":" + String(shadeVal) + "}}";
    client.publish("@shadow/data/update", json.c_str());
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe("@msg/shade");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  dht.begin();
  delay(2000);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  client.setServer("broker.netpie.io", 1883);
  client.setCallback(callback);

  shadeServo.attach(SERVO_PIN);
  shadeServo.write(180); 
  currentAngle = 180;

  sendTelegram("🤖 *AgentMoist* is online and watching your plant!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) { 
    lastMsg = now;

    // Read soil moisture
    int raw = analogRead(SOIL_PIN);
    int moisturePercent = map(raw, 1023, 0, 0, 100);

    // Read DHT
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT22!");
      return;
    }

    Serial.printf("Temp: %.1f °C | Humidity: %.1f %% | Raw: %d | Moisture: %d%%\n", 
                  temperature, humidity, raw, moisturePercent);

    // Moisture state 
    MoistureState currentState;
    if (moisturePercent < 40) {
      currentState = STATE_DRY;
    } else if (moisturePercent <= 70) {
      currentState = STATE_PERFECT;
    } else {
      currentState = STATE_WET;
    }

    switch (currentState) {
      case STATE_DRY:
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, LOW);
        break;
      case STATE_PERFECT:
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(BLUE_PIN, LOW);
        break;
      case STATE_WET:
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, HIGH);
        break;
      default:
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, LOW);
        break;
    }

    // Publish to NetPie 
    String data = "{\"data\": {";
    data += "\"temperature\":" + String(temperature, 1) + ",";
    data += "\"humidity\":" + String(humidity, 1) + ",";
    data += "\"moisture\":" + String(moisturePercent) + ",";
    data += "\"shade\":\"" + String(shadeVal) + "\"";
    data += "}}";

    data.toCharArray(msg, data.length() + 1);
    client.publish("@shadow/data/update", msg);
    Serial.println("Data sent to Netpie");

    // Telegram Alerts
    if (currentState != lastState) {
      if (currentState == STATE_DRY) {
        sendTelegram("⚠️ *Alert:* Your plant is too dry!\nMoisture: *" + String(moisturePercent) + "%*");
      } else if (currentState == STATE_PERFECT) {
        sendTelegram("🌿 *Perfect moisture!*\nYour plant is thriving at *" + String(moisturePercent) + "%*");
      } else if (currentState == STATE_WET) {
        sendTelegram("💧 *Warning:* Too much water!\nMoisture: *" + String(moisturePercent) + "%*");
      }
      lastState = currentState;
    }
  }
  delay(1);
}
