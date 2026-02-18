// Hardware setup
const uint8_t RELAY_PIN = 2;

// Wi-Fi setup
const IPAddress DEVICE_IP(192, 168, 50, 99);
const IPAddress ROUTER_IP(192, 168, 50, 1);
const IPAddress SUBNET(255, 255, 0, 0);

// Modem/router
const IPAddress REMOTE_IP_1(8, 8, 8, 8); // Google DNS
const IPAddress REMOTE_IP_2(8, 8, 4, 4); // Google DNS
const IPAddress REMOTE_IP_3(4, 2, 2, 1); // Centurylink DNS

const long INTERVAL_GATEWAY = 10'000;
const long INTERVAL_REMOTE = 60'000;

const long RESET_DURATION = 10'000;
const long POST_RESET_WAIT = 450'000;

const int MAX_REMOTE_FAILS = 15;
const int MAX_MODEM_FAILS = 5;
const int MAX_SERVER_FAILS = 10;

const int POST_RESET_CHECK_INTERVAL = 30'000;
const int POST_RESET_RECONNECTION_TIMEOUT = 1'200'000; // 20 min 

// Server
const IPAddress SERVER_IP(192, 168, 50, 100);
const long INTERVAL_SERVER = 60'000;
const int MAX_SERVER_FAILS = 30;