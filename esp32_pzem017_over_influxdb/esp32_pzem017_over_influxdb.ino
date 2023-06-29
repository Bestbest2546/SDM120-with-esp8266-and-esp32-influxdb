  #include <ModbusMasterPzem017.h>
#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;,
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <Arduino.h>  
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "TTTA@DOM";
const char* password = "Ttta@2021";

// WiFi AP SSID
#define WIFI_SSID "TTTA@DOM"
// WiFi password
#define WIFI_PASSWORD "Ttta@2021"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://202.151.182.220:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "Pd9gfPjLZKvUTpUyksUFGCNls7PQ1Gtua0JjgxZI_4lEb3m6JqmnGAbQ_XRSANYIyDaRV_Af5WzuWVC7tqw2lg=="
// InfluxDB v2 organization id (Use:  InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "the tiger team academy"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "ttta"
#define TZ_INFO "<+07>-7"


AsyncWebServer server(80);




InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("Solar");




  static uint8_t pzemSlaveAddr = 0x01; //PZem Address
    static uint16_t NewshuntAddr = 0x0000;      // Declare your external shunt value. Default is 100A, replace to "0x0001" if using 50A shunt, 0x0002 is for 200A, 0x0003 is for 300A
      ModbusMaster node;
        float PZEMVoltage =0;
        float PZEMCurrent =0;
        float PZEMPower =0;
        float PZEMEnergy=0;
   

void setup() 
{
  Serial.begin(115200);

//---------------server over air------------------//
    WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");
  
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
  //--------------------------------------------//
   wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
     Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);   
}
 Serial.println();
  
    Serial2.begin(9600,SERIAL_8N2);
      setShunt(pzemSlaveAddr);
        node.begin(pzemSlaveAddr, Serial2);

 Serial.println();

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
          delay(1000);
}

void loop() {
   
  uint8_t result;
    result = node.readInputRegisters(0x0000, 6);
      if (result == node.ku8MBSuccess) {
        uint32_t tempdouble = 0x00000000;
          PZEMVoltage = node.getResponseBuffer(0x0000) / 100.0;
          PZEMCurrent = node.getResponseBuffer(0x0001) / 100.0;
        tempdouble =  (node.getResponseBuffer(0x0003) << 16) + node.getResponseBuffer(0x0002); // get the power value. Power value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit 
          PZEMPower = tempdouble / 10.0; //Divide the value by 10 to get actual power value (as per manual)
        tempdouble =  (node.getResponseBuffer(0x0005) << 16) + node.getResponseBuffer(0x0004);  //get the energy value. Energy value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit
          PZEMEnergy = tempdouble;

            Serial.print(PZEMVoltage, 1); //Print Voltage value on Serial Monitor with 1 decimal*/
            Serial.print("V   ");
            Serial.print(PZEMCurrent, 3); Serial.print("A   ");
            Serial.print(PZEMPower, 1); Serial.print("W  ");
            Serial.print(PZEMEnergy, 0); Serial.print("Wh  ");
       
              Serial.println();
    } else { Serial.println("Failed to read modbus");}

    sensor.clearFields();

    sensor.addField("Volt", PZEMVoltage);
    sensor.addField("Current", PZEMCurrent);
    sensor.addField("Watt", PZEMPower);
    sensor.addField("Wh", PZEMEnergy);

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
} //Loop Ends

void setShunt(uint8_t slaveAddr) {
  static uint8_t SlaveParameter = 0x06;                                                             /* Write command code to PZEM */
  static uint16_t registerAddress = 0x0003;                                                         /* change shunt register address command code */
  
  uint16_t u16CRC = 0xFFFF;                                                                         /* declare CRC check 16 bits*/
  u16CRC = crc16_update(u16CRC, slaveAddr);                                                         // Calculate the crc16 over the 6bytes to be send
  u16CRC = crc16_update(u16CRC, SlaveParameter);
  u16CRC = crc16_update(u16CRC, highByte(registerAddress));
  u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
  u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
  u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));
      
  Serial.println("Change shunt address");
  Serial2.write(slaveAddr); //these whole process code sequence refer to manual
  Serial2.write(SlaveParameter);
  Serial2.write(highByte(registerAddress));
  Serial2.write(lowByte(registerAddress));
  Serial2.write(highByte(NewshuntAddr));
  Serial2.write(lowByte(NewshuntAddr));
  Serial2.write(lowByte(u16CRC));
  Serial2.write(highByte(u16CRC));
    delay(10); delay(100);
    while (Serial2.available()) {
      Serial.print(char(Serial2.read()), HEX); //Prints the response and display on Serial Monitor (Serial)
      Serial.print(" ");
   }
} //setShunt Ends
