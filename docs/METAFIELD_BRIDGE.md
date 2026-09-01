# MetaField bridge (not in this milestone)

Only after the wireless swarm is deterministic and tested:

```
C3 FieldState
       ↓
MetaField FieldView
       ↓
FieldDelta
       ↓
field-bus message
```

Do not redefine the CAN-FD protocol inside this repository. `field-bus` stays the physical CAN-FD contract.
