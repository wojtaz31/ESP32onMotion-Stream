#include "includes.h"

void setup() {
  deviceSetup();
}

void loop() {
  if(digitalRead(pirPin)){
    stream_handler();
  }
}
