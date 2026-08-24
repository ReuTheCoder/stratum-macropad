// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H


#include "stratum.h"

void keyboard_post_init_user(void) {
    set_unicode_input_mode(UNICODE_MODE_WINCOMPOSE);
    touch_activity(); 
    apply_rgb_for_state();
}

enum layers {
    _MAIN = 0,
    _PRODUCTIVITY,
    _DESIGN,
    _SCIENTIFIC,
    _MEDIA
};

enum custom_keycodes {
    CT_SPOTIFY = SAFE_RANGE,
    CT_YOUTUBE,
    CT_LIBBY,
    CT_DEGREE,
    CT_PLUSMINUS,
    CT_TIMES,
    CT_SQRT,
    CT_PI,
    CT_SIGMA,
    CT_SUPER,
    CT_SUB,
    CT_LATEX
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* Layer 0 - Main */
    [_MAIN] = LAYOUT_ortho_3x3(
        LCTL(KC_S),           KC_ESC,                 KC_F5,
        LGUI(KC_E),           LGUI(KC_I),              LCTL(LSFT(KC_ESC)),
        LGUI(KC_DOT),         LGUI(LSFT(KC_S)),        KC_CALCULATOR
    ),
    /* K1 Save | K4 X (close/escape) | K7 Refresh
     * K2 File Explorer | K5 Settings | K8 Task Manager
     * K3 Emoji Panel | K6 Screenshot | K9 Calculator
     * NOTE: K4 interpreted as closing/escaping current context (KC_ESC) -
     * change to LALT(KC_F4) for "closing window" instead. */

    /* Layer 1 - Productivity */
    [_PRODUCTIVITY] = LAYOUT_ortho_3x3(
        LCTL(LALT(KC_C)),    KC_F7,               LCTL(KC_SPC),
        LCTL(KC_L),          LCTL(KC_E),          LCTL(KC_R),
        LCTL(KC_F),          LCTL(KC_K),          LCTL(KC_P)
    ),
    /* K1 Citations* | K4 Spell Check | K7 Clear Formatting
     * K2 Align Left | K5 Center | K8 Align Right
     * K3 Find | K6 Insert Link | K9 Print
     * *Citations -> LCTL+LALT+C;
     * check pecific app (Word/Google Docs/Zotero) for its actual shortcut and UPDATE */

    /* Layer 2 - Design */
    [_DESIGN] = LAYOUT_ortho_3x3(
        LCTL(KC_Z),          LCTL(KC_Y),              LCTL(KC_D),
        LCTL(KC_G),          LCTL(LSFT(KC_G)),        LCTL(KC_RBRC),
        LCTL(KC_EQL),        LCTL(KC_MINS),           LSFT(KC_1)
    ),
    /* K1 Undo | K4 Redo | K7 Duplicate
     * K2 Group | K5 Ungroup | K8 Bring Forward
     * K3 Zoom In | K6 Zoom Out | K9 Fit Screen
     * Fit Screen (Shift+1) matches Figma */

    /* Layer 3 - Scientific */
    [_SCIENTIFIC] = LAYOUT_ortho_3x3(
        CT_DEGREE,           CT_PLUSMINUS,        CT_TIMES,
        CT_SQRT,             CT_PI,               CT_SIGMA,
        CT_SUPER,            CT_SUB,              CT_LATEX
    ),
    /* K1 ° | K4 ± | K7 ×
     * K2 √ | K5 π | K8 Σ
     * K3 Superscript* | K6 Subscript* | K9 LaTeX $
     * *Superscript/Subscript send Ctrl+Shift+= / Ctrl+= (Word/Docs convention) */

    /* Layer 4 - Media */
    [_MEDIA] = LAYOUT_ortho_3x3(
        KC_MPLY,             KC_MPLY,             KC_MSTP,
        KC_MPRV,             KC_MNXT,             KC_MUTE,
        CT_SPOTIFY,          CT_YOUTUBE,          CT_LIBBY
    )
    /* K1 Play | K4 Pause (same key toggles) | K7 Stop
     * K2 Previous | K5 Next | K8 Mute
     * K3 Spotify | K6 YouTube | K9 Libby */
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;

    
    int8_t combo_idx = -1;
    if      (row == 0 && col == 1) combo_idx = 0; // K2
    else if (row == 1 && col == 0) combo_idx = 1; // K4
    else if (row == 2 && col == 1) combo_idx = 2; // K8

    if (settings_state == SSTATE_SLEEPING) {
        if (pressed) wake_from_sleep();
        return false;
    }

    if (settings_state != SSTATE_NORMAL && settings_state != SSTATE_HOLD_CONFIRM) {
        if (pressed && row == 2 && col == 2) { // K9 select
            handle_settings_select();
        }
        return false;
    }

    // settings_state is NORMAL or HOLD_CONFIRM here
    if (combo_idx >= 0) {
        if (pressed) {
            combo_key_was_combo[combo_idx] = false; // reset for this new press
            return false; // suppress; decide on release
        } else {
            if (!combo_key_was_combo[combo_idx]) {
                // Never became part of a combo attempt; safe to fire now
                uint16_t real_kc = keymap_key_to_keycode(get_highest_layer(layer_state),
                                                           (keypos_t){.row = row, .col = col});
                tap_code16(real_kc);
            }
            combo_key_was_combo[combo_idx] = false;
            return false;
        }
    }

    touch_activity();

    if (!pressed) return true;

    switch (keycode) {
        case CT_SPOTIFY:
            tap_code16(LCTL(KC_T));
            wait_ms(300);
            send_string("https://open.spotify.com/");
            wait_ms(600);
            tap_code(KC_ENT);
            return false;
        case CT_YOUTUBE:
            tap_code16(LCTL(KC_T));
            wait_ms(300);
            send_string("youtube.com");
            wait_ms(600);
            tap_code(KC_ENT);
            return false;
        case CT_LIBBY:
            tap_code16(LCTL(KC_T));
            wait_ms(300);
            send_string("https://libbyapp.com/shelf");
            wait_ms(600);
            tap_code(KC_ENT);
            return false;

        /* Scientific symbols - requires Unicode input enabled */
        case CT_DEGREE:    send_unicode_string("°"); return false;
        case CT_PLUSMINUS: send_unicode_string("±"); return false;
        case CT_TIMES:     send_unicode_string("×"); return false;
        case CT_SQRT:      send_unicode_string("√"); return false;
        case CT_PI:        send_unicode_string("π"); return false;
        case CT_SIGMA:     send_unicode_string("Σ"); return false;
        case CT_SUPER:     tap_code16(LCTL(LSFT(KC_EQL))); return false;
        case CT_SUB:       tap_code16(LCTL(KC_EQL)); return false;
        case CT_LATEX:     send_string("$$"); tap_code(KC_LEFT); return false;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (settings_state != SSTATE_NORMAL) {
        settings_encoder_nav(clockwise);
        return false;
    }
    touch_activity(); 

    switch (get_highest_layer(layer_state)) {
        case _MAIN:
            if (clockwise) { rgblight_increase_val(); }
            else            { rgblight_decrease_val(); }
            break;
        case _PRODUCTIVITY:
        case _SCIENTIFIC:
            if (clockwise) { tap_code16(LCTL(KC_TAB)); }
            else            { tap_code16(LCTL(LSFT(KC_TAB))); }
            break;
        case _DESIGN:
            if (clockwise) { tap_code(MS_WHLD); }
            else            { tap_code(MS_WHLU); }
            break;
        case _MEDIA:
            if (clockwise) { tap_code(KC_VOLU); }
            else            { tap_code(KC_VOLD); }
            break;
    }
    return true;
}