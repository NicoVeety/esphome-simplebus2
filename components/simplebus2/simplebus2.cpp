#include "simplebus2.h"
#include "esphome/core/log.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/application.h"
#include <Arduino.h>

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

      this->high_freq_.start();
      this->store_.rx_pin = this->rx_pin->to_isr();

      this->rx_pin->attach_interrupt(Simplebus2ComponentStore::gpio_intr, &this->store_, gpio::INTERRUPT_RISING_EDGE);

      ESP_LOGCONFIG(TAG, "Setup for Simplebus2 complete");
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
        for (auto &listener : listeners_)
        {
          listener->trigger(this->message_code, this->message_addr);
        }
        this->message_code = -1;
      }
    }

    void IRAM_ATTR HOT Simplebus2ComponentStore::gpio_intr(Simplebus2ComponentStore *arg)
    {
      arg->pin_triggered = true;
    }

void Simplebus2Component::process_interrupt()
{
    auto &s = this->store_;
    unsigned long now = micros();
    unsigned long pause_time = now - this->last_pause_time;

    if (pause_time > 18000)
    {
        this->message_started = false;
    }
    else if (pause_time >= 16000)
    {
        this->message_started = true;
        this->message_position = 0;
    }

    if (this->message_started)
    {
        if (pause_time >= 2000 && pause_time <= 4900)
        {
            this->message_bit_array[this->message_position++] = 0;
        }
        else if (pause_time >= 5000 && pause_time <= 9000)
        {
            this->message_bit_array[this->message_position++] = 1;
        }
    }

    if (this->message_position == 18)
    {
        this->message_started = false;

        unsigned int message_code = binary_to_int(0, 6, this->message_bit_array);
        unsigned int message_addr = binary_to_int(6, 8, this->message_bit_array);
        int message_checksum = binary_to_int(14, 4, this->message_bit_array);

        int checksum = __builtin_popcount(message_code) + __builtin_popcount(message_addr);

        if (checksum == message_checksum)
        {
            this->message_code = message_code;
            this->message_addr = message_addr;

            if (this->message_code != this->last_logged_message_code || this->message_addr != this->last_logged_message_addr)
            {
                ESP_LOGI(TAG, "Received command %i, address %i", message_code, message_addr);
                this->last_logged_message_code = this->message_code;
                this->last_logged_message_addr = this->message_addr;
            }
        }
        else
        {
            ESP_LOGW(TAG, "Incorrect checksum");
            ESP_LOGI(TAG, "Cmd %i, Addr %i, Cksm %i", message_code, message_addr, message_checksum);
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
      send_pwm();
      delay(bitToSend ? 6 : 3);
    }

    void send_message_start()
    {
      send_pwm();
      delay(17);
    }

    void Simplebus2Component::send_command(Simplebus2Data data)
    {
      ESP_LOGI(TAG, "Send command %i, address %i", data.command, data.address);

      this->rx_pin->detach_interrupt();

      int msgArray[18];
      int checksum = __builtin_popcount(data.command) + __builtin_popcount(data.address);

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
      for (int i = start_pos; i < start_pos + no_of_bits; i++)
      {
        bits[i] = (input >> (i - start_pos)) & 1;
      }
    }

    unsigned int Simplebus2Component::binary_to_int(int start_pos, int no_of_bits, int bin_array[])
    {
      unsigned int result = 0;
      for (int i = start_pos; i < start_pos + no_of_bits; i++)
      {
        result |= (bin_array[i] << (i - start_pos));
      }
      return result;
    }

  }
}