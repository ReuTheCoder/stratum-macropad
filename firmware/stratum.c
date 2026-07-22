#include "quantum.h"
#include "stratum.h"

// ---- Shared state ----
settings_state_t settings_state = SSTATE_NORMAL;
uint32_t hold_start_time = 0;
uint8_t main_menu_index = 0;

#define MAIN_MENU_COUNT 4
const char *main_menu_items[MAIN_MENU_COUNT] = {
    "Layer",
    "Brightness",
    "Sleep",
    "Exit"
};

uint8_t layer_menu_index = 0;          
#define LAYER_MENU_COUNT 6  

uint8_t user_brightness_pct = 100;          // default full brightness
uint8_t brightness_menu_index = 10;         // 0-10, representing 0%-100%
#define BRIGHTNESS_MENU_COUNT 11

uint8_t sleep_menu_index = 0;               
#define SLEEP_MENU_COUNT 8
const uint16_t sleep_timeout_minutes[6] = {0, 1, 5, 10, 15, 30}; // index 0-5
uint16_t idle_timeout_min = 0;              // 0 = disabled

uint32_t last_activity_time = 0; 

uint8_t scale_brightness(uint8_t base_v, uint8_t pct) {
    return (uint8_t)(((uint16_t)base_v * pct) / 100);
}           

// ---- Layer colors (HSV) - solid backlight per layer ----
typedef struct {
    uint8_t h;
    uint8_t s;
    uint8_t v;
} layer_color_t;

const layer_color_t layer_colors[] = {
    [0] = {155, 180, 200}, // Main - light blue
    [1] = {0,     0, 220}, // Productivity - cool white
    [2] = {200, 255, 200}, // Design - bright purple
    [3] = {0,   255, 220}, // Scientific - bright red
    [4] = {85,  255, 200}  // Media - bright green
};

void apply_rgb_for_state(void) {
    switch (settings_state) {
        case SSTATE_HOLD_CONFIRM:
            rgblight_mode(RGBLIGHT_MODE_BREATHING);
            return;
        case SSTATE_SLEEPING:                              
            rgblight_disable_noeeprom();
            return;
        case SSTATE_NORMAL: {
            uint8_t layer = get_highest_layer(layer_state);
            if (layer < (sizeof(layer_colors) / sizeof(layer_colors[0]))) {
                rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
                rgblight_sethsv_noeeprom(
                    layer_colors[layer].h,
                    layer_colors[layer].s,
                    scale_brightness(layer_colors[layer].v, user_brightness_pct)
                );
            }
            return;
        }
        case SSTATE_LAYER_MENU:                              
            if (layer_menu_index < 5) {
                rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
                rgblight_sethsv_noeeprom(
                    layer_colors[layer_menu_index].h,
                    layer_colors[layer_menu_index].s,
                    layer_colors[layer_menu_index].v
                );
            } else {
                rgblight_mode(RGBLIGHT_MODE_RAINBOW_MOOD);
            }
            return;
         case SSTATE_BRIGHTNESS_MENU: {                     
            uint8_t layer = get_highest_layer(layer_state);
            uint8_t pct = brightness_menu_index * 10;
            rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
            rgblight_sethsv_noeeprom(
                layer_colors[layer].h,
                layer_colors[layer].s,
                scale_brightness(layer_colors[layer].v, pct)
            );
            return;
        }
        default:
            // Any settings menu/submenu - rainbow
            rgblight_mode(RGBLIGHT_MODE_RAINBOW_MOOD);
            return;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    apply_rgb_for_state();
    return state;
}

// ---- Settings-mode hold detection ----
// Matrix positions: K2 (row0,col1), K4 (row1,col0), K8 (row2,col1)
#define COMBO_K2_ROW 0
#define COMBO_K2_COL 1
#define COMBO_K4_ROW 1
#define COMBO_K4_COL 0
#define COMBO_K8_ROW 2
#define COMBO_K8_COL 1

#define SETTINGS_HOLD_MS 3000

bool combo_keys_all_held(void) {
    return matrix_is_on(COMBO_K2_ROW, COMBO_K2_COL) &&
           matrix_is_on(COMBO_K4_ROW, COMBO_K4_COL) &&
           matrix_is_on(COMBO_K8_ROW, COMBO_K8_COL);
}

void touch_activity(void) {
    last_activity_time = timer_read32();
}

void enter_settings_mode(void) {
    settings_state = SSTATE_MAIN_MENU;
    main_menu_index = 0;                
    apply_rgb_for_state();
}

void exit_settings_mode(void) {
    settings_state = SSTATE_NORMAL;
    apply_rgb_for_state();
}

void enter_sleep(void) {
    settings_state = SSTATE_SLEEPING;
    apply_rgb_for_state();
#ifdef OLED_ENABLE
    oled_off();
#endif
}

void wake_from_sleep(void) {
    settings_state = SSTATE_NORMAL;
    touch_activity();
    apply_rgb_for_state();
#ifdef OLED_ENABLE
    oled_on();
#endif
}

void settings_encoder_nav(bool clockwise) {
    switch (settings_state) {
        case SSTATE_MAIN_MENU:
            if (clockwise) {
                main_menu_index = (main_menu_index + 1) % MAIN_MENU_COUNT;
            } else {
                main_menu_index = (main_menu_index + MAIN_MENU_COUNT - 1) % MAIN_MENU_COUNT;
            }
            break;
        case SSTATE_LAYER_MENU:                              
            if (clockwise) {
                layer_menu_index = (layer_menu_index + 1) % LAYER_MENU_COUNT;
            } else {
                layer_menu_index = (layer_menu_index + LAYER_MENU_COUNT - 1) % LAYER_MENU_COUNT;
            }
            apply_rgb_for_state();   // live-preview layer color as you scroll through layer sub-menu
            break;
        case SSTATE_BRIGHTNESS_MENU:                        
            if (clockwise) {
                brightness_menu_index = (brightness_menu_index + 1) % BRIGHTNESS_MENU_COUNT;
            } else {
                brightness_menu_index = (brightness_menu_index + BRIGHTNESS_MENU_COUNT - 1) % BRIGHTNESS_MENU_COUNT;
            }
            apply_rgb_for_state();
            break;

        case SSTATE_SLEEP_MENU:                             
            if (clockwise) {
                sleep_menu_index = (sleep_menu_index + 1) % SLEEP_MENU_COUNT;
            } else {
                sleep_menu_index = (sleep_menu_index + SLEEP_MENU_COUNT - 1) % SLEEP_MENU_COUNT;
            }
            break;
        default:
            break; // Submenu navigation (Layer/Brightness/Sleep) arrives in 6d/6e
    }
}

void handle_settings_select(void) {
    switch (settings_state) {
        case SSTATE_MAIN_MENU:
            switch (main_menu_index) {
                case 0: // Layer
                    settings_state = SSTATE_LAYER_MENU;
                    layer_menu_index = get_highest_layer(layer_state);
                    apply_rgb_for_state();
                    break;
                case 1: // Brightness - submenu logic arrives in 6e
                    settings_state = SSTATE_BRIGHTNESS_MENU;
                    apply_rgb_for_state();
                    break;
                case 2: // Sleep - submenu logic arrives in 6e
                    settings_state = SSTATE_SLEEP_MENU;
                    apply_rgb_for_state();
                    break;
                case 3: // Exit
                    exit_settings_mode();
                    break;
            }
            break;

        case SSTATE_LAYER_MENU:
            if (layer_menu_index < 5) {
                layer_move(layer_menu_index);   // actually switch to the chosen layer
                exit_settings_mode();           // fully exit, per your Option A design
            } else {
                settings_state = SSTATE_MAIN_MENU; // Cancel - back to main menu
                apply_rgb_for_state();
            }
            break;

        case SSTATE_BRIGHTNESS_MENU:                        
            user_brightness_pct = brightness_menu_index * 10;
            settings_state = SSTATE_MAIN_MENU;
            apply_rgb_for_state();
            break;

        case SSTATE_SLEEP_MENU:                             
            if (sleep_menu_index <= 5) {
                idle_timeout_min = sleep_timeout_minutes[sleep_menu_index];
                settings_state = SSTATE_MAIN_MENU;
                apply_rgb_for_state();
            } else if (sleep_menu_index == 6) { // Sleep Now
                enter_sleep();
            } else { // Cancel
                settings_state = SSTATE_MAIN_MENU;
                apply_rgb_for_state();
            }
            break;

        default:
            break; // Brightness/Sleep select logic arrives in 6e
    }
}

void settings_matrix_scan(void) {
    switch (settings_state) {
        case SSTATE_NORMAL:
            if (combo_keys_all_held()) {
                hold_start_time = timer_read32();
                settings_state = SSTATE_HOLD_CONFIRM;
                apply_rgb_for_state();
            }
            break;

        case SSTATE_HOLD_CONFIRM:
            if (!combo_keys_all_held()) {
                settings_state = SSTATE_NORMAL;
                apply_rgb_for_state();
            } else if (timer_elapsed32(hold_start_time) >= SETTINGS_HOLD_MS) {
                enter_settings_mode();
            }
            break;

        default:
            break; // Menu navigation logic arrives in Phase 6c
    }
}

void matrix_scan_user(void) {
    settings_matrix_scan();

    if (settings_state == SSTATE_NORMAL &&
        idle_timeout_min > 0 &&
        timer_elapsed32(last_activity_time) >= (uint32_t)idle_timeout_min * 60000) {
        enter_sleep();
    }
}

// ---- Keyboard-level init ----
void keyboard_post_init_kb(void) {
    keyboard_post_init_user();
}

// ---- OLED ----
#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;
}

const char *layer_name(uint8_t layer) {
    switch (layer) {
        case 0: return "Main";
        case 1: return "Productivity";
        case 2: return "Design";
        case 3: return "Scientific";
        case 4: return "Media";
        default: return "Unknown";
    }
}

void draw_layer_screen(void) {
    oled_clear();
    uint8_t layer = get_highest_layer(layer_state);
    const char *name = layer_name(layer);

    uint8_t len = 0;
    while (name[len] != '\0') len++;

    uint8_t col = (21 > len) ? (21 - len) / 2 : 0;
    oled_set_cursor(col, 1);
    oled_write(name, false);
}

void draw_hold_progress_screen(void) {
    oled_clear();

    const char *msg = "Hold for settings";
    uint8_t len = 0;
    while (msg[len] != '\0') len++;
    uint8_t col = (21 > len) ? (21 - len) / 2 : 0;
    oled_set_cursor(col, 0);
    oled_write(msg, false);

    uint32_t elapsed = timer_elapsed32(hold_start_time);
    if (elapsed > SETTINGS_HOLD_MS) elapsed = SETTINGS_HOLD_MS;

    uint8_t bar_width = 18;
    uint8_t filled = (uint8_t)((elapsed * bar_width) / SETTINGS_HOLD_MS);

    char bar[19];
    for (uint8_t i = 0; i < bar_width; i++) {
        bar[i] = (i < filled) ? '#' : '-';
    }
    bar[bar_width] = '\0';

    oled_set_cursor(1, 2);
    oled_write(bar, false);
}

void draw_main_menu_screen(void) {     
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write("Settings", false);
    bool blink_on = (timer_read32() % 1000) < 500;
    oled_set_cursor(0, 2);
    oled_write(blink_on ? ">" : " ", false);
    oled_write(main_menu_items[main_menu_index], false);
}

void draw_layer_menu_screen(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write("Layer", false);

    bool blink_on = (timer_read32() % 1000) < 500;
    oled_set_cursor(0, 2);
    oled_write(blink_on ? ">" : " ", false);

    if (layer_menu_index < 5) {
        oled_write(layer_name(layer_menu_index), false);
    } else {
        oled_write("Cancel", false);
    }
}

void draw_brightness_menu_screen(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write("Brightness", false);

    bool blink_on = (timer_read32() % 1000) < 500;
    oled_set_cursor(0, 2);
    oled_write(blink_on ? ">" : " ", false);

    char buf[5];
    uint8_t pct = brightness_menu_index * 10;
    snprintf(buf, sizeof(buf), "%3d%%", pct);
    oled_write(buf, false);
}

void draw_sleep_menu_screen(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write("Sleep", false);

    bool blink_on = (timer_read32() % 1000) < 500;
    oled_set_cursor(0, 2);
    oled_write(blink_on ? ">" : " ", false);

    if (sleep_menu_index <= 5) {
        char buf[16];
        if (sleep_timeout_minutes[sleep_menu_index] == 0) {
            oled_write("Off", false);
        } else {
            snprintf(buf, sizeof(buf), "%d min", sleep_timeout_minutes[sleep_menu_index]);
            oled_write(buf, false);
        }
    } else if (sleep_menu_index == 6) {
        oled_write("Sleep Now", false);
    } else {
        oled_write("Cancel", false);
    }
}

bool oled_task_user(void) {
    switch (settings_state) {
        case SSTATE_HOLD_CONFIRM:
            draw_hold_progress_screen();
            break;
        case SSTATE_NORMAL:
            draw_layer_screen();
            break;
        case SSTATE_MAIN_MENU:
            draw_main_menu_screen();
            break;
        case SSTATE_LAYER_MENU:                    
            draw_layer_menu_screen();
            break;
        case SSTATE_BRIGHTNESS_MENU:                
            draw_brightness_menu_screen();
            break;
        case SSTATE_SLEEP_MENU:                     
            draw_sleep_menu_screen();
            break;
        case SSTATE_SLEEPING:                       
            break; // screen is off, nothing to draw
        default:
            oled_clear();
            oled_set_cursor(0, 1);
            oled_write("Submenu...", false);
            break;
    }
    return false;
}

#endif