#include <WiFi.h>
#include <HTTPClient.h>

//
// ==================== PIN DEFINITIONS ====================
//
int TRIG = 5;
int ECHO = 18;
int LED_PIN = 25;
int BUZZER = 21;
int LDR_PIN = 34;
int POT_PIN = 35;
int SOS_BUTTON = 23;

//
// ==================== WIFI + TELEGRAM SETTINGS ====================
//
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* BOT_TOKEN = "7926861177:AAE0KBeTVM2xyVZmhMZsCt761sSLXxPZsws";   // your bot token
const char* CHAT_ID   = "7901539070";                                       // your chat ID

//
// ==================== SETUP ====================
//
void setup() {

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SOS_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("System Starting...");
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected Successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

//
// ==================== DISTANCE SENSOR FUNCTION ====================
//
long getDistance() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  return distance;
}

//
// ==================== TELEGRAM SOS MESSAGE (ROBUST VERSION) ====================
//
void sendTelegramSOS() {

  bool messageSent = false; // Flag to track success

  // 1. Try to connect and send if WiFi appears connected
  if (WiFi.status() == WL_CONNECTED) {
    
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String body =
      "{"
        "\"chat_id\": \"" + String(CHAT_ID) + "\","
        "\"text\": \"🚨 SOS ALERT 🚨\\nRahul needs help!\\n"
                   "Location: https://maps.google.com/?q=10.9020,76.9036\","
        "\"parse_mode\": \"Markdown\""
      "}";

    int response = http.POST(body);

    Serial.print("Telegram Response Code: ");
    Serial.println(response);

    // If response is 200 (OK), mark as success
    if (response > 0) {
      Serial.println("Response body:");
      Serial.println(http.getString());
      messageSent = true; 
    } else {
      Serial.println("Error: Connected to WiFi but Internet failed (DNS/Server Error).");
    }

    http.end();
  } 
  else {
    Serial.println("Error: WiFi Not Connected.");
  }

  // 2. FALBACK: If the message was NOT sent successfully (for ANY reason)
  if (messageSent == false) {
    
    Serial.println("Playing Backup Alarm...");

  
    for(int i = 0; i < 5; i++) {
      tone(BUZZER, 2500);        
      digitalWrite(LED_PIN, HIGH);
      delay(2000);               
      
      noTone(BUZZER);
      digitalWrite(LED_PIN, LOW);
      delay(500);                
    }
  }
}
//
// ==================== BUZZER ALERT PATTERN ====================
//
void alertPattern(int speed, int toneFreq) {
  tone(BUZZER, toneFreq);
  delay(speed);
  noTone(BUZZER);
  delay(speed);
}

//
// ==================== SOS ALERT PATTERN (LED + BUZZER) ====================
//
void sosAlert() {

  for (int i = 0; i < 5; i++) {

    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER, 3000);
    delay(200);

    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER);
    delay(200);
  }
}

//
// ==================== MAIN LOOP ====================
//
void loop() {

  long distance = getDistance();
  int ldrValue = analogRead(LDR_PIN);
  int potValue = analogRead(POT_PIN);
  int threshold = map(potValue, 0, 4095, 20, 100);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Threshold: ");
  Serial.print(threshold);
  Serial.print(" | LDR: ");
  Serial.println(ldrValue);

  //
  // ----------- LDR AUTO LIGHT CONTROL -----------
  //
  if (ldrValue > 1500)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  //
  // ----------- SOS BUTTON LOGIC -----------
  //
  if (digitalRead(SOS_BUTTON) == LOW) {

    delay(50); // debounce

    if (digitalRead(SOS_BUTTON) == LOW) {

      Serial.println("\n===== 🚨 SOS BUTTON PRESSED 🚨 =====");
      Serial.println("SENDING TELEGRAM ALERT...");

      sendTelegramSOS();
      sosAlert();

      while (digitalRead(SOS_BUTTON) == LOW) delay(50);
      delay(200);
    }
  }

  //
  // ----------- BUZZER DISTANCE ALERT -----------
  //
  if (distance >= threshold || distance <= 0) {

    noTone(BUZZER);

  } else if (distance >= 50) {

    alertPattern(200, 1000);

  } else if (distance >= 30) {

    alertPattern(120, 1500);

  } else if (distance >= 15) {

    alertPattern(70, 2000);

  } else {

    tone(BUZZER, 2500);
  }

  delay(50);
}
