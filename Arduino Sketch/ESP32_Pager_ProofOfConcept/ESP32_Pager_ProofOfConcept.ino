/*
ESP32 Pager Proof Of Concept



*/
#include "periph.h"
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>




SX1278 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
// create Pager client instance using the FSK module
PagerClient pager(&radio);


#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);




// Global Config variables. Might be worth moving them to EEPROM at some time
float offset = 0.0044;  // device specific, in MHz. VHF: 0.0014 UHF: 0.0044
float frequency = 439.98750;


void setup() {
  Serial.begin(115200);
  displayInit();
  hwInit();
  pocsagInit();
  pocsagStartRx();
}

void loop() {
  // the number of batches to wait for
  // 2 batches will usually be enough to fit short and medium messages
  if (pager.available() >= 2) {
    Serial.print(F("[Pager] Received pager data, decoding ... "));
    // you can read the data as an Arduino String
    String str;
    uint32_t addr = 0;
    int state = pager.readData(str, 0, &addr);
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));

      // print the received data
      Serial.print(F("[Pager] Address:\t"));
      Serial.print(String(addr));
      Serial.print(F("[Pager] Data:\t"));
      Serial.println(str);


      if(addr==65009){
        displayPage(addr,str);
        ringBuzzer();
      }

    } else {
      // some error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
  }
}


void displayPage(uint32_t address, String text) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(String(address));
  display.setCursor(0, 30);
  display.print(text);
  display.display();
}

void ringBuzzer(){
  tone(BUZZER, 2731, 130);
  tone(BUZZER, 3202, 130);
  tone(BUZZER, 2731, 130);
  tone(BUZZER, 3203, 130);
  tone(BUZZER, 2731, 130);
}


void displayInit() {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
      ;  // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("PAGER RX Proof of Concept ");
  display.display();
}


void hwInit() {
  tone(BUZZER, 2731, 120);
  noTone(BUZZER);
  delay(300);
  tone(BUZZER, 3202, 120);
  noTone(BUZZER);
}




void pocsagInit() {
  // initialize SX1278 with default settings
  Serial.print(F("[SX1278] Initializing ... "));
  int state = radio.beginFSK();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

  // initialize Pager client
  Serial.print(F("[Pager] Initializing ... "));
  state = pager.begin(frequency + offset, 1200);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
}

void pocsagStartRx() {
  // start receiving POCSAG messages
  Serial.print(F("[Pager] Starting to listen ... "));
  // address of this "pager":     1234567
  int state = pager.startReceive(LORA_DIO2, 200, 0);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
}