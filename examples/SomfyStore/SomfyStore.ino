/*
   multiple_accessories.ino

    Created on: 2020-05-16
        Author: Mixiaoxiao (Wang Bin)


   This example is a bridge (aka a gateway) which contains multiple accessories.

   This example includes 6 sensors:
   1. Temperature Sensor (HAP section 8.41)
   2. Humidity Sensor (HAP section 8.20)
   3. Light Sensor (HAP section 8.24)
   4. Contact Sensor (HAP section 8.9)
   5. Motion Sensor (HAP section 8.28)
   6. Occupancy Sensor (HAP section 8.29)

   You should:
   1. read and use the Example01_TemperatureSensor with detailed comments
      to know the DOWNic concept and usage of this library before other examples。
   2. erase the full flash or call homekit_storage_reset() in setup()
      to remove the previous HomeKit pairing storage and
      enable the pairing with the new accessory of this new HomeKit example.
*/


#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <EEPROM.h>
#include <EEPROMRollingCodeStorage.h>
#include <SomfyRemote.h>

#include "config.h"
#include "wifi_info.h"


#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define PORT_TX D1 // Output data on pin 23 (can range from 0 to 31). Check pin numbering on ESP8266.

#define REMOTE1 0x130040

EEPROMRollingCodeStorage rollingCodeStorage1(0);
SomfyRemote somfyRemote1(PORT_TX, REMOTE1, &rollingCodeStorage1);

#define POSITION_OPEN 100
#define POSITION_CLOSED 0
#define POSITION_STATE_CLOSING 0
#define POSITION_STATE_OPENING 1
#define POSITION_STATE_STOPPED 2

// Buttons
#define SYMBOL 640
#define UP 0x2
#define STOP 0x1
#define DOWN 0x4
#define PROG 0x8

byte frame[7];

void BuildFrame(byte *frame, byte button, REMOTE remote);
void SendCommand(byte *frame, byte sync);

const String string;

void setup() {
  Serial.begin(115200);

  pinMode(PORT_TX, OUTPUT);
  digitalWrite(PORT_TX, LOW);


  EEPROM.begin(4000);

  unsigned int current_code;
  EEPROM.get( 0, current_code );
  LOG_D("rolling_codes: %u", current_code);

  if ( reset_rolling_codes )
  {
    EEPROM.put( 0, 390 );
    EEPROM.commit();
  }

  wifi_connect(); // in wifi_info.h
  //homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  my_homekit_setup();
}

void loop() {
  my_homekit_loop();
  delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_current_position;
extern "C" homekit_characteristic_t cha_position_state;
extern "C" homekit_characteristic_t cha_target_position;

// Called when the value is read by iOS Home APP
homekit_value_t cha_programmable_switch_event_getter() {
  // Should always return "null" for reading, see HAP section 9.75
  return HOMEKIT_NULL_CPP();
}

void cha_target_position_setter(const homekit_value_t value) {
  uint8_t will = value.uint8_value;

  String string;

  homekit_characteristic_notify(&cha_current_position, cha_current_position.value);
  homekit_characteristic_notify(&cha_position_state, cha_position_state.value);
  homekit_characteristic_notify(&cha_target_position, cha_target_position.value);

  LOG_D("Current...");
  LOG_D("current %u, target %u, position %u, will %u", cha_current_position.value, cha_target_position.value, cha_position_state.value, will );

  cha_target_position.value.uint8_value = will;

  if (cha_current_position.value.uint8_value > cha_target_position.value.uint8_value) {
    cha_position_state.value.uint8_value = POSITION_STATE_CLOSING;
    string = "Down";
  } else if (cha_current_position.value.uint8_value < cha_target_position.value.uint8_value) {
    cha_position_state.value.uint8_value = POSITION_STATE_OPENING;
    string = "Up";
  } else {
    cha_position_state.value.uint8_value = POSITION_STATE_STOPPED;
    string = "My";
  }

  homekit_characteristic_notify(&cha_current_position, cha_current_position.value);
  homekit_characteristic_notify(&cha_position_state, cha_position_state.value);
  homekit_characteristic_notify(&cha_target_position, cha_target_position.value);

  LOG_D("Targeting...");
  LOG_D("current %u, target %u, position %u, will %u", cha_current_position.value.uint8_value, cha_target_position.value.uint8_value, cha_position_state.value.uint8_value, will );

  cha_target_position.value.uint8_value = will;

  homekit_characteristic_notify(&cha_current_position, cha_current_position.value);
  homekit_characteristic_notify(&cha_position_state, cha_position_state.value);
  homekit_characteristic_notify(&cha_target_position, cha_target_position.value);

  LOG_D("Moving...");
  LOG_D("current %u, target %u, position %u, will %u", cha_current_position.value.uint8_value, cha_target_position.value.uint8_value, cha_position_state.value.uint8_value, will );
  
  cha_current_position.value.uint8_value = will;

  homekit_characteristic_notify(&cha_current_position, cha_current_position.value);
  homekit_characteristic_notify(&cha_position_state, cha_position_state.value);
  homekit_characteristic_notify(&cha_target_position, cha_target_position.value);

  LOG_D("Finished");
  LOG_D("current %u, target %u, position %u, will %u", cha_current_position.value.uint8_value, cha_target_position.value.uint8_value, cha_position_state.value.uint8_value, will );
  
  const Command command = getSomfyCommand(string);
  somfyRemote1.sendCommand(command);
}

void my_homekit_setup() {
  cha_target_position.setter = cha_target_position_setter;
  arduino_homekit_setup(&config);
}

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // Show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

  }
}
