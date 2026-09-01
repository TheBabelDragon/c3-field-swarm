# Hardware

Milestone 1 requires only:

- ESP32-C3 Super Mini
- USB power

No external sensors, CAN transceiver, motor, LED strip, or ADC.

## PlatformIO target

`esp32-c3-devkitm-1` on `espressif32@6.9.0`, Arduino framework.

Most Super Mini boards enumerate as USB-CDC. If upload fails, hold BOOT, tap RESET, release BOOT.

```
pio run
pio run -t upload
pio device monitor
```

## Six-node desk layout

Provision logical identities 01–06. These are test names, not roles.

Assign with the serial console:

```
id 1
id 2
...
id 6
```

The board persists `node_id` in NVS and reboots.

Capabilities are advertised separately (`CAP_SENSOR`, `CAP_LED`, …). A node may hold several. None are required for the first firmware.

## Radio

ESP-NOW, Wi-Fi station mode, channel 1. Boards must share the channel. Keep them on the same table for the first fleet test.

## Later adapters

Touch, Hall, temperature, light, IMU, audio, WS2812, motor, ADC, I2C, SPI all attach *above* the swarm core. Do not compile them into transport or membership.
