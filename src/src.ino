#include "ESP8266WiFi.h"
#include <ESPping.h>
#include "secrets.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WakeOnLan.h>

WiFiUDP UDP;
WakeOnLan WOL(UDP);

void cycle_relay(long duration, long wait)
{
  digitalWrite(RELAY_PIN, LOW);
  long start_time = millis();
  Serial.println("Starting the reset");
  while (millis() - start_time < duration)
  {
    delay(50);
  }
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Reset complete\nWaiting for modem to come back online...");
  delay(wait);
  Serial.println("Waiting complete");
}

void wake_server()
{
  WOL.sendMagicPacket(mac_address);
}

void config_wifi()
{
  Serial.println("Configuring Wi-Fi module");
  WiFi.mode(WIFI_STA);
  WiFi.config(DEVICE_IP, ROUTER_IP, SUBNET);
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
  if (Ping.ping(ROUTER_IP))
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
  if (Ping.ping(SERVER_IP))
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
  if (Ping.ping(REMOTE_IP_1))
  {
    Serial.println("Remote 1 responded");
    return true;
  }
  if (Ping.ping(REMOTE_IP_2))
  {
    Serial.println("Remote 2 responded");
    return true;
  }
  if (Ping.ping(REMOTE_IP_3))
  {
    Serial.println("Remote 3 responded");
    return true;
  }

  Serial.println("No remote respond");
  return false;
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

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Seting up WOL");
  WOL.setRepeat(3, 1'000);
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

  Serial.println("Setup Complete");
}

// Check the type
String connection_status(int status)
{
  switch (status)
  {
  case 0:
    return "Idle";
  case 1:
    return "SSID not available";
  case 3:
    return "Connected";
  case 4:
    return "Connection Failed";
  case 6:
    return "Wrong Password";
  case 7:
    return "Disconnected";
  }
  return "Unknown";
}

// Runs to make sure that the chip is still connected 
bool check_connection()
{
  auto status = WiFi.status();
  if (status == 3)
  {
    Serial.println("The WiFi status is fine");
  }

  WiFi.reconnect()

  Serial.printf("Connection status: %s\n", connection_status(status));
}

unsigned long previous_milli_gateway = 0;
unsigned long previous_milli_remote = 0;
unsigned long previous_milli_server = 0;

int modem_fails = 0;
int remote_fails = 0;
int server_fails = 0;

bool relay_cycle_requested = false;
bool relay_cycle_triggered = false;
bool activate = true;

unsigned long relay_shutdown_milli = 0;
unsigned long next_check_milli = 0;

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previous_milli_remote >= INTERVAL_REMOTE && activate)
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
    if (remote_fails >= MAX_REMOTE_FAILS)
    {
      Serial.println("Remote failed too many times - restarting the modem");
      cycle_relay(RESET_DURATION, POST_RESET_WAIT);
      remote_fails = 0; // trying to give the server a better chance
    }
  }

  if (currentMillis - previous_milli_server >= INTERVAL_SERVER)
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
    if (server_fails >= MAX_SERVER_FAILS)
    {
      Serial.println("Server failed too many times - sending a wake signal");
      wake_server();
    }
  }

  if (currentMillis - previous_milli_gateway >= INTERVAL_GATEWAY)
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
    if (modem_fails >= MAX_MODEM_FAILS)
    {
      Serial.println("Modem failed too many times - restarting");
      cycle_relay(RESET_DURATION, POST_RESET_WAIT);
    }
  }

  // Read incoming serial
  if (Serial.available() > 0)
  {
    String received = Serial.readStringUntil('\n');
    Serial.println(received);
  }
}

enum State {
  REGULAR,
  RELAY_RESET,
  WAITING_FOR_CONNECTION,
  PROBLEM
};

struct MonitoringStates
{
  unsigned long current_millis = 0;
  State current_state = REGULAR;
  State previous_state = REGULAR;
  unsigned long last_state_change = 0;
  int modem_fails = 0;
  int server_fails = 0;
  int remote_fails = 0;
  unsigned long last_modem_check = 0;
  unsigned long last_server_check = 0;
  unsigned long last_remote_check = 0;
  bool relay_reset_requested = true;
  bool relay_reset_started = false;
  unsigned long relay_reset_start_millis = 0;
  unsigned long last_connection_check = 0;
  int wifi_status = 0;
};

void change_state(MonitoringStates& s, State new_state)
{
  s.previous_state = s.current_state;
  s.current_state = new_state;
  s.last_state_change = s.current_millis;
}

void monitor_remote(MonitoringStates& monitoring_states)
{
  if (monitoring_states.current_millis - monitoring_states.last_remote_check >= INTERVAL_REMOTE)
  {
    monitoring_states.last_remote_check = monitoring_states.current_millis;
    if (check_remote())
    {
      monitoring_states.remote_fails = 0;
      return;
    }
    monitoring_states.remote_fails++;
  }
}

void monitor_server(MonitoringStates& monitoring_states)
{
  if (monitoring_states.current_millis - monitoring_states.last_server_check >= INTERVAL_SERVER)
  {
    monitoring_states.last_server_check = monitoring_states.current_millis;
    if (check_server())
    {
      monitoring_states.server_fails = 0;
      return;
    }
    monitoring_states.server_fails++;
  }
}
void monitor_router(MonitoringStates& monitoring_states)
{
  
  if (monitoring_states.current_millis - monitoring_states.last_modem_check >= INTERVAL_GATEWAY)
  {
    monitoring_states.last_modem_check = monitoring_states.current_millis;
    if (check_router())
    {
      monitoring_states.modem_fails = 0;
      return;
    }
    monitoring_states.modem_fails++;
  }
}

void regular(MonitoringStates& monitoring_states)
{
  monitor_remote(monitoring_states);
  monitor_server(monitoring_states);
  monitor_router(monitoring_states);

  if (monitoring_states.server_fails >= MAX_SERVER_FAILS)
  {
    monitoring_states.server_fails = 0;
    Serial.println("Server did not respond too many times, waking-up");
    wake_server();
  }

  if (monitoring_states.remote_fails >= MAX_REMOTE_FAILS)
  {
    monitoring_states.remote_fails = 0;
    monitoring_states.modem_fails = 0;
    Serial.println("Remote failed too many times - restarting");
    change_state(monitoring_states, RELAY_RESET);
    monitoring_states.relay_reset_requested = true;
  }

  if (monitoring_states.modem_fails >= MAX_MODEM_FAILS)
  {
    monitoring_states.modem_fails = 0;
    monitoring_states.remote_fails = 0;
    Serial.println("Modem failed too many times - restarting");
    change_state(monitoring_states, RELAY_RESET);
    monitoring_states.relay_reset_requested = true;
  }
}

void relay_reset(MonitoringStates& monitoring_states)
{
  // Relay reset has not started - so we need to start it 
  if (monitoring_states.relay_reset_requested && !monitoring_states.relay_reset_started)
  {
    monitoring_states.relay_reset_requested = false;
    monitoring_states.relay_reset_started = true;
    monitoring_states.relay_reset_start_millis = monitoring_states.current_millis;
    Serial.println("Starting the reset");
    digitalWrite(RELAY_PIN, LOW);
  }

  // Reset complete
  if (monitoring_states.relay_reset_started && monitoring_states.current_millis - monitoring_states.relay_reset_start_millis >= RESET_DURATION)
  {
    digitalWrite(RELAY_PIN, HIGH);
    monitoring_states.relay_reset_started = false;
    Serial.println("Reset completed");
    change_state(monitoring_states, WAITING_FOR_CONNECTION);
  }  
}

void wait_for_connection(MonitoringStates& monitoring_states)
{
  if (monitoring_states.current_millis - (monitoring_states.relay_reset_start_millis + RESET_DURATION) < POST_RESET_WAIT)
  {
    return;
  }
  if (monitoring_states.current_millis - monitoring_states.last_connection_check < POST_RESET_CHECK_INTERVAL)
  {
    return;
  }

  if (monitoring_states.current_millis - monitoring_states.last_state_change > POST_RESET_RECONNECTION_TIMEOUT)
  {
    Serial.println("Unable to connect to WiFi after 20 min, resetting the board");
    ESP.restart(); 
    return;
  }
  monitoring_states.last_connection_check = monitoring_states.current_millis;

  Serial.println("Checking connection");
  if (monitoring_states.wifi_status == 3)
  {
    change_state(monitoring_states, REGULAR);
    return;
  }

  Serial.printf("Connection status: %s\n", connection_status(monitoring_states.wifi_status));
}

void problem(MonitoringStates& monitoring_states)
{

}
void new_main()
{
  static MonitoringStates monitoring_states;
  monitoring_states.current_millis = millis();
  monitoring_states.wifi_status = WiFi.status();

  switch (monitoring_states.current_state)
  {
    case REGULAR:
      regular(monitoring_states);
      break;
    case RELAY_RESET:
      relay_reset(monitoring_states);
      break;
    case WAITING_FOR_CONNECTION:
      wait_for_connection(monitoring_states);
      break;
    case PROBLEM:
      problem(monitoring_states);
      break;
  }

  if (Serial.available() > 0)
  {
    String received = Serial.readStringUntil('\n');
    Serial.println(received);
  }
}
