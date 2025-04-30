#include "ESP8266WiFi.h"
#include <ESPping.h>
#include "secrets.h"
#include "config.h"

void cycle_relay(long duration, long wait)
{
  digitalWrite(relay_pin, LOW);
  long start_time = millis();
  Serial.println("Starting the reset");
  while (millis() - start_time < duration)
  {
    delay(50);
  }
  digitalWrite(relay_pin, HIGH);
  Serial.println("Reset complete\nWaiting for modem to come back online...");
  delay(wait);
  Serial.println("Waiting complete");
}

void config_wifi()
{
  Serial.println("Configuring Wi-Fi module");
  WiFi.mode(WIFI_STA);
  WiFi.config(device_ip, router_ip, subnet);
}

void check_wifi_shield()
{
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
      ;
  }
}

void scan_networks()
{
  Serial.println("Scanning for networks...");
  int numNetworks = WiFi.scanNetworks();

  if (numNetworks == 0)
  {
    Serial.println("No networks found.");
  }
  else
  {
    Serial.println("Networks found:");
    for (int i = 0; i < numNetworks; i++)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (Signal strength: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
    }
  }
}

void print_ip()
{
  Serial.printf("IP Address:\n\t%s\n", WiFi.localIP().toString().c_str());
}

void print_mac()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.printf("Device MAC Address:\t%02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool check_router()
{
  if (Ping.ping(router_ip))
  {
    Serial.println("Router responded");
    return true;
  }
  else
  {
    Serial.println("Router did not respond");
    return false;
  }
}

bool check_server()
{
  if (Ping.ping(server_ip))
  {
    Serial.println("Server responded");
    return true;
  }
  else
  {
    Serial.println("Server did not respond");
    return false;
  }
}

bool check_remote()
{
  if (Ping.ping(remote_ip))
  {
    Serial.println("Remote responded");
    return true;
  }
  else
  {
    Serial.println("Remote did not respond");
    return false;
  }
}

void connect_wifi()
{
  unsigned long start = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
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
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);
}

void monitoring_loop()
{
}

void setup()
{
  // Avoid sending garbage during initialization of the serial port
  delay(4'000);
  Serial.begin(9600);
  delay(4'000);

  check_wifi_shield();
  config_wifi();
  scan_networks();
  connect_wifi();

  Serial.println("Initializing Pins");

  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, HIGH);

  Serial.println("Setup Complete");
}

unsigned long previous_milli_gateway = 0;
unsigned long previous_milli_remote = 0;
unsigned long previous_milli_server = 0;

int modem_fails = 0;
int remote_fails = 0;
int server_fails = 0;

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previous_milli_remote >= interval_remote)
  {
    previous_milli_remote = currentMillis;
    if (!check_remote())
    {
      remote_fails++;
    }
    else
    {
      remote_fails = 0;
    }
    if (remote_fails >= max_remote_fails)
    {
      Serial.println("Remote failed too many times - restarting the modem");
      cycle_relay(reset_duration, post_reset_wait);
    }
  }

  if (currentMillis - previous_milli_server >= interval_server)
  {
    previous_milli_server = currentMillis;
    if (!check_server())
    {
      server_fails++;
    }
    else
    {
      server_fails = 0;
    }
    if (server_fails >= max_server_fails)
    {
      Serial.println("Server failed too many times - sending a wake signal");
      // TODO: add a magic packet sender here
    }
  }

  if (currentMillis - previous_milli_gateway >= interval_gateway)
  {
    previous_milli_gateway = currentMillis;
    if (!check_router())
    {
      modem_fails++;
    }
    else
    {
      modem_fails = 0;
    }
    if (modem_fails >= max_modem_fails)
    {
      Serial.println("Modem failed too many times - restarting");
      cycle_relay(reset_duration, post_reset_wait);
    }
  }
}
