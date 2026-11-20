#include QMK_KEYBOARD_H
#include "raw_hid.h"

#define H(k) HYPR(KC_##k)
#define CMD_STATUS 0x42 // host -> board: {0x42, h, s, v} sets a global solid color, not saved to EEPROM

// Fallback defaults only. Load via/creator_micro.agents.layout.json in VIA for the real layout, macros included.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    LAYOUT(
        H(R),     H(P),    H(S),    KC_MPLY,
        A(KC_2),  H(L),    H(F),    H(Z),
        KC_Y,     KC_N,    KC_RGHT, KC_ESC,
        A(KC_1),  A(KC_SPC), KC_ENT, TO(1)
    ),
    LAYOUT(
        KC_MPLY,  KC_TRNS, KC_TRNS, KC_NO,
        KC_HOME,  KC_TRNS, KC_TRNS, KC_TRNS,
        KC_END,   KC_TRNS, KC_TRNS, KC_TRNS,
        UG_TOGG,  KC_F13,  KC_ENT,  TO(2)
    ),
    LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, TO(3)
    ),
    LAYOUT(
        KC_TRNS, QK_BOOT, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, TO(0)
    ),
};

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    { ENCODER_CCW_CW(H(LEFT), H(RIGHT)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(C(KC_Z), C(KC_Y)) },
    { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(C(KC_Z), C(KC_Y)) },
    { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(C(KC_Z), C(KC_Y)) },
};

bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] != CMD_STATUS) return false;
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(data[1], data[2], data[3]);
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_sethsv_noeeprom(data[1], data[2], data[3]);
    host_raw_hid_send(data, length); // VIA convention: echo the packet back
    return true;
}

// Layer indicator LEDs: one lit for layers 1-3, none on layer 0
layer_state_t layer_state_set_user(layer_state_t state) {
    layer_state_cmp(state, 1) ? work_louder_micro_led_1_on() : work_louder_micro_led_1_off();
    layer_state_cmp(state, 2) ? work_louder_micro_led_2_on() : work_louder_micro_led_2_off();
    layer_state_cmp(state, 3) ? work_louder_micro_led_3_on() : work_louder_micro_led_3_off();
    return state;
}

// Indicator LED brightness 0-5, shown in VIA's Lighting > Indicators (channel 0, id 1)
static uint8_t led_level;
static void led_apply(void) { work_louder_micro_led_all_set(led_level * 255 / 5); }

void eeconfig_init_user(void) { eeconfig_update_user(1 | 0x80); }

void matrix_init_user(void) {
    uint32_t v = eeconfig_read_user();
    led_level = (v & 0x80) ? (v & 0x7f) : 1; // unset EEPROM -> default 1
    if (led_level > 5) led_level = 1;
    led_apply();
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    if (data[1] != id_custom_channel || data[2] != 1) { data[0] = id_unhandled; return; }
    switch (data[0]) {
        case id_custom_set_value: led_level = data[3] > 5 ? 5 : data[3]; led_apply(); break;
        case id_custom_get_value: data[3] = led_level; break;
        case id_custom_save:      eeconfig_update_user(led_level | 0x80); break;
    }
}

// the kb-level wake animation turns the indicators off, so relight the current layer
void suspend_wakeup_init_user(void) {
    layer_state_set_user(layer_state);
    led_apply();
}
