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
unsigned long previousMillis = 0;

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

bool check_google() {
  if (client.connect("www.neverssl.com", 80)) {
    Serial.println("Successfully connected to nastrurcja!");
    client.stop();
    return true;
  } else {
    Serial.println("Failed to connect to nasturcja.");
    return false;
  }
}

// bool check_gateway() {
//   if (client.connect("192.168.50.1", 80)) {
//     Serial.println("Successfully connected to Gateway!");
//     client.stop();
//     return true;
//   } else {
//     Serial.println("Failed to connect to Gateway.");
//     return false;
//   }
// }

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // check_gateway();
    check_google();
  }
}
