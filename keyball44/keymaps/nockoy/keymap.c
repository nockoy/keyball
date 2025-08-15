// Custom keymap for nockoy
// Based on Keyball44 firmware

#include QMK_KEYBOARD_H
#include "quantum.h"

enum custom_keycodes {
    KC_MY_BTN1 = SAFE_RANGE,
    KC_MY_BTN2,
    KC_MY_BTN3,
    KC_MY_SCR,
    KC_PLACEHOLDER_4,    // User4 (未使用)
    KC_PLACEHOLDER_5,    // User5 (未使用)
    KC_CTRL_W,           // User6 - Ctrl+W (タブを閉じる)
    KC_CTRL_R,           // User7 - Ctrl+R (ページリロード)
    KC_CHROME_PREV_TAB,  // User8 - Cmd+Option+左矢印 (前のタブ) 
    KC_CHROME_NEXT_TAB,  // User9 - Cmd+Option+右矢印 (次のタブ)
    KC_CMD_M,            // User10 - Cmd+M
};

// トラックボール制御用の変数
const uint16_t mouse_layer = 6;   // マウスレイヤー
uint16_t layer_timer;             // レイヤー切り替え用タイマー
bool scroll_mode = false;         // スクロールモードの状態
uint16_t layer_timeout = 1000;    // レイヤー3から他のレイヤーに戻る時間（ミリ秒）
int16_t movement_threshold = 2;   // レイヤー遷移のための最小動き量の閾値

// スクロール制御用の変数
int16_t scroll_v_counter = 0;
int16_t scroll_h_counter = 0;
int16_t scroll_threshold = 10;

// マウスレイヤーを有効にする
void enable_mouse_layer(void) {
    layer_on(mouse_layer);
    layer_timer = timer_read();
}

// マウスレイヤーを無効にする
void disable_mouse_layer(void) {
    layer_off(mouse_layer);
    scroll_mode = false;
    scroll_v_counter = 0;
    scroll_h_counter = 0;
}

// 絶対値を返すヘルパー関数
int16_t my_abs(int16_t num) {
    return num < 0 ? -num : num;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    
    switch (keycode) {
        case KC_MY_BTN1:
        case KC_MY_BTN2:
        case KC_MY_BTN3:
        {
            report_mouse_t currentReport = pointing_device_get_report();
            uint8_t btn = 1 << (keycode - KC_MY_BTN1);
            
            if (record->event.pressed) {
                currentReport.buttons |= btn;
            } else {
                currentReport.buttons &= ~btn;
            }

            enable_mouse_layer();
            pointing_device_set_report(currentReport);
            pointing_device_send();
            return false;
        }

        case KC_MY_SCR:
            if (record->event.pressed) {
                scroll_mode = !scroll_mode;  // スクロールモードをトグル
                enable_mouse_layer();
            }
            return false;

        case KC_MS_BTN4:
        case KC_MS_BTN5:
            // ブラウザの戻る/進むボタン
            if (record->event.pressed) {
                enable_mouse_layer();
            }
            return true;  // QMKの標準処理を継続

        case KC_CTRL_W:
            // Ctrl+W (タブを閉じる)
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_W);
                enable_mouse_layer();
            } else {
                unregister_code(KC_W);
                unregister_code(KC_LCTL);
            }
            return false;

        case KC_CTRL_R:
            // Ctrl+R (ページリロード)
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_R);
                enable_mouse_layer();
            } else {
                unregister_code(KC_R);
                unregister_code(KC_LCTL);
            }
            return false;

        case KC_CHROME_NEXT_TAB:
            // Cmd+Option+右矢印 (次のタブ)
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_LALT);
                register_code(KC_RGHT);
                enable_mouse_layer();
            } else {
                unregister_code(KC_RGHT);
                unregister_code(KC_LALT);
                unregister_code(KC_LGUI);
            }
            return false;

        case KC_CHROME_PREV_TAB:
            // Cmd+Option+左矢印 (前のタブ)
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_LALT);
                register_code(KC_LEFT);
                enable_mouse_layer();
            } else {
                unregister_code(KC_LEFT);
                unregister_code(KC_LALT);
                unregister_code(KC_LGUI);
            }
            return false;

        case KC_CMD_M:
            // Cmd+M
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_M);
                enable_mouse_layer();
            } else {
                unregister_code(KC_M);
                unregister_code(KC_LGUI);
            }
            return false;

        default:
            if (record->event.pressed) {
                // レイヤー3以外のキーが押されたらマウスレイヤーを無効にする
                if (layer_state_is(mouse_layer)) {
                    disable_mouse_layer();
                }
            }
            break;
    }
   
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    int16_t current_x = mouse_report.x;
    int16_t current_y = mouse_report.y;
    
    // 現在layer6にいるかどうかをチェック
    bool is_mouse_layer_active = layer_state_is(mouse_layer);
    
    // トラックボールが動いた場合の処理
    if (current_x != 0 || current_y != 0) {
        // layer6にいない場合は閾値をチェックしてから遷移
        if (!is_mouse_layer_active) {
            if (my_abs(current_x) >= movement_threshold || my_abs(current_y) >= movement_threshold) {
                enable_mouse_layer();
            } else {
                // 閾値未満の微小な動きは無視
                mouse_report.x = 0;
                mouse_report.y = 0;
                return mouse_report;
            }
        }
        // layer6にいる場合は閾値なしで高感度（タイマーをリセット）
        else {
            layer_timer = timer_read();
        }
        
        if (scroll_mode) {
            // スクロールモードの場合
            int8_t scroll_h = 0;
            int8_t scroll_v = 0;
            
            scroll_h_counter += current_x;
            scroll_v_counter += current_y;
            
            // 水平スクロール
            while (my_abs(scroll_h_counter) > scroll_threshold) {
                if (scroll_h_counter > 0) {
                    scroll_h_counter -= scroll_threshold;
                    scroll_h = 1;
                } else {
                    scroll_h_counter += scroll_threshold;
                    scroll_h = -1;
                }
            }
            
            // 垂直スクロール
            while (my_abs(scroll_v_counter) > scroll_threshold) {
                if (scroll_v_counter > 0) {
                    scroll_v_counter -= scroll_threshold;
                    scroll_v = -1;
                } else {
                    scroll_v_counter += scroll_threshold;
                    scroll_v = 1;
                }
            }
            
            mouse_report.x = 0;
            mouse_report.y = 0;
            mouse_report.h = scroll_h;
            mouse_report.v = scroll_v;
        }
    }

    return mouse_report;
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    
    oled_write_P(PSTR("Layer:"), false);
    oled_write(get_u8_str(get_highest_layer(layer_state), ' '), false);
    
    if (scroll_mode) {
        oled_write_P(PSTR(" SCROLL"), false);
    }
}
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    LAYOUT_universal(
        KC_ESC, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
        KC_LCTL, KC_A, KC_S, LT(6, KC_D), KC_F, KC_G, KC_H, KC_J, LT(6, KC_K), KC_L, KC_ENT, KC_ENT,
        KC_LSFT, LSFT_T(KC_Z), LGUI_T(KC_X), KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, LCTL_T(KC_DOT), KC_BSPC, KC_BSPC, 
        KC_LSFT, KC_LCTL, KC_LGUI, LALT_T(KC_LNG2), LSFT_T(KC_TAB), LT(2, KC_SPC), LT(1, KC_LNG1), LT(1, KC_LNG1), KC_RGUI, KC_RCTL
    ),
    
    LAYOUT_universal(
        KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_TRNS,
        KC_TRNS, LCTL_T(KC_EQL), KC_LBRC, KC_SLSH, KC_MINS, KC_INT1, KC_SCLN, KC_QUOT, KC_RBRC, KC_NUHS, KC_INT3, KC_TRNS,
        KC_TRNS, LSFT_T(KC_PLUS), KC_LCBR, KC_QUES, KC_UNDS, LSFT(KC_INT1), KC_COLN, KC_DQUO, KC_RCBR, LSFT(KC_NUHS), LSFT(KC_INT3),  KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    
    LAYOUT_universal(
        KC_TRNS, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, LGUI(KC_INT3),  KC_TRNS,
        KC_TRNS, KC_PLUS, KC_LCBR, KC_QUES, KC_UNDS, LSFT(KC_INT1), KC_COLN, KC_DQUO, KC_RCBR, LSFT(KC_NUHS), LSFT(KC_INT3),  KC_TRNS,
        KC_TRNS, KC_LSFT, KC_LGUI, KC_LALT, KC_LNG2, KC_LSFT, KC_SPC, KC_LNG1, KC_TRNS, KC_TRNS, KC_DEL,  KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    LAYOUT_universal(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    
    LAYOUT_universal(
        KC_TRNS, KC_NO, KC_TAB, KC_NO, KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_TRNS,
        KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_TRNS,
        KC_TRNS, KC_LSFT, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, MO(5), MO(6), KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    
    LAYOUT_universal(
        KC_TRNS, RGB_TOG, RGB_MOD, RGB_HUI, RGB_SAI, RGB_VAI, CPI_I100, CPI_D100, KC_NO, KC_NO, KC_NO, KBC_SAVE,
        KC_TRNS, RGB_M_P, RGB_M_B, RGB_M_R, RGB_M_SW, RGB_M_SN, CPI_I1K, CPI_D1K, KC_NO, KC_NO, KC_NO, KBC_RST,
        KC_TRNS, RGB_M_K, RGB_M_X, RGB_M_G, KC_NO, KC_NO, QK_BOOT, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ), 

    LAYOUT_universal(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_MY_BTN2, KC_MY_SCR, KC_MY_BTN1, KC_TRNS, KC_TRNS, KC_MY_BTN1, KC_MY_SCR, KC_MY_BTN3, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};