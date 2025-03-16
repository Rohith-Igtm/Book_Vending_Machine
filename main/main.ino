#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <time.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi Credentials
#define WIFI_SSID "Redmi Note 7 Pro"
#define WIFI_PASSWORD "rohithaa"

// Firebase Credentials
#define FIREBASE_HOST "book-vending-52801-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyCyNs09bLpZ_t9KSTDkXAEv5fH6JuqQluU"
#define FIREBASE_EMAIL "bookvending@gmail.com"
#define FIREBASE_PASSWORD "bookvending"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Servo mg90s;
Servo mg996r;

int mg90sPin = 12;
int mg996rPin = 13;

int button1 = 14;
int button2 = 27;
int button3 = 26;
int button4 = 25;

// RFID Setup
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

String authorizedUID1 = "F380FA2E";
String authorizedUID2 = "B3876AF5";
String currentUser = "";
bool userAuthorized = false;
bool bookTaken = false;

// NTP Time Setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// Function Prototypes
void checkRFID();
void sendToFirebase(String studentID, String bookName);
String getCurrentTime();
void resetSession();

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  mg90s.attach(mg90sPin);
  mg996r.attach(mg996rPin);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);

  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) Serial.println("Failed to get time from NTP");
  else Serial.println("NTP time acquired successfully.");

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Scan your RFID card...");
}

void loop() {
  if (!userAuthorized || bookTaken) {
    checkRFID();
    return;
  }

  if (digitalRead(button1) == LOW) {
    Serial.println("Button 1 Pressed");
    dispenseBook(0, "Book1");
  }
  else if (digitalRead(button2) == LOW) {
    Serial.println("Button 2 Pressed");
    dispenseBook(60, "Book2");
  }
  else if (digitalRead(button3) == LOW) {
    Serial.println("Button 3 Pressed");
    dispenseBook(120, "Book3");
  }
  else if (digitalRead(button4) == LOW) {
    Serial.println("Button 4 Pressed");
    dispenseBook(180, "Book4");
  }
}

void checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println("RFID UID: " + uid);

  if (uid == authorizedUID1) {
    currentUser = "Rohith";
    userAuthorized = true;
    bookTaken = false;
    Serial.println("Access Granted: Rohith");
  }
  else if (uid == authorizedUID2) {
    currentUser = "Siddarth";
    userAuthorized = true;
    bookTaken = false;
    Serial.println("Access Granted: Siddarth");
  }
  else {
    Serial.println("Access Denied");
    userAuthorized = false;
    currentUser = "";
  }

  // End the current RFID session
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Wait until the card is removed before allowing another scan
  while (mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
  }
}

void dispenseBook(int rotationAngle, String bookName) {
  if (rotationAngle > 0) {
    mg996r.write(rotationAngle);
    delay(1000);
  }
  mg90s.write(50);
  delay(1000);
  mg996r.write(0);
  sendToFirebase(currentUser, bookName);
  mg90s.write(0);
  delay(1000);
  bookTaken = true;
  userAuthorized = false;
  currentUser = "";
  Serial.println("Please scan your RFID again for next book.");
}

String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "TimeError";
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}

void sendToFirebase(String studentID, String bookName) {
  FirebaseJson json;
  String timestamp = getCurrentTime();

  // Build a JSON object with your log data
  json.add("student", studentID);
  json.add("book", bookName);
  json.add("timestamp", timestamp);

  // Push the JSON to Firebase; this creates a unique key that sorts by time
  if (Firebase.pushJSON(fbdo, "/logs", json))
    Serial.println("✔ Data sent to Firebase");
  else 
    Serial.println("❌ Failed to send data: " + fbdo.errorReason());

  Serial.println("Data sent to Firebase:");
  Serial.println("Student: " + studentID);
  Serial.println("Book: " + bookName);
  Serial.println("Timestamp: " + timestamp);
}

void resetSession() {
  // Optional: reset any session data if needed
  userAuthorized = false;
  bookTaken = false;
  currentUser = "";
}











