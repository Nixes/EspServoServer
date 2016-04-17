#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Servo.h>

#define DEBUG

// network stuff START
  const char* ssid     = ""; // <<<<<<<<< wifi network name here
  const char* password = ""; // <<<<<<<<< wifi password here

  const int port = 5643;
  WiFiUDP server;

  unsigned long packetTimer;
  unsigned int packetCount;
// network stuff END

// servo stuff START
  Servo PanX;
  Servo TiltY;

  const bool PanX_invert  = true;
  const bool TiltY_invert  = false;

  const int TiltYPos_min = 50;
  const int PanXPos_min = 0;

  const int TiltYPos_max = 120;
  const int PanXPos_max = 180; // confirmed with servo range test

  unsigned long servoTimeout = millis(); // time since a servo command was recieved
// servo stuff END

int setMinMax(const int* min,const int* max,int* value) {
  int newValue;
  if (value > max) {
    newValue = *max;
  } else if (value < min) {
    newValue = *min;
  }
  return newValue;
}

void servoValidate(int PanXPos,int TiltYPos)  {
  // do some input validation
  if (TiltYPos >= TiltYPos_min && TiltYPos <= TiltYPos_max) {
    #ifdef DEBUG
    Serial.print("Tilt value passed validation = ");
    Serial.println(TiltYPos);
    #endif
    int final_TiltYPos;
    if (TiltY_invert) {
      final_TiltYPos = 180 - TiltYPos;
    } else {
      final_TiltYPos = TiltYPos;
    }
    #ifdef DEBUG
    Serial.print("Final Tilt Value = ");
    Serial.println(final_TiltYPos);
    #endif
    TiltY.attach(D2);
    TiltY.write(final_TiltYPos);
    servoTimeout = millis(); // reset servo timeout
  }
  #ifdef DEBUG
  else {
    Serial.print("Tilt value failed validation, was: ");
    Serial.println(TiltYPos);
    TiltY.attach(D2);
    TiltY.write(setMinMax(&TiltYPos_min,&TiltYPos_max,&TiltYPos));
    servoTimeout = millis(); // reset servo timeout
  }
  #endif

  if (PanXPos >= PanXPos_min && PanXPos <= PanXPos_max) {
    #ifdef DEBUG
    Serial.print("Pan value passed validation = ");
    Serial.println(TiltYPos);
    #endif
    int final_PanXPos;
    if (PanX_invert) {
      final_PanXPos = 180 - PanXPos;
    } else {
      final_PanXPos = PanXPos;
    }
    #ifdef DEBUG
    Serial.print("Final Pan Value = ");
    Serial.println(final_PanXPos);
    #endif
    PanX.attach(D1);
    PanX.write(final_PanXPos);
    servoTimeout = millis(); // reset servo timeout
  }
  #ifdef DEBUG
  else {
    Serial.print("Pan value failed validation, was: ");
    Serial.println(PanXPos);
    PanX.attach(D1);
    PanX.write(setMinMax(&PanXPos_min,&PanXPos_min,&PanXPos));
    servoTimeout = millis(); // reset servo timeout
  }
  #endif
  //servoSet(PanXPos,TiltYPos);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin(port); // start udp server
  Serial.print("UDP Server Ready on port: ");
  Serial.println(port);

  packetTimer = millis();
  packetCount = 0;
}


void loop() {
  int cb = server.parsePacket();
  if (!cb) {
    //Serial.println("no packet yet");
  } else {
    while (server.available()) {
      char startbyte = server.read(); // read looks like it consumes the value
      if (startbyte == 0xFE) {
        char pan = server.read();
        char tilt = server.read();
        char stopbyte = server.read();
        if (stopbyte == 0xFF) {
          //Serial.println("VALID PACKET");
          // do some stuff
          servoValidate( (int)pan,(int)tilt);
          delay(20); // needed to allow servos to do stuff
          // measure some stuff
          packetCount++;
          if (packetTimer + 1000 < millis()) {
            Serial.print("    Packets/Sec: ");
            Serial.println(packetCount);
            packetCount=0;
            packetTimer=millis();
          }
        }
      }
    }
  }
  if ( (millis()-servoTimeout) > 1000) { // gives dodgey servos that twitch a break
    PanX.detach();
    TiltY.detach();
  }
}
