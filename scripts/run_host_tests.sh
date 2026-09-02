#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-g++}"
FLAGS=(-std=c++17 -I"$ROOT/include" -I"$ROOT/src" -I"$ROOT/test/host_unity"
       -DSWARM_FIRMWARE_VERSION=1 -DSWARM_PROTOCOL_VERSION=1
       -DSWARM_MAX_NODES=6 -DSWARM_HEARTBEAT_MS=500 -DUNIT_TEST)
COMMON=(
  "$ROOT/src/transport/packet_codec.cpp"
  "$ROOT/src/field/field_state.cpp"
  "$ROOT/src/field/field_delta.cpp"
  "$ROOT/src/field/field_tick.cpp"
  "$ROOT/src/field/field_system.cpp"
  "$ROOT/src/systems/diffusion.cpp"
  "$ROOT/src/systems/information_decay.cpp"
  "$ROOT/src/systems/aggregation.cpp"
  "$ROOT/src/swarm/election.cpp"
  "$ROOT/src/swarm/membership.cpp"
  "$ROOT/src/swarm/discovery.cpp"
)

fail=0
run() {
  local name="$1"
  local src="$2"
  local out
  out="$(mktemp)"
  echo "== $name"
  "$CXX" "${FLAGS[@]}" "$src" "${COMMON[@]}" -o "$out"
  if ! "$out"; then
    fail=1
  fi
  rm -f "$out"
}

run test_protocol "$ROOT/test/test_protocol/test_protocol.cpp"
run test_field "$ROOT/test/test_field/test_field.cpp"
run test_election "$ROOT/test/test_election/test_election.cpp"
run test_join "$ROOT/test/test_join/test_join.cpp"
run test_determinism "$ROOT/test/test_determinism/test_determinism.cpp"

exit "$fail"
