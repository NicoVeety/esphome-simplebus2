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
        this->publish_state(true);
      }
      else if  (this->command == 55 && this->address == 255)
      {
        this->publish_state(false);
      }
    }
  }
}
