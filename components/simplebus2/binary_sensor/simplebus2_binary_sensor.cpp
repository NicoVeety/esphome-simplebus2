#include "simplebus2_binary_sensor.h"

namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2.binary";

    // Trigger function to handle incoming command and address and set the state
    void Simplebus2BinarySensor::trigger(uint16_t command, uint16_t address)
    {
      // Log received command and address for better debugging
      ESP_LOGI(TAG, "Received command %d, address %d", command, address);
      
      // Check if the command and address match
      if (this->command == command && this->address == address)
      {
        // Publish the sensor state as true
        this->publish_state(true);

        // Start the auto-off timer if auto_off is set (non-zero) and timer is not already running
        if (this->auto_off > 0 && this->timer == 0)
        {
          ESP_LOGI(TAG, "Started timer for %d seconds", this->auto_off);
          this->timer = millis();  // Record the current time when the timer started
        }
      }
      else
      {
        // Log if the command does not match to debug if something else interrupts the flow
        ESP_LOGI(TAG, "Command or address did not match. Ignoring.");
      }
    }

    // Loop function to check if the timer has expired and turn off the sensor
    void Simplebus2BinarySensor::loop()
    {
      uint32_t now_millis = millis();
      
      // Log to ensure loop is being called frequently
      ESP_LOGI(TAG, "Loop is being called. Current time: %u, Timer: %u", now_millis, this->timer);

      // Check if the timer is active
      if (this->timer > 0)
      {
        // Check if the time difference exceeds the auto_off duration
        if (now_millis - this->timer >= this->auto_off * 1000)
        {
          ESP_LOGI(TAG, "Timer ended, publishing state false");
          // Publish the sensor state as false and reset the timer
          this->publish_state(false);
          this->timer = 0;  // Reset timer
        }
      }
    }

  }
}
