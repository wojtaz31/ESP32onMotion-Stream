#include "allow_access.h"
WiFiUDP udp;
const char * udpAddress = "192.168.1.4";
ALLOW_ACCESS(WiFiUDP, tx_buffer, char*);
ALLOW_ACCESS(WiFiUDP, tx_buffer_len, size_t);
const char* ssid = "your ssid";
const char* password = "your password";
char buffer_size[8192];
const int udpPort = 6000;
const int pirPin = 14;