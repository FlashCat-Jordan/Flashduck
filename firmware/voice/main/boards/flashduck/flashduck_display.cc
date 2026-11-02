#include "flashduck_display.h"

#include <esp_log.h>
#include <esp_random.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

#include "assets/lang_config.h"
#include "display/lvgl_display/lvgl_theme.h"
#include "system_info.h"

#define TAG "FlashduckDisplay"

namespace {
constexpr int kEyeSize = 96;       // 眼白直径
constexpr int kRootSize = 104;     // 留出边框余量
constexpr int kEyeOffset = (kRootSize - kEyeSize) / 2;
constexpr int kIrisSize = 54;
constexpr int kIrisOffset = (kEyeSize - kIrisSize) / 2;
constexpr int kPupilSize = 26;
constexpr int kPupilOffset = (kIrisSize - kPupilSize) / 2;
constexpr int kEyeTop = 18;        // 上方 1/2 区域，避开顶部状态栏
constexpr uint32_t kBlinkCloseMs = 80;
constexpr uint32_t kBlinkHoldMs = 35;
constexpr uint32_t kBlinkOpenMs = 170;

bool IsClockText(const char* text) {
    return text != nullptr && strlen(text) == 5 && isdigit(static_cast<unsigned char>(text[0])) &&
           isdigit(static_cast<unsigned char>(text[1])) && text[2] == ':' &&
           isdigit(static_cast<unsigned char>(text[3])) &&
           isdigit(static_cast<unsigned char>(text[4]));
}

bool StartsWithVersion(const char* text) {
    if (text == nullptr) {
        return false;
    }
    const size_t prefix_len = strlen(Lang::Strings::VERSION);
    return strncmp(text, Lang::Strings::VERSION, prefix_len) == 0;
}

bool IsSystemUserAgent(const char* text) {
    return text != nullptr && strcmp(text, SystemInfo::GetUserAgent().c_str()) == 0;
}
}  // namespace

FlashduckDisplay::FlashduckDisplay(esp_lcd_panel_io_handle_t panel_io,
                                   esp_lcd_panel_handle_t panel, int width, int height,
                                   int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                                   bool swap_xy)
    : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y, mirror_x, mirror_y,
                    swap_xy) {}

FlashduckDisplay::~FlashduckDisplay() {
    if (eye_timer_ != nullptr) {
        lv_timer_delete(eye_timer_);
        eye_timer_ = nullptr;
    }
}

void FlashduckDisplay::SetupUI() {
    if (setup_ui_called_) {
        return;
    }

    SpiLcdDisplay::SetupUI();

    // 眼睛就是主视觉，不再显示低电量大弹窗；电量仍由状态栏电池图标表达。
    if (low_battery_popup_ != nullptr) {
        lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
    }

    auto* dark_theme = LvglThemeManager::GetInstance().GetTheme("dark");
    if (dark_theme != nullptr) {
        SetTheme(dark_theme);
    }

    // 停掉 GIF/emoji 内容，只保留自绘眼睛。
    if (gif_controller_) {
        gif_controller_->Stop();
        gif_controller_.reset();
    }
    if (emoji_image_ != nullptr) {
        lv_obj_add_flag(emoji_image_, LV_OBJ_FLAG_HIDDEN);
    }
    if (emoji_label_ != nullptr) {
        lv_obj_add_flag(emoji_label_, LV_OBJ_FLAG_HIDDEN);
    }
    if (emoji_box_ != nullptr) {
        lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
    }
    if (preview_image_ != nullptr) {
        lv_obj_add_flag(preview_image_, LV_OBJ_FLAG_HIDDEN);
    }

    // 顶部只保留电池/网络等图标，状态文字全部交给眼睛表达。
    if (status_label_ != nullptr) {
        lv_obj_add_flag(status_label_, LV_OBJ_FLAG_HIDDEN);
    }

    CreateEye();

    if (!eye_state_set_) {
        SetEyeState(EyeState::Idle);
    }
    current_style_ = target_style_;
    ApplyPupil();
    ApplyIrisColor();
    ApplyLids(0.0f);

    uint32_t now = lv_tick_get();
    next_move_ms_ = now + 500;
    next_blink_ms_ = now + target_style_.blink_min_ms +
                     (esp_random() % (target_style_.blink_max_ms - target_style_.blink_min_ms + 1));
    eye_timer_ = lv_timer_create(OnEyeTimer, 30, this);
    ESP_LOGI(TAG, "单眼动画已启动");
}

void FlashduckDisplay::HideStatusLabel() {
    if (!setup_ui_called_ || status_label_ == nullptr) {
        return;
    }
    DisplayLockGuard lock(this);
    lv_obj_add_flag(status_label_, LV_OBJ_FLAG_HIDDEN);
}

void FlashduckDisplay::SetStatus(const char* status) {
    if (status == nullptr) {
        return;
    }
    // 顶部不再显示任何状态文字；所有状态都映射成眼睛神态。
    HideStatusLabel();

    if (IsClockText(status) || StartsWithVersion(status) || IsSystemUserAgent(status)) {
        return;
    }

    if (strcmp(status, Lang::Strings::STANDBY) == 0) {
        SetEyeState(EyeState::Idle);
    } else if (strcmp(status, Lang::Strings::LISTENING) == 0) {
        SetEyeState(EyeState::Listening);
    } else if (strcmp(status, Lang::Strings::SPEAKING) == 0) {
        SetEyeState(EyeState::Speaking);
    } else if (strcmp(status, Lang::Strings::CONNECTING) == 0 ||
               strcmp(status, Lang::Strings::SCANNING_WIFI) == 0 ||
               strcmp(status, Lang::Strings::WIFI_CONFIG_MODE) == 0 ||
               strcmp(status, Lang::Strings::REGISTERING_NETWORK) == 0 ||
               strcmp(status, Lang::Strings::DETECTING_MODULE) == 0) {
        SetEyeState(EyeState::Scanning);
    } else if (strcmp(status, Lang::Strings::ACTIVATION) == 0 ||
               strcmp(status, Lang::Strings::LOADING_PROTOCOL) == 0 ||
               strcmp(status, Lang::Strings::CHECKING_NEW_VERSION) == 0) {
        SetEyeState(EyeState::Connected);
    } else if (strncmp(status, Lang::Strings::CONNECTED_TO,
                       strlen(Lang::Strings::CONNECTED_TO)) == 0) {
        SetEyeState(EyeState::Connected);
    } else if (strncmp(status, Lang::Strings::CONNECT_TO,
                       strlen(Lang::Strings::CONNECT_TO)) == 0) {
        SetEyeState(EyeState::Scanning);
    }
}

void FlashduckDisplay::ShowNotification(const char* notification, int duration_ms) {
    if (notification == nullptr) {
        return;
    }
    // 版本、扫描 WiFi、连接中、已连接都改成眼神，不再占用顶部文字。
    if (StartsWithVersion(notification)) {
        return;
    }
    if (strncmp(notification, Lang::Strings::CONNECTED_TO,
                strlen(Lang::Strings::CONNECTED_TO)) == 0) {
        SetEyeState(EyeState::Connected);
        return;
    }
    if (strcmp(notification, Lang::Strings::SCANNING_WIFI) == 0 ||
        strncmp(notification, Lang::Strings::CONNECT_TO,
                strlen(Lang::Strings::CONNECT_TO)) == 0) {
        SetEyeState(EyeState::Scanning);
        return;
    }
    SpiLcdDisplay::ShowNotification(notification, duration_ms);
}

FlashduckDisplay::EyeStyle FlashduckDisplay::StyleFor(EyeState state) {
    switch (state) {
        case EyeState::Scanning:
            // 扫描/连接：冷青色虹膜、瞳孔缩小、眼睛睁大并左右扫视。
            return {0.08f, 0.02f, 0.70f, 12.0f, 3.0f, 900, 1400, 3000, 6500,
                    lv_color_hex(0x2FD0FF)};
        case EyeState::Connected:
            // 已连接：绿色虹膜 + 明显月牙眯眼，短暂停留后回待机。
            return {0.52f, 0.28f, 1.10f, 2.0f, 1.0f, 2000, 3000, 2500, 5000,
                    lv_color_hex(0x52E37A)};
        case EyeState::Listening:
            // 聆听：紫罗兰虹膜、完全睁大、瞳孔放到最大、眼神定住。
            return {0.00f, 0.00f, 1.35f, 3.0f, 2.0f, 1500, 3000, 5000, 9000,
                    lv_color_hex(0x9B7BFF)};
        case EyeState::Speaking:
            // 说话：橙色虹膜、眼睛明亮、眼神更活跃、眨眼更频繁。
            return {0.10f, 0.02f, 1.15f, 11.0f, 5.0f, 400, 1000, 1600, 3000,
                    lv_color_hex(0xFF5A2A)};
        case EyeState::Idle:
        default:
            // 待机：暖琥珀色、放松半眯、慢速随机看、偶尔眨眼。
            return {0.32f, 0.10f, 1.00f, 9.0f, 5.0f, 1400, 3400, 3000, 6500,
                    lv_color_hex(0xFFB000)};
    }
}

void FlashduckDisplay::SetEyeState(EyeState state) {
    const uint32_t now = lv_tick_get();
    if (eye_state_ == state) {
        if (state == EyeState::Connected) {
            state_hold_until_ = now + 2200;
        }
        return;
    }

    eye_state_ = state;
    eye_state_set_ = true;
    ESP_LOGI(TAG, "眼状态 -> %d", static_cast<int>(state));
    target_style_ = StyleFor(state);
    state_hold_until_ = state == EyeState::Connected ? now + 2200 : 0;
    next_scan_ms_ = now + 200;
    next_move_ms_ = now + 250;
    target_x_ = 0.0f;
    target_y_ = 0.0f;

    const uint32_t blink_span = target_style_.blink_max_ms - target_style_.blink_min_ms + 1;
    next_blink_ms_ = now + target_style_.blink_min_ms + (esp_random() % blink_span);
}

void FlashduckDisplay::ApplyPupil() {
    if (pupil_ == nullptr) {
        return;
    }
    const float scale = std::clamp(current_style_.pupil_scale, 0.75f, 1.30f);
    const int size = static_cast<int>(kPupilSize * scale + 0.5f);
    const int offset = (kIrisSize - size) / 2;
    lv_obj_set_size(pupil_, size, size);
    lv_obj_set_pos(pupil_, offset, offset);
}

void FlashduckDisplay::ApplyIrisColor() {
    if (iris_ == nullptr) {
        return;
    }
    lv_obj_set_style_bg_color(iris_, current_style_.iris_color, 0);
}

void FlashduckDisplay::ApplyLids(float blink_fraction) {
    if (lid_top_ == nullptr || lid_bottom_ == nullptr) {
        return;
    }
    const float top_base = std::clamp(current_style_.upper_lid, 0.0f, 0.80f);
    const float bottom_base = std::clamp(current_style_.lower_lid, 0.0f, 0.60f);
    const float f = std::clamp(blink_fraction, 0.0f, 1.0f);
    const float top = top_base + (1.0f - top_base) * f;
    const float bottom = bottom_base + (1.0f - bottom_base) * f * 0.15f;
    const int top_height = static_cast<int>(kEyeSize * top + 0.5f);
    const int bottom_height = static_cast<int>(kEyeSize * bottom + 0.5f);
    lv_obj_set_height(lid_top_, top_height);
    lv_obj_set_height(lid_bottom_, bottom_height);
    lv_obj_set_y(lid_bottom_, kEyeOffset + kEyeSize - bottom_height);
}

void FlashduckDisplay::SetChatMessage(const char* role, const char* content) {
    // 启动时 Application 会把 BOARD_NAME/version 作为 system 消息推到聊天区；
    // 这不是聊天内容，直接吞掉。
    if (role != nullptr && content != nullptr && strcmp(role, "system") == 0 &&
        IsSystemUserAgent(content)) {
        return;
    }
    SpiLcdDisplay::SetChatMessage(role, content);
}

bool FlashduckDisplay::SetEmotion(const char* emotion) {
    // 表情由眼睛自身的自然运动表达，忽略上层传入的 GIF 表情名。
    (void)emotion;
    return true;
}

void FlashduckDisplay::SetPreviewImage(std::unique_ptr<LvglImage> image) {
    const bool show_eye = (image == nullptr);
    {
        DisplayLockGuard lock(this);
        if (eye_root_ != nullptr) {
            if (show_eye) {
                lv_obj_remove_flag(eye_root_, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(eye_root_);
            } else {
                lv_obj_add_flag(eye_root_, LV_OBJ_FLAG_HIDDEN);
            }
        }
        if (eye_timer_ != nullptr) {
            if (show_eye) {
                lv_timer_resume(eye_timer_);
            } else {
                lv_timer_pause(eye_timer_);
            }
        }
    }

    SpiLcdDisplay::SetPreviewImage(std::move(image));

    if (show_eye) {
        DisplayLockGuard lock(this);
        // 父类在预览结束时可能重新打开 emoji 内容，这里重新压回眼睛。
        if (gif_controller_) {
            gif_controller_->Stop();
        }
        if (emoji_box_ != nullptr) {
            lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
        }
        if (emoji_image_ != nullptr) {
            lv_obj_add_flag(emoji_image_, LV_OBJ_FLAG_HIDDEN);
        }
        if (emoji_label_ != nullptr) {
            lv_obj_add_flag(emoji_label_, LV_OBJ_FLAG_HIDDEN);
        }
        if (eye_root_ != nullptr) {
            lv_obj_remove_flag(eye_root_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(eye_root_);
        }
        if (status_bar_ != nullptr) {
            lv_obj_move_foreground(status_bar_);
        }
    }
}

void FlashduckDisplay::UpdateStatusBar(bool update_all) {
    SpiLcdDisplay::UpdateStatusBar(update_all);
    if (low_battery_popup_ != nullptr) {
        lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
    }
}

void FlashduckDisplay::CreateEye() {
    lv_obj_t* screen = lv_screen_active();
    if (screen == nullptr) {
        return;
    }
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    eye_root_ = lv_obj_create(screen);
    lv_obj_set_size(eye_root_, kRootSize, kRootSize);
    lv_obj_align(eye_root_, LV_ALIGN_TOP_MID, 0, kEyeTop);
    lv_obj_set_style_bg_opa(eye_root_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(eye_root_, 0, 0);
    lv_obj_set_style_pad_all(eye_root_, 0, 0);
    lv_obj_clear_flag(eye_root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(eye_root_, LV_OBJ_FLAG_CLICKABLE);

    sclera_ = lv_obj_create(eye_root_);
    lv_obj_set_size(sclera_, kEyeSize, kEyeSize);
    lv_obj_set_pos(sclera_, kEyeOffset, kEyeOffset);
    lv_obj_set_style_radius(sclera_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(sclera_, lv_color_hex(0xFFF6E7), 0);
    lv_obj_set_style_bg_opa(sclera_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sclera_, 2, 0);
    lv_obj_set_style_border_color(sclera_, lv_color_hex(0x242424), 0);
    lv_obj_clear_flag(sclera_, LV_OBJ_FLAG_SCROLLABLE);

    iris_ = lv_obj_create(sclera_);
    lv_obj_set_size(iris_, kIrisSize, kIrisSize);
    lv_obj_set_pos(iris_, kIrisOffset, kIrisOffset);
    lv_obj_set_style_radius(iris_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(iris_, lv_color_hex(0xFFB300), 0);
    lv_obj_set_style_bg_opa(iris_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(iris_, 0, 0);
    lv_obj_clear_flag(iris_, LV_OBJ_FLAG_SCROLLABLE);

    pupil_ = lv_obj_create(iris_);
    lv_obj_set_size(pupil_, kPupilSize, kPupilSize);
    lv_obj_set_pos(pupil_, kPupilOffset, kPupilOffset);
    lv_obj_set_style_radius(pupil_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_, lv_color_hex(0x0B0B0B), 0);
    lv_obj_set_style_bg_opa(pupil_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pupil_, 0, 0);
    lv_obj_clear_flag(pupil_, LV_OBJ_FLAG_SCROLLABLE);

    highlight_ = lv_obj_create(iris_);
    lv_obj_set_size(highlight_, 9, 9);
    lv_obj_set_pos(highlight_, 10, 7);
    lv_obj_set_style_radius(highlight_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(highlight_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(highlight_, LV_OPA_80, 0);
    lv_obj_set_style_border_width(highlight_, 0, 0);
    lv_obj_clear_flag(highlight_, LV_OBJ_FLAG_SCROLLABLE);

    lid_top_ = lv_obj_create(eye_root_);
    lv_obj_set_size(lid_top_, kEyeSize, 0);
    lv_obj_set_pos(lid_top_, kEyeOffset, kEyeOffset);
    lv_obj_set_style_radius(lid_top_, 0, 0);
    lv_obj_set_style_bg_color(lid_top_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lid_top_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(lid_top_, 0, 0);
    lv_obj_clear_flag(lid_top_, LV_OBJ_FLAG_SCROLLABLE);

    lid_bottom_ = lv_obj_create(eye_root_);
    lv_obj_set_size(lid_bottom_, kEyeSize, 0);
    lv_obj_set_pos(lid_bottom_, kEyeOffset, kEyeOffset + kEyeSize);
    lv_obj_set_style_radius(lid_bottom_, 0, 0);
    lv_obj_set_style_bg_color(lid_bottom_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lid_bottom_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(lid_bottom_, 0, 0);
    lv_obj_clear_flag(lid_bottom_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_move_foreground(lid_top_);
    lv_obj_move_foreground(lid_bottom_);
    if (status_bar_ != nullptr) {
        lv_obj_move_foreground(status_bar_);
    }
}

void FlashduckDisplay::OnEyeTimer(lv_timer_t* timer) {
    auto* self = static_cast<FlashduckDisplay*>(lv_timer_get_user_data(timer));
    if (self != nullptr) {
        self->UpdateEye();
    }
}

void FlashduckDisplay::UpdateEye() {
    if (eye_root_ == nullptr || iris_ == nullptr) {
        return;
    }

    const uint32_t now = lv_tick_get();

    // 已连接是一个短暂神态，1.8 秒后自然回到待机眼神。
    if (eye_state_ == EyeState::Connected && state_hold_until_ != 0 &&
        static_cast<int32_t>(now - state_hold_until_) >= 0) {
        SetEyeState(EyeState::Idle);
    }

    // 状态之间的眼睑/瞳孔/扫视范围平滑过渡，不硬切。
    constexpr float kStyleLerp = 0.08f;
    current_style_.upper_lid += (target_style_.upper_lid - current_style_.upper_lid) * kStyleLerp;
    current_style_.lower_lid += (target_style_.lower_lid - current_style_.lower_lid) * kStyleLerp;
    current_style_.pupil_scale +=
        (target_style_.pupil_scale - current_style_.pupil_scale) * kStyleLerp;
    current_style_.gaze_x += (target_style_.gaze_x - current_style_.gaze_x) * kStyleLerp;
    current_style_.gaze_y += (target_style_.gaze_y - current_style_.gaze_y) * kStyleLerp;
    auto lerp_channel = [](uint8_t from, uint8_t to) -> uint8_t {
        return static_cast<uint8_t>(from + (static_cast<int>(to) - static_cast<int>(from)) * 0.08f);
    };
    current_style_.iris_color.red =
        lerp_channel(current_style_.iris_color.red, target_style_.iris_color.red);
    current_style_.iris_color.green =
        lerp_channel(current_style_.iris_color.green, target_style_.iris_color.green);
    current_style_.iris_color.blue =
        lerp_channel(current_style_.iris_color.blue, target_style_.iris_color.blue);
    ApplyPupil();
    ApplyIrisColor();

    if (eye_state_ == EyeState::Scanning) {
        // 扫描 WiFi：不是乱看，而是缓慢左右扫，像在找信号。
        if (static_cast<int32_t>(now - next_scan_ms_) >= 0) {
            scan_right_ = !scan_right_;
            target_x_ = (scan_right_ ? 1.0f : -1.0f) * current_style_.gaze_x;
            target_y_ = 0.0f;
            next_scan_ms_ = now + 800 + (esp_random() % 500);
        }
    } else if (eye_state_ == EyeState::Connected) {
        target_x_ = 0.0f;
        target_y_ = 0.0f;
    } else if (eye_state_ == EyeState::Listening) {
        // 聆听：眼神基本定住，只做很小的微扫视，配合放大的瞳孔。
        if (static_cast<int32_t>(now - next_move_ms_) >= 0) {
            target_x_ = static_cast<float>(static_cast<int>(esp_random() % 5) - 2);
            target_y_ = static_cast<float>(static_cast<int>(esp_random() % 3) - 1);
            next_move_ms_ = now + 1500 + (esp_random() % 2000);
        }
    } else {
        // 待机/说话：慢速随机扫视；说话时参数更活跃。
        if (static_cast<int32_t>(now - next_move_ms_) >= 0) {
            const int gx = std::max(1, static_cast<int>(current_style_.gaze_x));
            const int gy = std::max(1, static_cast<int>(current_style_.gaze_y));
            int tx = 0;
            int ty = 0;
            if ((esp_random() % 100) >= 18) {
                tx = static_cast<int>(esp_random() % (2 * gx + 1)) - gx;
                ty = static_cast<int>(esp_random() % (2 * gy + 1)) - gy;
            }
            target_x_ = static_cast<float>(tx);
            target_y_ = static_cast<float>(ty);
            const uint32_t span =
                target_style_.move_max_ms > target_style_.move_min_ms
                    ? target_style_.move_max_ms - target_style_.move_min_ms
                    : 1;
            next_move_ms_ = now + target_style_.move_min_ms + (esp_random() % span);
        }
    }

    const float max_x = std::max(1.0f, current_style_.gaze_x);
    const float max_y = std::max(1.0f, current_style_.gaze_y);
    target_x_ = std::clamp(target_x_, -max_x, max_x);
    target_y_ = std::clamp(target_y_, -max_y, max_y);
    current_x_ += (target_x_ - current_x_) * 0.16f;
    current_y_ += (target_y_ - current_y_) * 0.16f;
    lv_obj_set_pos(iris_, kIrisOffset + static_cast<int>(std::lround(current_x_)),
                   kIrisOffset + static_cast<int>(std::lround(current_y_)));

    float blink_fraction = 0.0f;
    if (!blinking_ && static_cast<int32_t>(now - next_blink_ms_) >= 0) {
        blinking_ = true;
        blink_start_ms_ = now;
        double_blink_ = (esp_random() % 100) < 12;
        const uint32_t span =
            target_style_.blink_max_ms > target_style_.blink_min_ms
                ? target_style_.blink_max_ms - target_style_.blink_min_ms
                : 1;
        next_blink_ms_ = now + target_style_.blink_min_ms + (esp_random() % span);
    }

    if (blinking_) {
        const uint32_t elapsed = now - blink_start_ms_;
        float close_fraction = 0.0f;
        bool finished = false;

        if (elapsed < kBlinkCloseMs) {
            close_fraction = static_cast<float>(elapsed) / kBlinkCloseMs;
        } else if (elapsed < kBlinkCloseMs + kBlinkHoldMs) {
            close_fraction = 1.0f;
        } else if (elapsed < kBlinkCloseMs + kBlinkHoldMs + kBlinkOpenMs) {
            close_fraction =
                1.0f - static_cast<float>(elapsed - kBlinkCloseMs - kBlinkHoldMs) / kBlinkOpenMs;
        } else {
            finished = true;
        }

        if (finished) {
            blinking_ = false;
            if (double_blink_) {
                double_blink_ = false;
                next_blink_ms_ = now + 180;
            }
        } else {
            blink_fraction = close_fraction * close_fraction * (3.0f - 2.0f * close_fraction);
        }
    }

    ApplyLids(blink_fraction);
}
