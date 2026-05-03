#include "ir_handler.h"

void processIR() {
    if (!irrecv.decode(&irResults)) return;

    if (state == STATE_SETUP) {
        if (irResults.decode_type != decode_type_t::UNKNOWN) {
            detectedProtocol = irResults.decode_type;
            detectedBits     = irResults.bits;
            isAcSupported    = IRac::isProtocolSupported(irResults.decode_type);
            Serial.printf("IR detected: %s, %u bits, AC=%d\n",
                          typeToString(irResults.decode_type).c_str(),
                          irResults.bits, isAcSupported);
        }
    } else if (state == STATE_NORMAL) {
        decode_type_t saved = (decode_type_t)prefs.getUChar("proto", 0);
        if (irResults.decode_type == saved) {
            Serial.println("Manual remote use detected, syncing state");
        }
    }

    irrecv.resume();
}
