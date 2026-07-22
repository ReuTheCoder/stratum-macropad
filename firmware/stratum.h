#pragma once
#include "quantum.h"

typedef enum {
    SSTATE_NORMAL,
    SSTATE_HOLD_CONFIRM,
    SSTATE_MAIN_MENU,
    SSTATE_LAYER_MENU,
    SSTATE_BRIGHTNESS_MENU,
    SSTATE_SLEEP_MENU,
    SSTATE_SLEEPING
} settings_state_t;

extern settings_state_t settings_state;
extern uint32_t hold_start_time;
extern uint32_t last_activity_time;
extern uint8_t main_menu_index;
extern uint8_t user_brightness_pct;

void apply_rgb_for_state(void);
void enter_settings_mode(void);
void exit_settings_mode(void);
void settings_encoder_nav(bool clockwise);
void handle_settings_select(void);
void touch_activity(void);