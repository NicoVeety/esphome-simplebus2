#include "simplebus2.h"
#include "esphome/core/log.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/application.h"
#include <Arduino.h>
#include <Wire.h>


namespace esphome
{
  namespace simplebus2
  {

    static const char *const TAG = "simplebus2";

    void Simplebus2Component::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up Simplebus2");

      this->rx_pin->setup();
      this->tx_pin->setup();

      ledcSetup(0, 25000, 8);
      ledcAttachPin(this->tx_pin->get_pin(), 0);

      auto &s = this->store_;

      this->high_freq_.start();

      s.rx_pin = this->rx_pin->to_isr();

      this->rx_pin->attach_interrupt(Simplebus2ComponentStore::gpio_intr, &this->store_, gpio::INTERRUPT_RISING_EDGE);

      ESP_LOGCONFIG(TAG, "Setup for Simplebus2 complete");
    }

    void Simplebus2Component::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Simplebus2:");
    }

    void Simplebus2Component::loop()
    {
      for (auto &listener : listeners_)
      {
        listener->loop();
      }

      if (this->store_.pin_triggered)
      {
        this->process_interrupt();
      }

      if (this->message_code > 0)
      {
        ESP_LOGD(TAG, "Received command %i, address %i", this->message_code, this->message_addr);
        for (auto &listener : listeners_)
        {
          listener->trigger(this->message_code, this->message_addr);
        }

        this->message_code = -1;
      }
    }

    void IRAM_ATTR HOT Simplebus2ComponentStore::gpio_intr(Simplebus2ComponentStore *arg)
    {
      if (!arg->pin_triggered)
      {
        arg->pin_triggered = true;
      }
    }

    void Simplebus2Component::process_interrupt()
    {
      auto &s = this->store_;
      
      unsigned long now = micros();
      unsigned long pause_time = now - this->last_pause_time;

      if (pause_time > 18000 && this->message_started)
      {
        ESP_LOGD(TAG, "Resetting preamble - %i", pause_time);
        this->message_started = false;
      }

      if (pause_time >= 16000 && pause_time <= 18000)
      {
        ESP_LOGD(TAG, "Preamble - %i", pause_time);
        this->message_started = true;
        this->message_position = 0;
      }

      if (this->message_started)
      {
        switch (pause_time)
        {
        case 2000 ... 4900:
        {
          ESP_LOGD(TAG, "0 - %i", pause_time);
          this->message_bit_array[this->message_position] = 0;
          this->message_position++;
          break;
        }
        case 5000 ... 9000:
        {
          ESP_LOGD(TAG, "1 - %i", pause_time);
          this->message_bit_array[this->message_position] = 1;
          this->message_position++;
          break;
        }
        default:
        {
          break;
        }
        }
      }

      if (this->message_position == 18)
      {
        this->message_started = false;

        unsigned int message_code = binary_to_int(0, 6, this->message_bit_array);
        ESP_LOGD(TAG, "Message Code %u", message_code);
        unsigned int message_addr = binary_to_int(6, 8, this->message_bit_array);
        ESP_LOGD(TAG, "Message Addr %u", message_addr);
        int message_checksum = binary_to_int(14, 4, this->message_bit_array);

        int checksum = 0;
        checksum += __builtin_popcount(message_code);
        checksum += __builtin_popcount(message_addr);

        if (checksum == message_checksum)
        {
          this->message_code = message_code;
          this->message_addr = message_addr;
        }
        else
        {
          ESP_LOGD(TAG, "Incorrect checksum");
          this->message_code = -1;
        }
      }

      this->last_pause_time = now;
      s.pin_triggered = false;
    }

    void Simplebus2Component::register_listener(Simplebus2Listener *listener)
    {
      this->listeners_.push_back(listener);
    }

    void send_pwm()
    {
      ledcWrite(0, 50);
      delay(3);
      ledcWrite(0, 0);
    }

    void send_message(bool bitToSend)
    {
      if (bitToSend)
      {
        send_pwm();
        delay(6);
      }
      else
      {
        send_pwm();
        delay(3);
      }
    }

    void send_message_start()
    {
      send_pwm();
      delay(17);
    }

    void Simplebus2Component::send_command(Simplebus2Data data)
    {
      ESP_LOGD(TAG, "Sending command %i, address %i", data.command, data.address);

      this->rx_pin->detach_interrupt();

      int msgArray[18];
      int checksum = 0;

      checksum += __builtin_popcount(data.command);
      checksum += __builtin_popcount(data.address);
      Serial.print(checksum);
      Serial.println(" ");
      int_to_binary(data.command, 0, 6, msgArray);
      int_to_binary(data.address, 6, 8, msgArray);
      int_to_binary(checksum, 14, 4, msgArray);
      send_message_start();
      for (int i = 0; i < 18; i++)
      {
        send_message(msgArray[i]);
      }
      send_pwm();

      this->rx_pin->attach_interrupt(Simplebus2ComponentStore::gpio_intr, &this->store_, gpio::INTERRUPT_RISING_EDGE);
    }

    void Simplebus2Component::int_to_binary(unsigned int input, int start_pos, int no_of_bits, int *bits)
    {
      unsigned int mask = 1;
      int zeroedstart_pos = start_pos - 1;
      for (int i = start_pos; i < no_of_bits + start_pos; i++)
      {
        bits[i] = (input & (1 << (i - start_pos))) != 0;
      }
    }

    unsigned int Simplebus2Component::binary_to_int(int start_pos, int no_of_bits, int bin_array[])
    {
      unsigned int integer = 0;
      unsigned int mask = 1;
      for (int i = start_pos; i < no_of_bits + start_pos; i++)
      {
        if (bin_array[i])
        {
          integer |= mask;
        }
        mask = mask << 1;
      }
      return integer;
    }
  }
}
