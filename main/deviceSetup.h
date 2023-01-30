void pinSetup() {
  pinMode(pirPin, INPUT);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
}

void wifiSetup() {
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected, ip: ");
  Serial.print(WiFi.localIP());
}

void deviceSetup() {
  pinSetup();
  cameraSetup();
  wifiSetup();
}