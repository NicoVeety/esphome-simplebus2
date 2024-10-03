#include "simplebus2_binary_sensor.h"

namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2.binary";

    class Simplebus2BinarySensor : public PollingComponent, public BinarySensor
    {
    public:
      uint32_t timer{0};
      uint16_t command;
      uint16_t address;
      uint16_t auto_off;

      Simplebus2BinarySensor() : PollingComponent(1000)
      {
      }

      void trigger(uint16_t command, uint16_t address)
      {
        if (this->command == command && this->address == address)
        {
          this->publish_state(true);
          if (this->auto_off > 0)
          {
            ESP_LOGI(TAG, "Started timer for auto_off: %d seconds", this->auto_off);
            this->timer = static_cast<uint32_t>(millis() + (this->auto_off * 1000));
          }
        }
      }
      void loop() override
      {
        uint32_t now_millis = millis();
        ESP_LOGI(TAG, "now_millis: %u, timer: %u", now_millis, this->timer);

        if (this->timer && now_millis >= this->timer)
        {
          ESP_LOGI(TAG, "Timer ended");
          this->publish_state(false);
          this->timer = 0;
        }
      }
    };

  }
}
