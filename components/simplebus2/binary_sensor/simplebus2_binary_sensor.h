#pragma once
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "../simplebus2.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"

namespace esphome
{
  namespace simplebus2
  {

    class Simplebus2BinarySensor : public binary_sensor::BinarySensor, public Component, public Simplebus2Listener
    {
    public:
      void trigger(uint16_t command, uint16_t address) override;
      
      void loop() override;

      void set_command(uint16_t command) { this->command = command; }
      void set_address(uint16_t address) { this->address = address; }
      void set_auto_off(uint16_t auto_off) { this->auto_off = auto_off; }    

    protected:
      uint32_t timer = 0;
      uint16_t address = 0;
      uint16_t command = 0;
      uint16_t auto_off = 0;
    };

  }
}
