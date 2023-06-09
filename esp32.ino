#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <Ultrasonic.h>
#include <esp_task_wdt.h>


// WiFi-settings
const char *ssid = "IOTLABRA"; // Enter your WiFi name
const char *password = "iotlabra2020";  // Enter WiFi password

// MQTT Broker-settings
const char *mqtt_broker = "172.16.200.53";
const int mqtt_port = 1883;

// Servo-settings
#define SERVO_PIN 27
Servo servoMotor;

// LED pin
int LED = 12;

// DHT11 settings
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// DHT11-timer variables and period-variable for 10 seconds
unsigned long startMillis; 
unsigned long currentMillis;
const unsigned long period = 10000;

// WiFi and MQTT-client settings
WiFiClient espClient;
PubSubClient client(espClient);

// Ultrasonic ranger settings
Ultrasonic ultrasonic(26);
unsigned long startMillisRanger; 
unsigned long currentMillisRanger;
const unsigned long periodRanger = 3000;

// Lightsensor timer settings
unsigned long startMillisLight; 
unsigned long currentMillisLight;
const unsigned long periodLight = 30000;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // Connect to MQTT-broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str())) {
      Serial.println("Mqtt broker connected");
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
  }

  // subscribe to MQTT-topics
  client.subscribe("esp32/led");
  client.subscribe("esp32/servo");
  client.subscribe("esp32/ultrasonic");

  // Initialize LED, DHT11 and Servo
  pinMode(LED, OUTPUT);
  dht.begin();
  servoMotor.attach(SERVO_PIN);

  // Timers
  startMillis = millis();
  startMillisRanger = millis();
  startMillisLight = millis();
}

// callback-function to subscribe to topics
void callback(char *topic, byte *payload, unsigned int length) {

  // Listening esp32/led -topic
  if (strcmp(topic, "esp32/led")==0){
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String led_message;
  
  // Saving topic message to variable
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
      led_message += (char)payload[i];
  }

  // Statements for MQTT-message in esp32/led
    if (led_message == "on"){
      Serial.println("LED on");
      digitalWrite(LED, HIGH);
    }

    if (led_message == "off"){
      Serial.println("LED off");
      digitalWrite(LED, LOW);
    }
  }

  // Listening to esp32/servo -topic
  if (strcmp(topic, "esp32/servo")==0){
    int servopos;

    // Saving message to servopos-variable
    for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
      servopos += (char)payload[i];
    }

  // Moving servo to position from servopos-variable  
  servoMotor.write(servopos);
  }
}

// Function for dht11-sensor
void dht11temp() {

  // Save DHT11-sensordata to variables
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Publish data in JSON-format to "esp32/temp"
  char payload[50];
  sprintf(payload, "{\"temperature\": %.2f, \"humidity\": %.2f}", temperature, humidity);
  client.publish("esp32/temp", payload);
}

// Function for ultrasonicranger
void sonicranger() {
  int RangeInCentimeters = ultrasonic.MeasureInCentimeters();

  // If distance less than 20 cm, publish to MQTT-topic
  if (RangeInCentimeters <= 20) {
    client.publish("esp32/ultrasonic", "alert");
    Serial.println("ALERT!");
  }
}

// Function for light sensor
void lightread() {
  int lightvalue = analogRead(33);
  if (lightvalue <= 100) {
    client.publish("esp32/light", "dark");
  }
  if (lightvalue >= 2000) {
    client.publish("esp32/light", "bright");
  }
  if (lightvalue > 100 && lightvalue < 2000) {
    client.publish("esp32/light", "semilight");
  }
  Serial.println(lightvalue);
}

void loop() {
  // Run MQTT every loop to listen to topics
  client.loop();

  // Using timer to run dht11-function
  currentMillis = millis(); 
  if (currentMillis - startMillis >= period){
    dht11temp();
    startMillis = currentMillis;
  }

  // Using timer to run sonicranger
  currentMillisRanger = millis(); 
  if (currentMillisRanger - startMillisRanger >= periodRanger){
    sonicranger();
    startMillisRanger = currentMillisRanger;
  }

  // Using timer to run lightsensor
  currentMillisLight = millis();
  if (currentMillisLight - startMillisLight >= periodLight){
    lightread();
    startMillisLight = currentMillisLight;
  }
}
