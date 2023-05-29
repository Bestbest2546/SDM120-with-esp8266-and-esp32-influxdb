#include <InfluxDbClient.h>

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
#include <SoftwareSerial.h>

float V=0;
float A=0;
float W=0;
float Wh=0;
float PF=0;
float F=0;

#define modbusaddr 1
#define modbusaddr 2
#define modbusaddr 3

#define WIFI_SSID "TTTA@DOM"                                                                                                    // WiFi password
#define WIFI_PASSWORD "Ttta@2021"                                                                                               // InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://202.151.182.220:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "Pd9gfPjLZKvUTpUyksUFGCNls7PQ1Gtua0JjgxZI_4lEb3m6JqmnGAbQ_XRSANYIyDaRV_Af5WzuWVC7tqw2lg=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "the tiger team academy"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "ttta"

#define TZ_INFO "<+07>-7"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensor("Read SMD120 3  meters");


float reform_uint16_2_float32(uint16_t u1, uint16_t u2)
{
uint32_t num = ((uint32_t)u1 & 0xFFFF) << 16 | ((uint32_t)u2 & 0xFFFF);
float numf;
memcpy(&numf, &num, 4);
return numf;
}



float getRTU(ModbusMaster& node, uint16_t m_startAddress)
{
uint8_t m_length = 2;
uint16_t result;

node.preTransmission({});
node.postTransmission({});
result = node.readInputRegisters(m_startAddress, m_length);



if (result == node.ku8MBSuccess)
{
return reform_uint16_2_float32(node.getResponseBuffer(0), node.getResponseBuffer(1));
}


return 0;
}

void setup()
{
Serial.begin(115200);
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

//   sensor.addTag("Voltage","Value");
//   sensor.addTag("Current", "value");
//   sensor.addTag("Watt", "value");
//   sensor.addTag("Total Active Power", "value");
//   sensor.addTag("Power Factor", "value");
//   sensor.addTag("Frequency", "value");
   
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

}

void loop()
{
SoftwareSerial SerialMod1(D1, D2);
ModbusMaster node1;
SerialMod1.begin(2400);
node1.begin(1, SerialMod1);


float V1 = getRTU(node1, 0x0000);
float A1 = getRTU(node1, 0x0006);
float W1 = getRTU(node1, 0x000C);
float Wh1 = getRTU(node1, 0x0156);
float PF1 = getRTU(node1, 0x001E);
float F1 = getRTU(node1, 0x0046);
float APP1= getRTU(node1, 0x0012);
float RP1 = getRTU(node1, 0x0018);
float PA1 = getRTU(node1, 0x0024);
float IAE1 = getRTU(node1, 0x0048);
float EAE1 = getRTU(node1, 0x004A);
float IRE1 = getRTU(node1, 0x004C);
float ERE1 = getRTU(node1, 0x004E);
float TRE1 = getRTU(node1, 0x0158);


sensor.clearFields();
  
   sensor.addField("Voltage1",V1);
   sensor.addField("Watt1", W1);
   sensor.addField("Current1",A1);
   sensor.addField("Total Active Energy1",Wh1);
   sensor.addField("Frequency1",F1);
   sensor.addField("Power Factor1",PF1);
   sensor.addField("Apparent Power1",APP1);
   sensor.addField("Reactive Power1",RP1);
   sensor.addField("Phase Angle1",PA1);
   sensor.addField("Import  Active Energy1",IAE1);
   sensor.addField("Export Active Energy1",EAE1);
   sensor.addField("Import  Reactive Energy1",IRE1);
   sensor.addField("Export  Reactive Energy1",ERE1);
   sensor.addField("Total  Reactive Energy1",TRE1);


Serial.println("Device 1:");
Serial.println("Voltage : " + String(V1, 2));
Serial.println("Current : " + String(A1, 2));
Serial.println("Active Power : " + String(W1, 2));
Serial.println("Total Active Energy : " + String(Wh1, 2));
Serial.println("Power Factor : " + String(PF1, 2));
Serial.println("Frequency : " + String(F1, 2));
Serial.println("Apparent Power : " + String(APP1, 2));
Serial.println("Reactive Power : " + String(RP1, 2));
Serial.println("Phase Angle : " + String(PA1, 2));
Serial.println("Import  Active Energy : " + String(IAE1, 2));
Serial.println("Export Active Energy : " + String(EAE1, 2));
Serial.println("Import  Reactive Energy : " + String(IRE1, 2));
Serial.println("Export  Reactive Energy : " + String(ERE1, 2));
Serial.println("Total  Reactive Energy : " + String(TRE1, 2));
Serial.println("===================");
delay(1000);


SoftwareSerial SerialMod2(D3, D4);
ModbusMaster node2;
SerialMod2.begin(2400);
node2.begin(1, SerialMod2);


float V2 = getRTU(node2, 0x0000);
float A2 = getRTU(node2, 0x0006);
float W2 = getRTU(node2, 0x000C);
float Wh2 = getRTU(node2, 0x0156);
float PF2 = getRTU(node2, 0x001E);
float F2 = getRTU(node2, 0x0046);
float APP2= getRTU(node2, 0x0012);
float RP2 = getRTU(node2, 0x0018);
float PA2 = getRTU(node2, 0x0024);
float IAE2 = getRTU(node2, 0x0048);
float EAE2 = getRTU(node2, 0x004A);
float IRE2 = getRTU(node2, 0x004C);
float ERE2 = getRTU(node2, 0x004E);
float TRE2 = getRTU(node2, 0x0158);

  
   sensor.addField("Voltage2",V2);
   sensor.addField("Watt2", W2);
   sensor.addField("Current2",A2);
   sensor.addField("Total Active Energy2",Wh2);
   sensor.addField("Frequency2",F2);
   sensor.addField("Power Factor2",PF2);
   sensor.addField("Apparent Power2",APP2);
   sensor.addField("Reactive Power2",RP2);
   sensor.addField("Phase Angle2",PA2);
   sensor.addField("Import  Active Energy2",IAE2);
   sensor.addField("Export Active Energy2",EAE2);
   sensor.addField("Import  Reactive Energy2",IRE2);
   sensor.addField("Export  Reactive Energy2",ERE2);
   sensor.addField("Total  Reactive Energy2",TRE2);
   

Serial.println("Device 2:");
Serial.println("Voltage : " + String(V2, 2));
Serial.println("Current : " + String(A2, 2));
Serial.println("Active Power : " + String(W2, 2));
Serial.println("Total Active Energy : " + String(Wh2, 2));
Serial.println("Power Factor : " + String(PF2, 2));
Serial.println("Frequency : " + String(F2, 2));
Serial.println("Apparent Power : " + String(APP2, 2));
Serial.println("Reactive Power : " + String(RP2, 2));
Serial.println("Phase Angle : " + String(PA2, 2));
Serial.println("Import  Active Energy : " + String(IAE2, 2));
Serial.println("Export Active Energy : " + String(EAE2, 2));
Serial.println("Import  Reactive Energy : " + String(IRE2, 2));
Serial.println("Export  Reactive Energy : " + String(ERE2, 2));
Serial.println("Total  Reactive Energy : " + String(TRE2, 2));
Serial.println("===================");
delay(1000);

SoftwareSerial SerialMod3(D5, D6);
ModbusMaster node3;
SerialMod3.begin(2400);
node3.begin(1, SerialMod3);

float V3 = getRTU(node3, 0x0000);
float A3 = getRTU(node3, 0x0006);
float W3 = getRTU(node3, 0x000C);
float Wh3 = getRTU(node3, 0x0156);
float PF3 = getRTU(node3, 0x001E);
float F3 = getRTU(node3, 0x0046);
float APP3= getRTU(node3, 0x0012);
float RP3= getRTU(node3, 0x0018);
float PA3 = getRTU(node3, 0x0024);
float IAE3 = getRTU(node3, 0x0048);
float EAE3 = getRTU(node3, 0x004A);
float IRE3 = getRTU(node3, 0x004C);
float ERE3 = getRTU(node3, 0x004E);
float TRE3 = getRTU(node3, 0x0158);

  
   sensor.addField("Voltage3",V3);
   sensor.addField("Watt3", W3);
   sensor.addField("Current3",A3);
   sensor.addField("Total Active Energy3",Wh3);
   sensor.addField("Frequency3",F3);
   sensor.addField("Power Factor3",PF3);
   sensor.addField("Apparent Powe3",APP3);
   sensor.addField("Reactive Power3",RP3);
   sensor.addField("Phase Angle3",PA3);
   sensor.addField("Import  Active Energy3",IAE3);
   sensor.addField("Export Active Energy3",EAE3);
   sensor.addField("Import  Reactive Energy3",IRE3);
   sensor.addField("Export  Reactive Energy3",ERE3);
   sensor.addField("Total  Reactive Energy3",TRE3);

Serial.println("Device 3:");
Serial.println("Voltage : " + String(V3, 2));
Serial.println("Current : " + String(A3, 2));
Serial.println("Active Power : " + String(W3, 2));
Serial.println("Total Active Energy : " + String(Wh3, 2));
Serial.println("Power Factor : " + String(PF3, 2));
Serial.println("Frequency : " + String(F3, 2));
Serial.println("Apparent Power : " + String(APP3, 2));
Serial.println("Reactive Power : " + String(RP3, 2));
Serial.println("Phase Angle : " + String(PA3, 2));
Serial.println("Import  Active Energy : " + String(IAE3, 2));
Serial.println("Export Active Energy : " + String(EAE3, 2));
Serial.println("Import  Reactive Energy : " + String(IRE3, 2));
Serial.println("Export  Reactive Energy : " + String(ERE3, 2));
Serial.println("Total  Reactive Energy : " + String(TRE3, 2));
Serial.println("===================");
delay(1000);

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
