  
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <Arduino.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <OneButton.h>



uint8_t LED = D4;
///////////////////////////////////////////
// This Block will be set with WiFiManager this are default values
IPAddress ip(192,168,178,19);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);
// This Block will also be setup with WifiManager but needs to be stored by us
// default values for other secrets
uint16_t domophoneHttpsPort=4711;
char domophoneIP[15+1]="1.1.1.1";
char APIKEY[19+1] = "FFFF-FFFF-FFFF-FFFF";
const char configFile[] = "/config.txt";  // <- SD library uses 8.3 filenames


//flag for saving data
bool shouldSaveConfig = false;
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String path = "/webapi.cgi";
String clientID = "439A557E991F490D";
String clientName="WebClientSample";
int clientSize=15;
const String PARAMS_OPEN_GATEWAY = "4";
const String PARAMS_OPEN_GATE = "5";
//PARAMS_OPEN_GATEWAY="apikey=$APIKEY&clientId=$ClientID&clientName=$ClientName&clientNameSize=15&action=1&param1=4"
//PARAMS_OPEN_GATE="apikey=$APIKEY&clientId=$ClientID&clientName=$ClientName&clientNameSize=15&action=1&param1=5"


void open(const String gwtype);
void wirelessconfig(const bool reset);
void click1();
void click2();
void longPressStart1();
void duringLongPress1();
void longPressStart2();
void duringLongPress2();
void longPressStop();
boolean flag = false;

void loadConfiguration(const char *filename);
void saveConfiguration(const char *filename);

// Setup a new OneButton on pin A1.  
OneButton button1(D1, false, false);
// Setup a new OneButton on pin A2.  
OneButton button2(D2, false, false);

unsigned long StartTimeButton1 = 0;
unsigned long ElapsedTimeButton1 = 0;
unsigned long StartTimeButton2 = 0;
unsigned long ElapsedTimeButton2 = 0;
// both buttons need to be pressed for 5 SECONDS
#define LONG_PRESS_MS 5000

void setup() 
{
  // Serial Debug
  Serial.begin(115200);
  // Debugging only
  pinMode(LED, OUTPUT);
  // connected to DONE TPL5110 pin via pull-down 
  pinMode(D3,OUTPUT);
  // keep the DONE of TPL5110 low until we finished
  digitalWrite(D3,LOW);
  button1.attachClick(click1);
  button1.attachLongPressStart(longPressStart1);
  // we need only one long press stop 
  button1.attachLongPressStop(longPressStop);
 
  button2.attachClick(click2);
  button2.attachLongPressStart(longPressStart2);
   // we need only one long press stop 
  button2.attachLongPressStop(longPressStop);
  
}


//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() 
{
  button1.tick();
  button2.tick();
  // You can implement other code in here or just wait a while 
  delay(10);  
}
//=======================================================================


void wirelessconfig(const bool reset)
{
  WiFiManager wifiManager;
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setSTAStaticIPConfig(ip, gateway, subnet);
  if(reset)
  {
    wifiManager.resetSettings();
    Serial.println("DELETE_CONFIG");
  }
  char port[6] = {0};
  itoa(domophoneHttpsPort,port,10);
  WiFiManagerParameter custom_APIKEY("APIKEY", "API-KEY", APIKEY, sizeof(APIKEY));
  WiFiManagerParameter custom_domophoneHttpsPort("domophoneHttpsPort", "Port", port, 6);
  WiFiManagerParameter custom_domophoneIP("domophoneIP", "Domo. IP", domophoneIP, sizeof(domophoneIP));
  wifiManager.addParameter(&custom_APIKEY);
  wifiManager.addParameter(&custom_domophoneHttpsPort);
  wifiManager.addParameter(&custom_domophoneIP);
  
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    
  }
  if(shouldSaveConfig)
  {
    Serial.println("safe config");
    strlcpy(APIKEY, custom_APIKEY.getValue(),sizeof(APIKEY));
    strlcpy(domophoneIP, custom_domophoneIP.getValue(),sizeof(domophoneIP));
    domophoneHttpsPort = atoi(custom_domophoneHttpsPort.getValue());
    Serial.println(APIKEY);
    Serial.println(domophoneHttpsPort);
    Serial.println(domophoneIP);
    saveConfiguration(configFile);
    shouldSaveConfig = false;
    // TPL5110 Terminate Power
    // safe config should trigger terminate power
    digitalWrite(D3,HIGH);
  } 
}

void click1() {
  loadConfiguration(configFile);
  wirelessconfig(false);
  open(PARAMS_OPEN_GATEWAY);
  digitalWrite(D3,HIGH);
  // TPL5110 Terminate Power
  digitalWrite(D3,HIGH);
} // click1

void click2() {
  loadConfiguration(configFile);
  wirelessconfig(false);
  open(PARAMS_OPEN_GATE);
  digitalWrite(D3,HIGH);
  // TPL5110 Terminate Power
  digitalWrite(D3,HIGH);
} // click1

void longPressStart1(){
  StartTimeButton1 = millis();
  Serial.println("StartLong1");  
}
void longPressStart2(){
  StartTimeButton2 = millis();
  Serial.println("StartLong2");
}

void longPressStop(){
  ElapsedTimeButton1 = millis() - StartTimeButton1;
  ElapsedTimeButton2 = millis() - StartTimeButton2;
  Serial.println(ElapsedTimeButton1);
  Serial.println(ElapsedTimeButton2);
  
  if(ElapsedTimeButton1 > LONG_PRESS_MS && ElapsedTimeButton2 > LONG_PRESS_MS && !flag)
  {
    flag = true;
    Serial.println("DEADBEAF_RESET_CONFIG");
    wirelessconfig(true);
  }
}


void open(const String gwtype)
{
  
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(5000);
  Serial.println("startSecure Connection");
  if (!client.connect(domophoneIP,domophoneHttpsPort)) {
    Serial.println("Secure connection failed");
    Serial.flush();
    digitalWrite(D3,HIGH);
    return;
  }
  Serial.flush();
  Serial.println("endSecure Connection");
  String apikey = APIKEY;
  String PostData = "apikey=" + apikey + "&clientId=" + clientID + "&clientName=" + clientName + "&clientNameSize=" + clientSize + "&action=1&param1=" + gwtype;
  Serial.println(PostData);
  client.println("POST /webapi.cgi HTTP/1.1");
  String hostHeader = String("Host: ") + domophoneIP;
  client.println(hostHeader);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(PostData.length());
  client.println();
  client.println(PostData);

  Serial.println("Data posted");
  
  //client.stop();
 
  while (client.connected())
  {
    if ( client.available() )
    {
      //String str=client.readString();
      // Serial.println(str);
      client.stop();
    }      
  }
}

// Loads the configuration from a file
void loadConfiguration(const char *filename) {
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(filename))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(filename, "r");
      if (configFile) 
      {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
           Serial.println(F("Failed to read file, using default configuration")); 
        }     
        domophoneHttpsPort = doc["domophoneHttpsPort"] | domophoneHttpsPort;
        strlcpy(domophoneIP,doc["domophoneIP"] | domophoneIP, sizeof(domophoneIP));
        strlcpy(APIKEY,doc["APIKEY"] | APIKEY, sizeof(APIKEY));
        Serial.println("config file was read");
        configFile.close();
      }
    }  
    SPIFFS.end();
  } 
  else {
    Serial.println("failed to mount FS");
  }  
}

void saveConfiguration(const char *filename)
{
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    SPIFFS.remove(filename);
    File configFile = SPIFFS.open(filename,"w");
    if (!configFile) 
    {
      Serial.println(F("Failed to create file"));
      return;
    }
    StaticJsonDocument<256> doc;
    doc["domophoneHttpsPort"] = domophoneHttpsPort;
    doc["APIKEY"] = APIKEY;
    doc["domophoneIP"] = domophoneIP;
    // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0)
    {
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
    SPIFFS.end();
  } 
  else 
  {
    Serial.println("failed to mount FS");
  }  
}

