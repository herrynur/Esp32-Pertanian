#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define adc_hum 33
#define adc_ph 36
#define relay 27
#define led 2
#define ledwifi 4

int adc_hum_value;
int adc_ph_value;
int percent_hum;
float ph;

unsigned long previousMillis = 0;
const long interval = 1000;

unsigned long awalMilis = 0;
const long intervalMilis = 10000;
int status = LOW;
bool dataread = false;

void samplingOn(int timee);
void samplingOff(int timee);

// hum
// 2500 kering
// 0 basah

// Setting pass dan ssid wifi disini
const char *ssid = "Asus_X01BDA";
const char *password = "heri1234567";

// mqtt
const char *mqtt_server = "broker.hivemq.com";
#define MQTT_USERNAME ""
#define MQTT_KEY ""
// Setting id disini, id disini harus sama dengan yang di Android
String topic = "smartplant/plant1";
String dataKirim = "-,-,-";

// wifi client
WiFiClient espClient;
PubSubClient client(espClient);
void setup_wifi();
void reconnect();

// connectt wifi
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(ledwifi, HIGH);
    delay(500);
    digitalWrite(ledwifi, LOW);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(ledwifi, HIGH);
}

// Reconnect saat putus
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(adc_hum, INPUT);
  pinMode(adc_ph, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(ledwifi, OUTPUT);
  Serial.begin(115200);
}

void loop()
{
  // adc_hum_value = analogRead(adc_hum);
  // //adc_ph_value = analogRead(adc_ph);
  // percent_hum = constrain(map(adc_hum_value, 800, 2500, 100, 0), 0, 100);

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  samplingOn(1000);
  samplingOff(5000);
}

void samplingOn(int _time)
{
  bool isEnd = false;
  unsigned long timeStart = millis();
  while (!isEnd)
  {
    digitalWrite(relay, HIGH);
    digitalWrite(led, HIGH);
    adc_hum_value = analogRead(adc_hum);
    adc_ph_value = analogRead(adc_ph);
    percent_hum = constrain(map(adc_hum_value, 800, 2500, 100, 0), 0, 100);
    ph = map(adc_ph_value, 0, 270, 70, 40);
    if (ph < 0)
      ph = 70;
    Serial.print("Humidity ADC: ");
    Serial.print(adc_hum_value);
    Serial.print("  | Humidity: ");
    Serial.print(percent_hum);
    Serial.print(" % ||   Ph adc: ");
    Serial.print(adc_ph_value);
    Serial.print(" ||   Ph: ");
    Serial.println(ph/10);
    //
    dataKirim = "26," + String(percent_hum) + "," + String(ph/10);
    if ((unsigned long)millis() - timeStart > _time)
    {
      Serial.print("data Kirim : ");
      Serial.println(dataKirim);
      client.publish((char *)topic.c_str(), (char *)dataKirim.c_str());
      isEnd = true;
    }
  }
}

void samplingOff(int _time)
{
  bool isEnd = false;
  unsigned long timeStart = millis();
  Serial.flush();
  while (!isEnd)
  {
    digitalWrite(relay, LOW);
    digitalWrite(led, LOW);
    if ((unsigned long)millis() - timeStart > _time)
      isEnd = true;
  }
}