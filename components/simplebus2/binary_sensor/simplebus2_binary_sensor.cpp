#include "simplebus2_binary_sensor.h"

namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2.binary";

    void Simplebus2BinarySensor::trigger(uint16_t command, uint16_t address)
    {
      if (this->command == command && this->address == address)
      {
        ESP_LOGI(TAG, "Checking condition 1: command = %d, address = %d", this->command, this->address);
        this->publish_state(true);
      }
      else if  (this->command == 63 && this->address == 255)
      {
        ESP_LOGI(TAG, "Checking condition 2: command = 63, address = 255");
        this->publish_state(false);
      }
    }
  }
}
