#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_PWMServoDriver.h>
#include <HTTPClient.h>

const char *ssid = "WIFI_NAME";
const char *password = "WIFI_PASSWORD";
const int PWW_FREQ = 50;
const int SERVO_MIN = 60;
const int SERVO_MAX = 520;
const int BUTTON_ONE_PIN = 14; // Button pin
const float LED_PIN = 15;      // LED pin
const float BUTTON_VELOCITY_MAX = 5;
const float BUTTON_VELOCITY_MIN = 0.1;
const char *SERVER_URL = "http://192.168.0.8:6666";

const unsigned long DEBOUNCE_DELAY = 250;
const unsigned long STATE3_DURATION = 5000; // 5 seconds
// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;
unsigned long state3_start_time = 0;
int state = 0; // State variable
               // State 0 all motors running
               // State 1 motor 1 stopped
               // State 2 motor 1-2 stopped
               // State 3 motor 1-3 stopped

float angleMotors[] = {0, 0, 0};
float velocityMotors[] = {1, 2, 3};
HTTPClient http;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void connectToWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void setupPWMServoDriver()
{
  pwm.begin();
  pwm.setPWMFreq(PWW_FREQ); // Set the PWM frequency (you may need to adjust this)
  Serial.print("PWM Servo Driver initialized with Frequency: ");
  Serial.println(PWW_FREQ);
}

void sendPostRequest(float angleMotors[], size_t size)
{
  // Prepare the angleMotors array data as JSON
  String postData = "[";
  for (int i = 0; i < size; i++)
  {
    if (i > 0)
      postData += ",";
    postData += String(angleMotors[i]);
  }
  postData += "]";

  // Send a POST request
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(postData);
  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.println(response);
  }
  else
  {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Hardware Interrupts
volatile bool buttonPressed = false;

void IRAM_ATTR buttonISR()
{
  button_time = millis();
  if (button_time - last_button_time > DEBOUNCE_DELAY)
  {
    buttonPressed = true;
    last_button_time = button_time;
  }
}

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  setupPWMServoDriver();
  pinMode(BUTTON_ONE_PIN, INPUT_PULLUP); // Set button pin as INPUT_PULLUP
  attachInterrupt(digitalPinToInterrupt(BUTTON_ONE_PIN), buttonISR, FALLING);
  pinMode(LED_PIN, OUTPUT); // Set LED pin as OUTPUT
}

void loop()
{
  if (state == 3 && millis() - state3_start_time >= STATE3_DURATION)
  {
    state = 0;
    digitalWrite(LED_PIN, LOW); // Turn on LED when state is 3
    Serial.println("State reset to 0");
  }
  if (buttonPressed)
  {
    // Reset servo position to 0 degrees
    if (state < 3)
    {
      state++;
      Serial.print("State changed to: ");
      Serial.println(state);

      if (state == 3)
      {
        state3_start_time = millis();
        digitalWrite(LED_PIN, HIGH); // Turn on LED when state is 3
        sendPostRequest(angleMotors, sizeof(angleMotors) / sizeof(angleMotors[0]));
      }
    }
    buttonPressed = false;
  }
  else
  {
    // Move servo from 0 to 180 degrees
    for (int i = 0; i < sizeof(angleMotors) / sizeof(angleMotors[0]); i++)
    {
      if (i < state)
      {
        continue;
      }
      if (angleMotors[i] > 180 || angleMotors[i] < 0)
      {

        velocityMotors[i] = velocityMotors[i] * -1;
      }

      angleMotors[i] = angleMotors[i] + velocityMotors[i];

      pwm.setPWM(i, 0, map(round(angleMotors[i]), 0, 180, SERVO_MIN, SERVO_MAX));
    }
  }

  delay(10);
}
