

struct REMOTE {
  unsigned int id;
  char const* name;
  unsigned int default_rolling_code;
  uint32_t eeprom_address;
  char const* description;
};


REMOTE remotes[] = {
  {0x100000, "Kitchen",       0, 0, "Kitchen" }
  , {0x100001, "BedRoom",       0, 4, "BedRoom" }
  , {0x100002, "LivingRoom",       0, 8, "LivingRoom" }
};


const bool reset_rolling_codes = true;

const int delay_group_radio_signal = 500;

const char *ssid = "";
const char *password = "";
