/*
 * OTAWebUpdater.ino Example from ArduinoOTA Library
 * Rui Santos
 * Complete Project Details https://randomnerdtutorials.com
 */
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


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

float V=0;
float A=0;
float W=0;
float Wh=0;
float PF=0;
float F=0;

#define modbusaddr 1

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

Point sensor("SDM ID1");


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


const char* host = "esp8266";
const char* ssid = "TTTA@DOM";
const char* password = "Ttta@2021";

ESP8266WebServer server(80);

/*
 * Login page
 */
const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP8266 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);

    WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      WiFiClient client;
      t_httpUpdate_return ret = ESPhttpUpdate.update(client, "", upload.filename);
      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          break;
      }
    }
  });

  server.begin();
}

void loop(void) {
  
  server.handleClient();
  delay(1);

  SoftwareSerial SerialMod1(D1, D2);
ModbusMaster node1;
SerialMod1.begin(9600);
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


