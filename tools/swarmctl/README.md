# swarmctl

Host-side serial operator for a running C3 node.

The swarm does not require this tool. Six boards on a table form the fleet by themselves.

```
python3 tools/swarmctl/swarmctl.py discover
python3 tools/swarmctl/swarmctl.py --port /dev/ttyACM0 status
python3 tools/swarmctl/swarmctl.py nodes
python3 tools/swarmctl/swarmctl.py state
python3 tools/swarmctl/swarmctl.py tick
python3 tools/swarmctl/swarmctl.py inject info 1.0
python3 tools/swarmctl/swarmctl.py logs
```

Requires `pyserial` only when talking to hardware.
