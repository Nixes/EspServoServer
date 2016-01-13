#include <ESP8266WiFi.h>
#include <Servo.h>

#define DEBUG

// network stuff START
    const char* ssid     = ""; // <<<<<<<<< wifi network name here
    const char* password = ""; // <<<<<<<<< wifi password here

    const int port = 5643;
    WiFiServer server (port);

    unsigned long packetTimer;
    unsigned int packetCount;
// network stuff END

// servo stuff START
    Servo servoX;
    Servo servoY;

    const bool servoX_invert  = true;
    const bool servoY_invert  = false;

    const int servoYPos_min = 50;
    const int servoXPos_min = 0;

    const int servoYPos_max = 120;
    const int servoXPos_max = 180; // confirmed with servo range test

    unsigned long servoTimeout = millis(); // saves the lives of shitty hobby servos
// servo stuff END

// TODO: move into servo validation section
void servoSet(int servoXPos,int servoYPos) {


    servoTimeout = millis(); // reset servo timeout
}

void servoValidate(int servoXPos,int servoYPos)  {
    // do some input validation
	if (servoYPos >= servoYPos_min && servoYPos <= servoYPos_max) {
		#ifdef DEBUG
			Serial.println("Y value passed validation");
		#endif
        int final_ServoYPos;
        if (servoY_invert) {
            final_ServoYPos = 180 - servoYPos;
        } else {
            final_ServoYPos = servoYPos;
        }
        servoY.attach(D2);
        servoY.write(final_ServoYPos);
	}
    #ifdef DEBUG
    else {
			Serial.print("Y value failed validation, was: ");
			Serial.println(servoYPos);
    }
    #endif

	if (servoXPos >= servoXPos_min && servoXPos <= servoXPos_max) {
		#ifdef DEBUG
			Serial.println("X value passed validation");
		#endif
        int final_ServoXPos;
        if (servoX_invert) {
           final_ServoXPos = 180 - servoXPos;
        } else {
            final_ServoXPos = servoXPos;
        }
        servoX.attach(D1);
        servoX.write(final_ServoXPos);
	}
    #ifdef DEBUG
    else {
			Serial.print("X value failed validation, was: ");
			Serial.println(servoXPos);
    }
    #endif
	//servoSet(servoXPos,servoYPos);
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
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin(); // start tcp server

    packetTimer = millis();
    packetCount = 0;
}


void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char startbyte = client.read(); // read looks like it consumes the value
        if (startbyte == 0xFE) {
          char posx = client.read();
          char posy = client.read();
          char stopbyte = client.read();
          if (stopbyte == 0xFF) {
            //Serial.println("VALID PACKET");
            // do some stuff
            //int servoYPos = (int)posx;int servoXPos = (int)posy;
            servoValidate( (int)posx,(int)posy);

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
  }
    if ( (millis()-servoTimeout) > 1000) { // gives dodgey servos that twitch a break
        servoX.detach();
        servoY.detach();
    }
}
