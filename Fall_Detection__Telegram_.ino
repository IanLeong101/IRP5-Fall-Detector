#include <WiFi.h> // Including WiFi library
#include <WiFiClientSecure.h> // Including secure WiFi client library
#include <UniversalTelegramBot.h> // Including Universal Telegram Bot library
#include <SPI.h> // Including SPI library
#include <Wire.h> // Including Wire library
#include <Adafruit_MPU6050.h> // Including Adafruit MPU6050 library
#include <Adafruit_Sensor.h> // Including Adafruit Sensor library
#include <Adafruit_GFX.h> // Including Adafruit Graphics library
#include <Adafruit_SSD1306.h>  // Including Adafruit SSD1306 library
#include "DHT.h" // Including DHT library
#include <delays.h> // Including delays library

// Able to use WiFi AP or Device tethering using 4G/5G etc.
//Remember to change to your own AP ssid and password

// Wi-Fi credentials
//const char* ssid = "Sour Uncle";
//const char* password = "boostislife123";

// Wireless Tethering Credentials
const char* ssid = "Ian iPhone"; // Wi-Fi network name
const char* password = "ian123456"; // Wi-Fi password

// Telegram Bot credentials (User has to have unique CHAT_ID to acquire messages from the Bot. Generate one here: https://t.me/username_to_id_bot)
#define BOTtoken "6705964715:AAEXwPYTOdEJ2Lv97poQAtkTXxh8LP7QFeU" // Telegram Bot token
#define CHAT_ID "644471195" // Change to your own CHAT_ID before using

WiFiClientSecure client; // Creating a secure WiFi client
UniversalTelegramBot bot(BOTtoken, client); // Creating a Telegram Bot instance

// MPU6050 parameters
#define MPU6050_ADDR 0x68 // MPU6050 I2C address
#define CUSTOM_SDA 18 // Custom SDA pin for I2C
#define CUSTOM_SCL 19 // Custom SCL pin for I2C

// OLED display parameters
#define SCREEN_WIDTH 128 // Screen width in pixels
#define SCREEN_HEIGHT 64 // Screen height in pixels
#define OLED_RESET -1 // Reset pin for OLED display
#define SCREEN_ADDRESS 0x3C // I2C address of the OLED display

Adafruit_MPU6050 mpu; // Creating MPU6050 instance

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating SSD1306 OLED display instance

// DHT sensor parameters
#define DHT11PIN 10 // Pin number for DHT11 sensor
DHT dht(DHT11PIN, DHT11); // Creating DHT sensor instance

// Delay timers
DELAYMS_T d1; // Delay timer 1
DELAYMS_T d2; // Delay timer 2

// Sensor data variables
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Raw sensor data for MPU6050
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0; // Formatted sensor data for MPU6050
float humi = 0; // Humidity
float temp = 0; // Temperature
int sensorValue = 0; // Raw sensor value for Pulse Sensor
int threshold = 3545; // Threshold for pulse detection
int BPM = 0; // Calculated heart rate
unsigned long lastBeatTime = 0; // Time of last detected beat
unsigned long interval = 0; // Time interval between beats

// Pin definitions
const int BUZZER_PIN = 2; // Pin for buzzer
const int BUTTON_PIN = 6; // Pin for button
const int REDLED_PIN = 3; // Pin for red LED
const int GREENLED_PIN = 5; // Pin for green LED
const int PULSESENSOR_PIN = 0; // Pin for Pulse Sensor

void setup() {
  Serial.begin(115200); // Starting serial communication
  delay(1000); // Delay for serial monitor to open

  d1.dlySet(500); // Setting non-blocking 500ms delay for d1
  d2.dlySet(2000); // Setting non-blocking 2000ms delay for d2

  pinMode(BUZZER_PIN, OUTPUT); // Setting buzzer pin as output
  pinMode(REDLED_PIN, OUTPUT); // Setting red LED pin as output
  pinMode(GREENLED_PIN, OUTPUT); // Setting green LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Setting button pin as input with internal pull-up resistor
  pinMode(PULSESENSOR_PIN, INPUT); // Setting Pulse Sensor pin as input

  // Connecting to Wi-Fi
  WiFi.mode(WIFI_STA); // Setting Wi-Fi mode to station mode
  WiFi.begin(ssid, password); // Attempting to connect to Wi-Fi network
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Setting root certificate for secure communication with Telegram
  while (WiFi.status() != WL_CONNECTED) { // Waiting for connection to be established
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Wire.begin(CUSTOM_SDA, CUSTOM_SCL); // Starting I2C communication with custom SDA and SCL pins

  // Initializing MPU6050
  if (!mpu.begin()) { // Checking if MPU6050 initialization failed
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Initializing OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Checking if SSD1306 OLED display initialization failed
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay(); // Clearing display

  // Initializing DHT sensor
  dht.begin(); // Starting DHT sensor
}

void loop() {
  // Reading sensor data
  dht_read(); // Reading data from DHT sensor
  pulse_read(); // Reading data from Pulse Sensor
  mpu_read(); // Reading data from MPU6050

  // Calculating fall amplitude
  ax = (AcX - 2050) / 16384.00; // Calculating x-axis acceleration
  ay = (AcY - 77) / 16384.00; // Calculating y-axis acceleration
  az = (AcZ - 1947) / 16384.00; // Calculating z-axis acceleration
  float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5); // Calculating raw amplitude
  int Amp = Raw_Amp * 10; // Converting amplitude to integer

  digitalWrite(GREENLED_PIN, LOW); // Turning off green LED

  // Checking for fall detection
  if (Amp >= 20) { // If amplitude is above threshold
    if (d1.dlyExpiredRestart()) { // If delay timer d1 has expired
      // Generating a tone on the buzzer
      tone(BUZZER_PIN, 1000); // Generating tone at 1000 Hz
      Serial.println("Fall Detected");
      digitalWrite(REDLED_PIN, HIGH); // Turning on red LED
      digitalWrite(GREENLED_PIN, LOW); // Turning off green LED
    }

    bot.sendMessage(CHAT_ID, "Fall Detected!"); // Sending Telegram message
    // Displaying fall warning on OLED display
    display_fall_warning();

    // Waiting for the button press
    while (digitalRead(BUTTON_PIN) == HIGH) {
      // Waiting for the button press
    }

    // Stopping the tone when the button is pressed
    noTone(BUZZER_PIN);
    Serial.println("Button pressed, false alarm");
    bot.sendMessage(CHAT_ID, "Button pressed, false alarm");
    digitalWrite(GREENLED_PIN, HIGH); // Turning on green LED
    digitalWrite(REDLED_PIN, LOW); // Turning off red LED
  }

  // Updating display
  display_lcd();
}

void mpu_read() {
  Wire.beginTransmission(MPU6050_ADDR); // Starting I2C transmission with MPU6050
  Wire.write(0x3B); // Writing to register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false); // Ending transmission without releasing bus
  Wire.requestFrom(MPU6050_ADDR, 14, true); // Requesting 14 registers from MPU6050
  AcX = Wire.read() << 8 | Wire.read(); // Reading x-axis acceleration
  AcY = Wire.read() << 8 | Wire.read(); // Reading y-axis acceleration
  AcZ = Wire.read() << 8 | Wire.read(); // Reading z-axis acceleration
  Tmp = Wire.read() << 8 | Wire.read(); // Reading temperature
  GyX = Wire.read() << 8 | Wire.read(); // Reading x-axis gyroscope data
  GyY = Wire.read() << 8 | Wire.read(); // Reading y-axis gyroscope data
  GyZ = Wire.read() << 8 | Wire.read(); // Reading z-axis gyroscope data
}

void dht_read() {
  humi = dht.readHumidity(); // Reading humidity from DHT sensor
  temp = dht.readTemperature(); // Reading temperature from DHT sensor
  if (d2.dlyExpiredRestart()) { // If delay timer d2 has expired
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("ºC ");
    Serial.print("Humidity: ");
    Serial.println(humi);
  }
}

void pulse_read() {
  sensorValue = analogRead(PULSESENSOR_PIN); // Reading sensor value from Pulse Sensor

  // Detecting a beat
  if (sensorValue > threshold) { // If sensor value is above threshold
    interval = millis() - lastBeatTime; // Calculating time interval since last beat
    lastBeatTime = millis(); // Updating last beat time

    // Calculating heart rate
    if (interval > 250) { // If time interval is greater than 250ms (ignoring noise)
      BPM = 60000 / interval; // Calculating BPM
      Serial.print("BPM = ");
      Serial.println(BPM);
    }
  }
}

void display_lcd() {
  // Clearing the display buffer
  display.clearDisplay();

  // Displaying text on OLED display
  display.setTextSize(1); // Setting text size
  display.setTextColor(WHITE); // Setting text color
  display.setCursor(0, 0); // Setting cursor position

  display.print("Sensor Readings"); // Displaying sensor readings
  display.println(); // Moving to next line
  display.println(); // Moving to next line

  display.print("BPM: "); // Displaying heart rate
  display.println(BPM); // Displaying heart rate
  display.println(); // Moving to next line

  display.print("Temperature: "); // Displaying temperature
  display.print(temp); // Displaying temperature
  display.print((char)247); // Displaying degree symbol
  display.println("C"); // Displaying temperature unit
  display.println(); // Moving to next line

  display.print("Humidity: "); // Displaying humidity
  display.print(humi); // Displaying humidity
  display.println("%"); // Displaying humidity unit

  display.println(); // Moving to next line

  display.display(); // Displaying content on OLED display
}

void display_fall_warning() {
  // Clearing the display buffer
  display.clearDisplay();

  // Displaying fall warning on OLED display
  display.setTextSize(3); // Setting text size
  display.setTextColor(WHITE); // Setting text color
  display.setCursor(0, 0); // Setting cursor position
  display.println("WARNING"); // Displaying warning
  display.setTextSize(2); // Setting text size
  display.setCursor(41, 25); // Setting cursor position
  display.println("FALL"); // Displaying fall
  display.setCursor(18, 45); // Setting cursor position
  display.print("DETECTED"); // Displaying detected
  display.display(); // Displaying content on OLED display
}
