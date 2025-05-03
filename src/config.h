// Hardware setup
const uint8_t relay_pin = 2;

// Wi-Fi setup
const IPAddress device_ip(192, 168, 50, 99);
const IPAddress router_ip(192, 168, 50, 1);
const IPAddress subnet(255, 255, 0, 0);

// Modem/router
const IPAddress remote_ip(8, 8, 8, 8); // Google DNS

const long interval_gateway = 10'000;
const long interval_remote = 60'000;

const long reset_duration = 10'000;
const long post_reset_wait = 450'000;

const int max_remote_fails = 5;
const int max_modem_fails = 5;

// Server
const IPAddress server_ip(192, 168, 50, 100);
const long interval_server = 10'000;
const int max_server_fails = 15;