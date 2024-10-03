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

        if (this->auto_off > 0)
        {
          ESP_LOGI(TAG, "Started timer");
          this->timer = millis() + (this->auto_off * 1000);
        }
      }
    }

    void Simplebus2BinarySensor::loop()
    {
      uint32_t now_millis = millis();

      if (this->timer > 0 && now_millis >= this->timer)
      {
        ESP_LOGI(TAG, "Timer ended");
        this->publish_state(false);
        this->timer = 0;
      }
    }

  }
}
