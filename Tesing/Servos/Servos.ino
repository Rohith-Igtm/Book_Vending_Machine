#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <time.h>

// WiFi Credentials
#define WIFI_SSID "Redmi Note 7 Pro"
#define WIFI_PASSWORD "rohithaa"

// Firebase Credentials
#define FIREBASE_HOST "book-vending-52801-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyCyNs09bLpZ_t9KSTDkXAEv5fH6JuqQluU"
#define FIREBASE_EMAIL "bookvending@gmail.com"
#define FIREBASE_PASSWORD "bookvending"

// Pin Configuration
#define MG90S_PIN 12
#define MG996R_PIN 13
#define BUTTON1_PIN 14
#define BUTTON2_PIN 27
#define BUTTON3_PIN 26
#define BUTTON4_PIN 25

// Servo Objects
Servo mg90s;
Servo mg996r;

// Time Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Debouncing
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 500;

void setup() {
  Serial.begin(115200);

  // Initialize buttons
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  // Initialize Servos
  mg996r.attach(MG996R_PIN, 500, 2500);
  mg90s.attach(MG90S_PIN, 500, 2500);
  mg996r.write(0);
  mg90s.write(0);
  delay(1000);

  // Connect to WiFi
  connectToWiFi();

  // Configure Firebase
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Configure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  verifyTimeSync();

  Serial.println("System Ready - Press any button to dispense book");
}

void loop() {
  handleButtons();
}

// WiFi Connection Handler
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// Time Synchronization Verifier
void verifyTimeSync() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get NTP time, retrying...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(2000);
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Permanent NTP failure!");
      while (1) delay(1000);
    }
  }
  Serial.println("NTP time synchronized");
}

// Button Handler with Debouncing
void handleButtons() {
  if ((millis() - lastButtonPress) < debounceDelay) return;

  if (digitalRead(BUTTON1_PIN) == LOW) {
    dispenseBook(0, "Book1");
  } else if (digitalRead(BUTTON2_PIN) == LOW) {
    dispenseBook(60, "Book2");
  } else if (digitalRead(BUTTON3_PIN) == LOW) {
    dispenseBook(120, "Book3");
  } else if (digitalRead(BUTTON4_PIN) == LOW) {
    dispenseBook(180, "Book4");
  }
}

// Book Dispensing Sequence
void dispenseBook(int angle, String bookName) {
  lastButtonPress = millis();

  // Rotate to position
  if (angle > 0) {
    mg996r.write(angle);
    delay(1500);
  }

  // Push book
  mg90s.write(180);  // Full extension
  delay(1000);

  // Return to home
  mg996r.write(0);
  mg90s.write(0);
  delay(1000);

  // Firebase Update
  sendToFirebase(bookName);
  Serial.println("Book Dispensed: " + bookName);
}

// Firebase Update
void sendToFirebase(String bookName) {
  if (!Firebase.ready()) {
    Serial.println("Firebase not ready!");
    return;
  }

  String timestamp = getCurrentTime();
  String basePath = "/Logs/DispensedBooks/";

  FirebaseJson json;
  json.set(sanitizeKey(bookName), timestamp);

  if (Firebase.updateNode(fbdo, basePath, json)) {
    Serial.println("✔ Data sent to Firebase");
  } else {
    Serial.println("❌ Firebase error: " + fbdo.errorReason());
  }
}

// Time Formatter
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "0000-00-00 00:00:00";

  char timeBuffer[20];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeBuffer);
}

// Firebase Key Sanitizer
String sanitizeKey(String input) {
  input.replace(" ", "_");
  input.replace(".", "");
  input.replace("#", "");
  input.replace("$", "");
  input.replace("[", "");
  input.replace("]", "");
  return input;
}
