# simplebus2ESPHome

This is a special personilized Version of "se-bastiaan"'s "esphome-simplebus2" Project Based on the Hardware from "Elektroarzt".
-I2C Potentiometers are removed and with it the Number Variable since i somehow killed mine and had to find a workaround Instead of U3 MCP4017 I hardwired a 4.7k resistor to D1 and GND and used the Bypass Connector (removed R11 and placed R1)
-Fix Auto reset Timer for binary Sensor
-Modify Logging
-other (i may have forgotten)

Own usage as an example:
```
external_components:
  - source: github://nicoveety/esphome-simplebus2

simplebus2:

text_sensor:
  - platform: simplebus2

button:
  - platform: template
    name: "Open Door"
    on_press:
      - simplebus2.send:
          command: 16
          address: 2
  - platform: template
    name: "RingOG_R"
    on_press:
      - simplebus2.send:
          command: 50
          address: 4
  - platform: template
    name: "RingOG_RFloor"
    on_press:
      - simplebus2.send:
          command: 21
          address: 4
#  - platform: template
#    name: "Blink"
#    on_press:
#      - simplebus2.send:
#          command: 51
#          address: 2
#  - platform: template
#    name: "Blink Aus"
#    on_press:
#      - simplebus2.send:
#          command: 52
#          address: 2
# Performance Monitoring Sensors
#sensor:
  # Monitor Free Heap Memory
#  - platform: template
#    name: "ESP32 Free Heap"
#    lambda: |-
#      return (float) ESP.getFreeHeap();
#    unit_of_measurement: "bytes"
  
  # Monitor Wi-Fi Signal Strength
#  - platform: wifi_signal
#    name: "ESP32 Wi-Fi Signal"
#    update_interval: 60s

  # Monitor Uptime
#  - platform: uptime
#    name: "ESP32 Uptime"
#    update_interval: 60s

#switch:
#  - platform: gpio
#    pin: GPIO04
#    name: "LEDTEST"

binary_sensor:
#  - platform: gpio
#    pin: GPIO08
#    name: "ButtonTest"
  - platform: simplebus2
    command: 50
    address: 2
    name: Klingel Aussen
    auto_off: 30s
  - platform: simplebus2
    command: 21
    address: 2
    name: Klingel Innen
    auto_off: 30s
```

Binary sensor
===================

You can configure binary sensors will trigger when a particular combination of command and address is received from the bus.

You can also set only the address, in this case the default command is 50, which occurs when a call is made from the outside intercom to the inside intercom.

Configuration examples:

	binary_sensor:
	  - platform: simplebus2
	    address: 2
	  - platform: simplebus2
	    command: 29
	    address: 1
	    name: Internal Door opened
	    auto_off: 60s

- **address** (**Required**, int): The address that when received sets the sensor to on .
- **command** (*Optional*, int): The command that when received sets the sensor to on . Defaults to  `50`.
- **auto_off** (*Optional*,  [Time](https://esphome.io/guides/configuration-types#config-time)):  The time after which the sensor returns to off. If set to `0s` the sensor once it goes on, it stays there until it is turned off by an automation. Defaults to  `30s`.
- **icon** (*Optional*, icon): Manually set the icon to use for the sensor in the frontend. Default to `mdi:doorbell`.
- **id** (*Optional*, string): Manually specify the ID for code generation.
- **name** (*Optional*, string): The name for the sensor. Default to `Incoming call`.

    Note:
    If you have friendly_name set for your device and you want 
    the sensor to use that name, you can set `name: None`.

Text sensor
===================

You can configure a text sensor that will output the last received command on the bus.

Configuration example:

	text_sensor:
	  - platform: simplebus2

Send a command
==================
To send commands to the bus, the following action is available:

	- simplebus2.send:
	    command: 16
	    address: 1

- **command** (**Required**, int)
- **address** (**Required**, int)

### Button:
The action can be easily inserted into a button type entity:

	button:
	  - platform: template
	    name: Open Door
	    on_press:
	      - simplebus2.send:
	          command: 16
	          address: 5

### Sending multiple commands:
There are some special configurations that require sending 2 or more commands consecutively on the bus.
In this case, a delay of at least 200ms must be inserted between the commands (one command takes about 180ms to be sent)

	- simplebus2.send:
	    command: 29
	    address: 1
	- delay: 200ms
	- simplebus2.send:
	    command: 16
	    address: 1
