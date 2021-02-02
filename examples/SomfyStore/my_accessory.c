/*
   my_accessory.c
   Define the accessory in C language using the Macro in characteristics.h

    Created on: 2020-05-16
        Author: Mixiaoxiao (Wang Bin)
*/

#include <homekit/homekit.h>
#include <homekit/characteristics.h>


void my_accessory_identify(homekit_value_t _value) {
  printf("accessory identify\n");
}

#define POSITION_OPEN 100
#define POSITION_CLOSED 0
#define POSITION_STATE_CLOSING 0
#define POSITION_STATE_OPENING 1
#define POSITION_STATE_STOPPED 2

homekit_characteristic_t cha_current_position = HOMEKIT_CHARACTERISTIC_(CURRENT_POSITION, POSITION_CLOSED);
homekit_characteristic_t cha_position_state = HOMEKIT_CHARACTERISTIC_(POSITION_STATE, POSITION_STATE_STOPPED);
homekit_characteristic_t cha_target_position = HOMEKIT_CHARACTERISTIC_(TARGET_POSITION, POSITION_CLOSED);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_bridge, .services = (homekit_service_t*[]) {
    // HAP section 8.17:
    // For a bridge accessory, only the primary HAP accessory object must contain this(INFORMATION) service.
    // But in my test,
    // the bridged accessories must contain an INFORMATION service,
    // otherwise the HomeKit will reject to pair.
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Somfy RTS"),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ToastCorp"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
    NULL
  }),
  HOMEKIT_ACCESSORY(.id = 2, .category = homekit_accessory_category_sensor, .services = (homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Kitchen"),
                             HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
                             NULL
    }),
    HOMEKIT_SERVICE(WINDOW_COVERING, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Kitchen"),
                             &cha_current_position,
                             &cha_position_state,
                             &cha_target_position,
                             NULL
    }),
    NULL
  }),
  NULL
};

homekit_server_config_t config = {
  .accessories = accessories,
  .password = "111-11-111",
  .setupId = "2AD1"
};
