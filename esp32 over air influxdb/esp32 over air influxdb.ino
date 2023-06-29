#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <ModbusMaster.h>
#include <HardwareSerial.h>

ModbusMaster node;
#define modbusaddr 1

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "TTTA@DOM";
const char* password = "Ttta@2021";

AsyncWebServer server(80);




void preTransmission()
{
  // รองรับการใช้ delay ได้ใน ESP32
  delayMicroseconds(200);
}

void postTransmission()
{
  // รองรับการใช้ delay ได้ใน ESP32
  delay(2);
}
// WiFi AP SSID
#define WIFI_SSID "TTTA@DOM"
#define WIFI_PASSWORD "Ttta@2021"
#define INFLUXDB_URL "http://202.151.182.220:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "Pd9gfPjLZKvUTpUyksUFGCNls7PQ1Gtua0JjgxZI_4lEb3m6JqmnGAbQ_XRSANYIyDaRV_Af5WzuWVC7tqw2lg=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "the tiger team academy"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "ttta"
#define TZ_INFO "<+07>-7"


InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("LOAD");

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 15, 4); // ใช้ UART2 แทนการใช้ SoftwareSerial ใน ESP32

   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);   
}
 Serial.println();

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
   node.begin(modbusaddr, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}


float reform_uint16_2_float32(uint16_t u1, uint16_t u2)
{  
  uint32_t num = ((uint32_t)u1 & 0xFFFF) << 16 | ((uint32_t)u2 & 0xFFFF);
  float numf;
  memcpy(&numf, &num, 4);
  return numf;
}

float getRTU(uint16_t m_startAddress){
  uint8_t m_length = 2;
  uint16_t result;
  float x;

  node.clearResponseBuffer();

  result = node.readInputRegisters(m_startAddress, m_length);
  if (result == node.ku8MBSuccess) {
     return reform_uint16_2_float32(node.getResponseBuffer(0), node.getResponseBuffer(1));
  }
}

void loop() {
  float Vol = getRTU(0x0000);
float Wat = getRTU(0x000C);
float Cur = getRTU(0x0006);
float Fre = getRTU(0x0046);
float TotalWh = getRTU(0x0156);
float PF = getRTU(0x001E);

  sensor.clearFields();

sensor.addField("Volt", Vol);
sensor.addField("Watts", Wat);
sensor.addField("Amp", Cur);
sensor.addField("kWh", TotalWh);
sensor.addField("HZ", Fre);
sensor.addField("PF", PF);

Serial.print("Writing: ");
Serial.println(sensor.toLineProtocol());

if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
Serial.println("Wifi connection lost");
}

if (!client.writePoint(sensor)) {
Serial.print("InfluxDB write failed: ");
Serial.println(client.getLastErrorMessage());
}


Serial.println("Wait 5s");
delay(5000);
}


