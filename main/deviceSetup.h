void deviceSetup(){
  pinMode(pirPin, INPUT);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  cameraSetup();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}
