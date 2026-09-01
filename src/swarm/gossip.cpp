#include "swarm/protocol.h"
#include "field/field_state.h"

bool digest_mismatch(const StateDigestPayload& remote,
                     uint32_t local_version,
                     uint32_t local_checksum) {
    if (remote.state_version != local_version) {
        return true;
    }
    if (remote.checksum != local_checksum) {
        return true;
    }
    return false;
}
