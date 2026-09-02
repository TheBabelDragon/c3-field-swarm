#pragma once

#include <stdint.h>
#include "phy_config.h"

enum BylightNode : uint8_t {
    NODE_3V3 = 0,
    NODE_GND,
    NODE_RX_ANA,
    NODE_VREF,
    NODE_RX_CONDITIONED,
    NODE_CMP_OUT,
    NODE_LED_ANODE,
    NODE_LED_CATHODE,
    NODE_DRAIN,
    NODE_COUNT
};

struct BylightAnalogDesign {
    uint32_t r_pd_top_ohm;
    uint32_t r_pd_bottom_ohm;
    uint32_t r_vref_top_ohm;
    uint32_t r_vref_bottom_ohm;
    uint32_t r_gate_ohm;
    uint32_t r_gate_pd_ohm;
    uint32_t r_led_ohm;
    float vcc_rx_v;
    float vcc_led_v;
    float vref_design_v;
    float rx_ana_idle_design_v;
};

struct BylightAnalogMeasured {
    bool valid;
    float rx_ana_idle_v;
    float rx_ana_illuminated_v;
    float rx_ana_modulation_vpp;
    float rx_ana_ambient_min_v;
    float rx_ana_ambient_max_v;
    float vref_v;
    float cmp_out_idle_v;
    float cmp_out_illuminated_v;
};

BylightAnalogDesign bylight_analog_design();
BylightRxPath bylight_analog_path();
void bylight_analog_set_path(BylightRxPath path);
void bylight_analog_set_measured(const BylightAnalogMeasured& m);
BylightAnalogMeasured bylight_analog_measured();
const char* bylight_node_name(BylightNode node);
const char* bylight_node_expect(BylightNode node, const char* condition);
bool bylight_nodes_electrically_isolated_ok();
