#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <ESPping.h>
#include "secrets.h"

const char* pingAddress = "192.168.50.100";

IPAddress local_IP(192, 168, 50, 99);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 0, 0);

WiFiClient client;

const long interval = 10000;  // 10 seconds


void check_wifi_shield() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
      ;
  }
}

void scan_networks() {
  Serial.println("Scanning for networks...");
  int numNetworks = WiFi.scanNetworks();

  if (numNetworks == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.println("Networks found:");
    for (int i = 0; i < numNetworks; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));  // SSID name
      Serial.print(" (Signal strength: ");
      Serial.print(WiFi.RSSI(i));  // Signal strength
      Serial.println(" dBm)");
    }
  }
}

void print_ip() {
  Serial.printf("IP Address:\n\t%s\n", WiFi.localIP().toString().c_str());
}

void print_mac() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.printf("Device MAC Address:\t%02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool check_gateway(){
  IPAddress gateway(192,168,50,1);
  if (Ping.ping(gateway)){
    Serial.println("Gateway responded");
    return true;
  }
  else{
    Serial.println("Gatway did not respond");
    return false;
  }
}

bool check_xopalt(){
  IPAddress xopalt(192,168,50,100);
  if (Ping.ping(xopalt)){
    Serial.println("Xopalt responded");
    return true;
  }
  else{
    Serial.println("Xopalt did not respond");
    return false;
  }
}

bool check_google(){
  IPAddress google(8,8,8,8);
  if (Ping.ping(google)){
    Serial.println("Google responded");
    return true;
  }
  else{
    Serial.println("Google did not respond");
    return false;
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);

  check_wifi_shield();
  scan_networks();

  unsigned long start = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Attempting to connect to ");
    Serial.println(ssid);
  }
  unsigned long end = millis();


  Serial.println("\nWiFi connection Successful");
  print_ip();
  print_mac();
  Serial.println(WiFi.macAddress());

  Serial.printf("Connection took: %d ms\n", int(float(end - start) / 1000));
  
}

unsigned long previous_milli_gateway = 0;
unsigned long previous_milli_google = 0;
unsigned long previous_milli_xopalt = 0;

const long interval_gateway_max = 600'000;
const long interval_google_max = 600'000;
const long interval_xopalt_max = 600'000;

const long interval_gateway_min = 1'000;
const long interval_google_min = 1'000;
const long interval_xopalt_min = 1'000;

const long interval_gateway_deafult = 10'000;
const long interval_google_deafult = 10'000;
const long interval_xopalt_deafult = 10'000;

long interval_gateway = 10'000;
long interval_google = 10'000;
long interval_xopalt = 10'000;

int gateway_fails = 0;
int google_fails = 0;
int xopalt_fails = 0;


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previous_milli_google >= interval_google) {
    previous_milli_google = currentMillis;
    if (!check_google()){
      google_fails++;
      interval_google = interval_google_min;
    }
    else{
      google_fails = 0;
      interval_google = interval_google_deafult;
    }
    if (google_fails > 5){
      Serial.println("Google failed too many times - restarting the modem");
      interval_google = interval_google_max;
    }
  }

  if (currentMillis - previous_milli_xopalt >= interval_xopalt) {
    previous_milli_xopalt = currentMillis;
    if (!check_xopalt()){
      xopalt_fails++;
      interval_xopalt = interval_xopalt_min;
    }
    else{
      xopalt_fails = 0;
      interval_xopalt = interval_xopalt_deafult;
    }
    if (xopalt_fails > 5){
      Serial.println("Xopalt failed too many times - sending a wake signal");
      interval_xopalt = interval_xopalt_max;
    }

  }

  if (currentMillis - previous_milli_gateway >= interval_gateway) {
    previous_milli_gateway = currentMillis;

    if (!check_gateway()){
      gateway_fails++;
      interval_gateway = interval_gateway_min;
    }
    else{
      gateway_fails = 0;
      interval_gateway = interval_gateway_deafult;
    }
    if (gateway_fails > 5){
      Serial.println("Gateway failed too many times - restarting");
      interval_gateway = interval_gateway_max;
    }


  }
}
