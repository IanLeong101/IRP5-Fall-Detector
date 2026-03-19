#include <WiFi.h> // Include the WiFi library for ESP32
#include <WiFiClientSecure.h> // Include the WiFiClientSecure library for secure connections
#include <UniversalTelegramBot.h> // Include the UniversalTelegramBot library for Telegram bot functionalities
#include <SPI.h> // Include the SPI library for SPI communication
#include <Wire.h> // Include the Wire library for I2C communication
#include <Adafruit_MPU6050.h> // Include the Adafruit_MPU6050 library for interfacing with the MPU6050 accelerometer and gyroscope
#include <Adafruit_Sensor.h> // Include the Adafruit_Sensor library for sensor handling
#include <Adafruit_GFX.h> // Include the Adafruit_GFX library for graphics functions
#include <Adafruit_SSD1306.h> // Include the Adafruit_SSD1306 library for controlling the SSD1306 OLED display
#include "DHT.h" // Include the DHT library for DHT11 temperature and humidity sensor
#include <delays.h> // Include the Delays library for non-blocking delays
#include <Firebase_ESP_Client.h> // Include the Firebase_ESP_Client library for Firebase integration
#include "addons/TokenHelper.h" // Include additional helper functions for token management
#include "addons/RTDBHelper.h" // Include additional helper functions for Real-Time Database (RTDB) operations

// Able to use WiFi AP or Device tethering using 4G/5G etc.
//Remember to change to your own AP ssid and password

// Wi-Fi credentials
//const char* ssid = "Sour Uncle";
//const char* password = "boostislife123";

// Wireless Tethering Credentials
const char* ssid = "Ian iPhone"; // Wi-Fi network name
const char* password = "ian123456"; // Wi-Fi password

// WiFi credentials
//const char* ssid = "game2"; // Define the WiFi SSID
//const char* password = "2627f68597a"; // Define the WiFi password

FirebaseData fbdo; // Create a FirebaseData object to handle Firebase data
FirebaseAuth auth; // Create a FirebaseAuth object for Firebase authentication
FirebaseConfig config; // Create a FirebaseConfig object for Firebase configuration

bool SignUpOK = false; // Flag to indicate if sign up to Firebase is successful

#define API_KEY "AIzaSyBlUYAkNN1a2hBtP2jkFCdOFxD6ePvSHS0" // Firebase API key
#define DATABASE_URL "https://irprxandian-default-rtdb.asia-southeast1.firebasedatabase.app/" // Firebase database URL

const char *TemperatureTopic = "DHT11/Temp"; // Topic for temperature data in Firebase
const char *HumidityTopic = "DHT11/Humi"; // Topic for humidity data in Firebase
const char *HeartRateTopic = "HeartRate/BPM"; // Topic for heart rate data in Firebase

const char *AXTopic = "Accelerometer/AcX"; // Topic for accelerometer X-axis data in Firebase
const char *AYTopic = "Accelerometer/AcY"; // Topic for accelerometer Y-axis data in Firebase
const char *AZTopic = "Accelerometer/AcZ"; // Topic for accelerometer Z-axis data in Firebase

const char *GXTopic = "Accelerometer/GyX"; // Topic for gyroscope X-axis data in Firebase
const char *GYTopic = "Accelerometer/GyY"; // Topic for gyroscope Y-axis data in Firebase
const char *GZTopic = "Accelerometer/GyZ"; // Topic for gyroscope Z-axis data in Firebase

const char *FallTopic = "Accelerometer/Fall"; // Topic for fall detection status in Firebase

void dht_read(); // Function prototype for reading DHT sensor data
void pulse_read(); // Function prototype for reading pulse sensor data
void mpu_read(); // Function prototype for reading MPU6050 sensor data
void display_fall_warning(); // Function prototype for displaying fall warning on OLED display
void display_lcd(); // Function prototype for displaying sensor readings on OLED display

// Function to establish connection to Firebase
void connectToFirebase() {
  config.api_key = API_KEY; // Set the Firebase API key
  config.database_url = DATABASE_URL; // Set the Firebase database URL

  // Attempt to sign up to Firebase using specified email and password
  // Change to another email before running everytime
  if (Firebase.signUp(&config, &auth, "SENLOUDPIARXWIN@outlook.com", "MkDpI9m22")) {
    Serial.println("Sign Up OK"); // Print sign up success message to serial monitor
    SignUpOK = true; // Set sign up flag to true
  } else {
    Serial.printf("Sign Up failed: %s\n", config.signer.signupError.message.c_str()); // Print sign up failure message to serial monitor
  }

  config.token_status_callback = tokenStatusCallback; // Set token status callback function
  Firebase.begin(&config, &auth); // Begin Firebase with specified configuration and authentication
  Firebase.reconnectWiFi(true); // Reconnect WiFi for Firebase
}

// Function to establish connection to WiFi
void connectToWiFi() {
  Serial.println("\nConnecting to WiFi..."); // Print connection status message to serial monitor
  WiFi.begin(ssid, password); // Connect to WiFi with specified SSID and password

  // Wait until WiFi connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); // Delay 1 second
    Serial.print("."); // Print '.' to indicate connection progress
  }

  // Print connection success message to serial monitor
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print local IP address
}

// Function to publish data to Firebase
void PublishToFirebase(const char* topic, int value) {
  // Check if Firebase is ready and sign up is successful
  if (Firebase.ready() && SignUpOK) {
    // Attempt to set integer value to specified topic in Firebase Real-Time Database
    if (Firebase.RTDB.setInt(&fbdo, topic, value)) {
      Serial.println(); // Print empty line
      Serial.print(value); // Print value to serial monitor
      Serial.print(" - Successfully saved to " + fbdo.dataPath()); // Print success message to serial monitor
      Serial.println(" (" + fbdo.dataType() + ") "); // Print data type to serial monitor
    } else {
      Serial.println("FAILED: " + fbdo.errorReason()); // Print failure message to serial monitor
    }
  }
}

// Telegram Bot credentials (User has to have unique CHAT_ID to acquire messages from the Bot. Generate one here: https://t.me/username_to_id_bot)
#define BOTtoken "6705964715:AAEXwPYTOdEJ2Lv97poQAtkTXxh8LP7QFeU" // Telegram Bot token
#define CHAT_ID "644471195" // Telegram Chat ID

WiFiClient wifiClient; // Create a WiFiClient object
WiFiClientSecure client; // Create a secure WiFiClient object
UniversalTelegramBot bot(BOTtoken, client); // Create a UniversalTelegramBot object with specified token and client

#define MPU6050_ADDR 0x68 // MPU6050 I2C address
#define CUSTOM_SDA 18 // Define custom SDA pin for I2C communication
#define CUSTOM_SCL 19 // Define custom SCL pin for I2C communication

#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define OLED_RESET -1 // OLED reset pin (not used)
#define SCREEN_ADDRESS 0x3C // OLED display address

Adafruit_MPU6050 mpu; // Create an Adafruit_MPU6050 object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Create an Adafruit_SSD1306 object for OLED display

#define DHT11PIN 10 // DHT11 sensor pin
DHT dht(DHT11PIN, DHT11); // Create a DHT object for DHT11 sensor

DELAYMS_T d2; // Create a DELAYMS_T object for non-blocking delays

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Variables to store sensor data
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0; // Variables to store accelerometer and gyroscope data

float humi = 0; // Variable to store humidity data
float temp = 0; // Variable to store temperature data

int fall = 0; // Variable to indicate fall detection status

unsigned int sensorValue = 0; // Variable to store sensor value
float BPM = 0.0; // Variable to store heart rate (beats per minute)
float newBPM = 0.0; // Variable to store updated heart rate
bool IgnoreReading = false; // Flag to ignore sensor reading
bool FirstPulseDetected = false; // Flag to indicate first pulse detection
unsigned long PulseTime = 0; // Variable to store pulse time
unsigned long timepassed = 0; // Variable to store time passed since last pulse

const int BUZZER_PIN = 2; // Buzzer pin
const int BUTTON_PIN = 6; // Button pin
const int REDLED_PIN = 3; // Red LED pin
const int GREENLED_PIN = 5; // Green LED pin
const int PULSESENSOR_PIN = 0; // Pulse sensor pin

void setup() {
  Serial.begin(115200); // Initialize serial communication
  delay(1000); // Delay 1 second to allow opening of serial monitor

  d2.dlySet(2000); // Set non-blocking delay to 2000 milliseconds

  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
  pinMode(REDLED_PIN, OUTPUT); // Set red LED pin as output
  pinMode(GREENLED_PIN, OUTPUT); // Set green LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up resistor
  pinMode(PULSESENSOR_PIN, INPUT); // Set pulse sensor pin as input

  connectToWiFi(); // Connect to WiFi network
  connectToFirebase(); // Connect to Firebase

  WiFi.mode(WIFI_STA); // Set WiFi mode to station (client)
  WiFi.begin(ssid, password); // Begin WiFi connection
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Set root certificate for secure connection to Telegram
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Delay 500 milliseconds
    Serial.print("."); // Print '.' to indicate connection progress
  }
  Serial.println(""); // Print empty line
  Serial.println("WiFi connected"); // Print WiFi connection success message to serial monitor

  Wire.begin(CUSTOM_SDA, CUSTOM_SCL); // Begin I2C communication with specified SDA and SCL pins

  if (!mpu.begin()) { // Check if MPU6050 chip is found
    Serial.println("Failed to find MPU6050 chip"); // Print failure message to serial monitor
    while (1) {
      delay(10); // Delay 10 milliseconds
    }
  }
  Serial.println("MPU6050 Found!"); // Print success message to serial monitor

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Check if SSD1306 OLED display is initialized
    Serial.println(F("SSD1306 allocation failed")); // Print failure message to serial monitor
    for (;;);
  }
  display.clearDisplay(); // Clear OLED display

  dht.begin(); // Begin DHT sensor communication

  pinMode(0, INPUT); // Set pin 0 as input
  timepassed = millis(); // Initialize timepassed variable with current milliseconds
}

void loop() {
  dht_read(); // Read DHT sensor data
  pulse_read(); // Read pulse sensor data
  mpu_read(); // Read MPU6050 sensor data

  ax = (AcX - 2050) / 16384.00; // Calculate normalized accelerometer X-axis value
  ay = (AcY - 77) / 16384.00; // Calculate normalized accelerometer Y-axis value
  az = (AcZ - 1947) / 16384.00; // Calculate normalized accelerometer Z-axis value
  float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5); // Calculate raw amplitude
  int Amp = Raw_Amp * 10; // Calculate amplitude

  // Check for fall detection
  if (Amp >= 20) {
    tone(BUZZER_PIN, 1000); // Generate a tone on the buzzer
    Serial.println("Fall Detected"); // Print fall detection message to serial monitor
    digitalWrite(REDLED_PIN, HIGH); // Turn on red LED
    digitalWrite(GREENLED_PIN, LOW); // Turn off green LED

    fall = 1; // Set fall detection status to true
    PublishToFirebase(FallTopic, fall); // Publish fall detection status to Firebase

    bot.sendMessage(CHAT_ID, "Fall Detected!"); // Send fall detection message to Telegram

    display_fall_warning(); // Display fall warning on OLED display

    // Wait for the button press to acknowledge false alarm
    while (digitalRead(BUTTON_PIN) == HIGH) {
    }

    noTone(BUZZER_PIN); // Stop the tone when the button is pressed

    fall = 0; // Set fall detection status to false
    PublishToFirebase(FallTopic, fall); // Publish fall detection status to Firebase

    Serial.println("Button pressed, false alarm"); // Print false alarm acknowledgment message to serial monitor
    bot.sendMessage(CHAT_ID, "Button pressed, false alarm"); // Send false alarm acknowledgment message to Telegram
    digitalWrite(GREENLED_PIN, HIGH); // Turn on green LED
    digitalWrite(REDLED_PIN, LOW); // Turn off red LED
  }

  display_lcd(); // Update OLED display with sensor readings
}

// Function to read MPU6050 sensor data
void mpu_read() {
  Wire.beginTransmission(MPU6050_ADDR); // Begin transmission to MPU6050
  Wire.write(0x3B); // Send start register address for accelerometer data
  Wire.endTransmission(false); // End transmission without releasing the bus
  Wire.requestFrom(MPU6050_ADDR, 14, true); // Request 14 bytes of data from MPU6050
  AcX = Wire.read() << 8 | Wire.read(); // Read accelerometer X-axis data
  AcY = Wire.read() << 8 | Wire.read(); // Read accelerometer Y-axis data
  AcZ = Wire.read() << 8 | Wire.read(); // Read accelerometer Z-axis data
  Tmp = Wire.read() << 8 | Wire.read(); // Read temperature data
  GyX = Wire.read() << 8 | Wire.read(); // Read gyroscope X-axis data
  GyY = Wire.read() << 8 | Wire.read(); // Read gyroscope Y-axis data
  GyZ = Wire.read() << 8 | Wire.read(); // Read gyroscope Z-axis data
  PublishToFirebase(GXTopic, GyX); // Publish gyroscope X-axis data to Firebase
  PublishToFirebase(GYTopic, GyY); // Publish gyroscope Y-axis data to Firebase
  PublishToFirebase(GZTopic, GyZ); // Publish gyroscope Z-axis data to Firebase
  PublishToFirebase(AXTopic, AcX); // Publish accelerometer X-axis data to Firebase
  PublishToFirebase(AYTopic, AcY); // Publish accelerometer Y-axis data to Firebase
  PublishToFirebase(AZTopic, AcZ); // Publish accelerometer Z-axis data to Firebase
}

// Function to read DHT11 sensor data
void dht_read() {
  humi = dht.readHumidity(); // Read humidity data from DHT sensor
  temp = dht.readTemperature(); // Read temperature data from DHT sensor
  if (d2.dlyExpiredRestart()) { // Check if non-blocking delay has expired
    Serial.print("Temperature: "); // Print temperature label to serial monitor
    Serial.print(temp); // Print temperature value to serial monitor
    Serial.print("ºC "); // Print temperature unit to serial monitor
    Serial.print("Humidity: "); // Print humidity label to serial monitor
    Serial.println(humi); // Print humidity value to serial monitor
  }
  PublishToFirebase(TemperatureTopic, temp); // Publish temperature data to Firebase
  PublishToFirebase(HumidityTopic, humi); // Publish humidity data to Firebase
}

// Function to read pulse sensor data
void pulse_read() {
  sensorValue = analogRead(PULSESENSOR_PIN); // Read sensor value from pulse sensor

  if (sensorValue > 3600 && !IgnoreReading) {
    PulseTime = millis() - timepassed; // Calculate pulse time
    timepassed = millis(); // Update timepassed
    IgnoreReading = true; // Set ignore reading flag to true
  }

  if (sensorValue < 3600) {
    IgnoreReading = false; // Set ignore reading flag to false
  }

  // Calculate BPM if a valid pulse is detected
  if (PulseTime > 0) {
    BPM = (1.0 / (PulseTime / 1000.0)) * 60.0; // Calculate beats per minute (BPM)

    // Reset PulseTime to 0 to avoid repeated calculations
    PulseTime = 0;

    // Check if BPM is within valid range
    if (BPM >= 40 && BPM <= 150) {
      newBPM = BPM; // Update BPM value
      Serial.print(sensorValue); // Print sensor value to serial monitor
      Serial.print("\t"); // Print tab character to serial monitor
      Serial.print(newBPM); // Print BPM value to serial monitor
      Serial.println(" BPM"); // Print BPM unit to serial monitor
    }
  }

  PublishToFirebase(HeartRateTopic, newBPM); // Publish heart rate data to Firebase
}

// Function to display sensor readings on OLED display
void display_lcd() {
  // Clear the display buffer
  display.clearDisplay();

  // Display text
  display.setTextSize(1); // Set text size to 1
  display.setTextColor(WHITE); // Set text color to white
  display.setCursor(0, 0); // Set cursor position
  display.print("Sensor Readings"); // Print sensor readings label
  display.println(); // Print empty line
  display.println(); // Print empty line
  display.print("BPM: "); // Print BPM label
  display.println(newBPM); // Print BPM value
  display.println(); // Print empty line
  display.print("Temperature: "); // Print temperature label
  display.print(temp); // Print temperature value
  display.print((char)247); // Print degree symbol
  display.println("C"); // Print temperature unit
  display.println(); // Print empty line
  display.print("Humidity: "); // Print humidity label
  display.print(humi); // Print humidity value
  display.println("%"); // Print humidity unit
  display.println(); // Print empty line

  display.display(); // Update OLED display
}

// Function to display fall warning on OLED display
void display_fall_warning() {
  // Clear the display buffer
  display.clearDisplay();

  // Display warning message
  display.setTextSize(3); // Set text size to 3
  display.setTextColor(WHITE); // Set text color to white
  display.setCursor(0, 0); // Set cursor position
  display.println("WARNING"); // Print warning message
  display.setTextSize(2); // Set text size to 2
  display.setCursor(41, 25); // Set cursor position
  display.println("FALL"); // Print fall message
  display.setCursor(18, 45); // Set cursor position
  display.print("DETECTED"); // Print detected message
  display.display(); // Update OLED display
}
