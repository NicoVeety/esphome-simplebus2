#include "simplebus2_binary_sensor.h"

namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2.binary";

    void Simplebus2BinarySensor::trigger(uint16_t command, uint16_t address)
    {
      ESP_LOGI(TAG, "Received command %d, address %d", command, address);
      
      if (this->command == command && this->address == address)
      {
        this->publish_state(true);

        if (this->auto_off > 0 && this->timer == 0)
        {
          ESP_LOGI(TAG, "Started timer for %d seconds", this->auto_off);
          this->timer = millis();
        }
      }
      else
      {
        ESP_LOGI(TAG, "Command or address did not match. Ignoring.");
      }
    }

    void Simplebus2BinarySensor::loop()
    {
      uint32_t now_millis = millis();
    
      ESP_LOGI(TAG, "Loop is being called. Current time: %u, Timer: %u", now_millis, this->timer);

      if (this->timer > 0)
      {
        if (now_millis - this->timer >= this->auto_off * 1000)
        {
          ESP_LOGI(TAG, "Timer ended, publishing state false");
          this->publish_state(false);
          this->timer = 0;
        }
      }
    }

  }
}
