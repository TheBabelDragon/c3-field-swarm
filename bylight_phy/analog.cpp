#include "analog.h"

static BylightRxPath g_path = BYLIGHT_RX_PASSIVE;
static BylightAnalogMeasured g_measured = {};

BylightAnalogDesign bylight_analog_design() {
    BylightAnalogDesign d;
    d.r_pd_top_ohm = BYLIGHT_R_PD_TOP_OHM;
    d.r_pd_bottom_ohm = BYLIGHT_R_PD_BOTTOM_OHM;
    d.r_vref_top_ohm = BYLIGHT_R_VREF_TOP_OHM;
    d.r_vref_bottom_ohm = BYLIGHT_R_VREF_BOTTOM_OHM;
    d.r_gate_ohm = BYLIGHT_R_GATE_OHM;
    d.r_gate_pd_ohm = BYLIGHT_R_GATE_PD_OHM;
    d.r_led_ohm = BYLIGHT_R_LED_OHM;
    d.vcc_rx_v = 3.3f;
    d.vcc_led_v = 5.0f;
    d.vref_design_v = 1.65f;
    d.rx_ana_idle_design_v = 3.3f * 22000.0f / (330000.0f + 22000.0f);
    return d;
}

BylightRxPath bylight_analog_path() { return g_path; }
void bylight_analog_set_path(BylightRxPath path) { g_path = path; }
void bylight_analog_set_measured(const BylightAnalogMeasured& m) { g_measured = m; }
BylightAnalogMeasured bylight_analog_measured() { return g_measured; }

const char* bylight_node_name(BylightNode node) {
    switch (node) {
        case NODE_3V3: return "TP_3V3";
        case NODE_GND: return "TP_GND";
        case NODE_RX_ANA: return "TP_RX_ANA";
        case NODE_VREF: return "TP_VREF";
        case NODE_RX_CONDITIONED: return "TP_RX_CONDITIONED";
        case NODE_CMP_OUT: return "TP_CMP_OUT";
        case NODE_LED_ANODE: return "NODE_LED_ANODE";
        case NODE_LED_CATHODE: return "NODE_LED_CATHODE";
        case NODE_DRAIN: return "NODE_DRAIN";
        default: return "UNKNOWN";
    }
}

const char* bylight_node_expect(BylightNode node, const char* condition) {
    if (node == NODE_RX_ANA) return "passive photodiode node — always probeable";
    if (node == NODE_CMP_OUT) return "MCP6561 OUT, 3.3 V CMOS";
    if (node == NODE_VREF) return "measure against RX_ANA before treating 1.65 V as optimal";
    (void)condition;
    return "";
}

bool bylight_nodes_electrically_isolated_ok() {
    return NODE_RX_ANA != NODE_VREF && NODE_RX_ANA != NODE_LED_ANODE &&
           NODE_RX_ANA != NODE_LED_CATHODE && NODE_RX_ANA != NODE_DRAIN;
}
