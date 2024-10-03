#include "simplebus2_binary_sensor.h"

namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2.binary";

    // Trigger function to handle incoming command and address and set the state
    void Simplebus2BinarySensor::trigger(uint16_t command, uint16_t address)
    {
      // Check if the command and address match
      if (this->command == command && this->address == address)
      {
        // Publish the sensor state as true
        this->publish_state(true);

        // Start the auto-off timer if auto_off is set (non-zero) and timer is not already running
        if (this->auto_off > 0 && this->timer == 0)
        {
          ESP_LOGI(TAG, "Started timer for %d seconds", this->auto_off);
          this->timer = millis() + (this->auto_off * 1000);  // Set timer expiration time
        }
      }
    }

    // Loop function to check if the timer has expired and turn off the sensor
    void Simplebus2BinarySensor::loop()
    {
      uint32_t now_millis = millis();

      // Check if the timer is active and has expired
      if (this->timer > 0 && now_millis >= this->timer)
      {
        ESP_LOGI(TAG, "Timer ended, publishing state false");
        // Publish the sensor state as false and reset the timer
        this->publish_state(false);
        this->timer = 0;  // Reset timer
      }
    }

  }
}
