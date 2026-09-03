# MetaField bridge

C3 nodes stay ESP-NOW among themselves. The host sees the fleet through serial.

```
ESP32-C3 fleet  --ESP-NOW-->  any USB C3  --C3JSON serial-->  metafield-engine/scripts/c3-bridge.py
                                                                  |
                                                                  v
                                                       /tmp/metafield/csi.jsonl
                                                                  |
                                                                  v
                                                             hello_view / World
```

One plugged-in node is enough for a census: membership + neighbor FieldState ride in `C3JSON`.
Plug in every C3 you have and the bridge opens all of them, then dedups by `node_id`.

Existing `[C3]` human lines stay. `C3JSON {\u2026}` is the machine line.
