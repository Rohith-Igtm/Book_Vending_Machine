# 📚 Book Vending Machine

A smart book vending machine system using **ESP32**, **RFID**, and **Firebase** for **student authentication and book issue logging**.

### 🔧 Features
- 📛 RFID-based student identification  
- 📖 Manual book name entry (via Serial Monitor)  
- ☁️ Logs book name and timestamp to **Firebase Realtime Database**  
- 📊 Simple, minimal, and cost-effective system

### 🛠️ Hardware Used
- ESP32 Dev Module  
- RC522 RFID Reader  
- RFID Tags  
- (Optional) Book dispensing mechanism can be added

### 📡 Software/Tools
- Arduino IDE  
- Firebase Realtime Database  
- ESP32 Board Package  
- Required Libraries:
  - `MFRC522`
  - `FirebaseESP32`
  - `WiFi`

### 🔗 Firebase Database Structure
```
/logs/
  └── StudentName/
        └── entryKey/
              ├── book: "Book Title"
              └── timestamp: "YYYY-MM-DD HH:MM:SS"
```

### 🚀 How it Works
1. Student scans RFID card → Name fetched  
2. Book name is entered via Serial Monitor  
3. Data (Book + Timestamp) is logged to Firebase  

### 📁 Setup
- Add your Wi-Fi and Firebase credentials in the code:
  ```cpp
  #define WIFI_SSID "Your_WiFi_SSID"
  #define WIFI_PASSWORD "Your_WiFi_Password"
  #define FIREBASE_HOST "your-project-id.firebaseio.com"
  #define FIREBASE_AUTH "your_firebase_database_secret"
  ```
- Upload the code to ESP32  
- Monitor Serial for inputs and log confirmation  

---
