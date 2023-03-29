#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Servo.h>


// WiFi-settings
const char *ssid = "<YOUR_WIFI_SSID>"; // Enter your WiFi name
const char *password = "<YOUR_WIFI_PASSWD>";  // Enter WiFi password

// MQTT Broker-settings
const char *mqtt_broker = "<MQTT_BROKER_IP>";
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

// DHT11-timer variables
unsigned long startMillis; 
unsigned long currentMillis;
const unsigned long period = 10000;

// WiFi and MQTT-client settings
WiFiClient espClient;
PubSubClient client(espClient);

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

  // Initialize LED, DHT11 and Servo
  pinMode(LED, OUTPUT);
  dht.begin();
  servoMotor.attach(SERVO_PIN);

  // Timer
  startMillis = millis();
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

void loop() {

  // Run MQTT every loop to listen to data
  client.loop();

  // Using timer to run dht11-function
  currentMillis = millis(); 
  if (currentMillis - startMillis >= period){
    dht11temp();
    startMillis = currentMillis;
  }
}
