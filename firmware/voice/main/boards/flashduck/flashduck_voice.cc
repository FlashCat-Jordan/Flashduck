#include "flashduck_voice.h"

#include <esp_log.h>
#include <esp_random.h>
#include <esp_timer.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include "application.h"
#include "audio_service.h"
#include "assets/lang_config.h"
#include "flashduck_controller.h"
#include "flashduck_movements.h"

#define TAG "FlashduckVoice"

namespace {

// 张嘴语汇的风格数，和 flashduck_movements.h 里的 MOUTH_GESTURE_STYLE_COUNT 一致。
constexpr int kGestureStyleCount = MOUTH_GESTURE_STYLE_COUNT;

// 两次发声之间的最短间隔（ms）。防止唤醒应答、开始听、回复三连抽挤在一起。
constexpr int64_t kListeningMinGapMs = 1500;

struct DuckSound {
    const char* name;
    std::string_view ogg;
    int duration_ms;
};

// 13 个情绪音，和 assets/common/duck_*.ogg 一一对应（时长按实际文件量过）。
const DuckSound kSounds[] = {
    {"angry", Lang::Sounds::OGG_DUCK_ANGRY, 2360},
    {"curious", Lang::Sounds::OGG_DUCK_CURIOUS, 1430},
    {"defiant", Lang::Sounds::OGG_DUCK_DEFIANT, 1410},
    {"devastated", Lang::Sounds::OGG_DUCK_DEVASTATED, 6610},
    {"excited", Lang::Sounds::OGG_DUCK_EXCITED, 2850},
    {"impatient", Lang::Sounds::OGG_DUCK_IMPATIENT, 2310},
    {"laugh", Lang::Sounds::OGG_DUCK_LAUGH, 2430},
    {"mmh", Lang::Sounds::OGG_DUCK_MMH, 1010},
    {"mock", Lang::Sounds::OGG_DUCK_MOCK, 2070},
    {"no", Lang::Sounds::OGG_DUCK_NO, 1480},
    {"sad", Lang::Sounds::OGG_DUCK_SAD, 3110},
    {"yes", Lang::Sounds::OGG_DUCK_YES, 810},
    {"yes_fast", Lang::Sounds::OGG_DUCK_YES_FAST, 690},
};
constexpr int kSoundCount = sizeof(kSounds) / sizeof(kSounds[0]);

int last_index = -1;
int last_style = -1;
int64_t last_play_ms = 0;
int current_mood = DUCK_MOOD_NEUTRAL;

int64_t NowMs() { return esp_timer_get_time() / 1000; }

bool PlaybackBusy() { return !Application::GetInstance().GetAudioService().IsPlaybackIdle(); }

// 挑一种张法，避开上一声用过的那一种。
int PickGestureStyle() {
    int style = static_cast<int>(esp_random() % kGestureStyleCount);
    if (style == last_style && kGestureStyleCount > 1) {
        style = (style + 1 + static_cast<int>(esp_random() % (kGestureStyleCount - 1))) %
                kGestureStyleCount;
    }
    last_style = style;
    return style;
}

// 情绪 → 脖子气质。认不出就中性。
int MoodFromEmotion(const char* emotion) {
    if (emotion == nullptr || emotion[0] == '\0') {
        return DUCK_MOOD_NEUTRAL;
    }
    std::string text(emotion);
    for (auto& c : text) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    if (text.find("happy") != std::string::npos || text.find("joy") != std::string::npos ||
        text.find("excited") != std::string::npos || text.find("laugh") != std::string::npos) {
        return DUCK_MOOD_HAPPY;
    }
    if (text.find("sad") != std::string::npos || text.find("cry") != std::string::npos ||
        text.find("depress") != std::string::npos) {
        return DUCK_MOOD_SAD;
    }
    if (text.find("angry") != std::string::npos || text.find("mad") != std::string::npos ||
        text.find("annoy") != std::string::npos || text.find("rage") != std::string::npos) {
        return DUCK_MOOD_ANGRY;
    }
    if (text.find("surpris") != std::string::npos || text.find("shock") != std::string::npos) {
        return DUCK_MOOD_SURPRISED;
    }
    if (text.find("curious") != std::string::npos || text.find("confus") != std::string::npos ||
        text.find("think") != std::string::npos) {
        return DUCK_MOOD_CURIOUS;
    }
    return DUCK_MOOD_NEUTRAL;
}

// 播放 + 同步张嘴转脖子。返回 false 表示这次没叫（忙 / 队列占用）。
bool PlaySoundAt(int index) {
    const DuckSound& sound = kSounds[index];

    if (PlaybackBusy()) {
        ESP_LOGD(TAG, "上一段声音还没播完，跳过 %s", sound.name);
        return false;
    }

    const int style = PickGestureStyle();
    if (!FlashduckSpeakGesture(style, sound.duration_ms, current_mood, esp_random())) {
        // 动作队列被占（正在走路/做动作）：不放声音，避免“只出声不动嘴”。
        ESP_LOGD(TAG, "动作队列忙，跳过 %s", sound.name);
        return false;
    }

    Application::GetInstance().GetAudioService().PlaySound(sound.ogg);
    last_index = index;
    last_play_ms = NowMs();
    ESP_LOGI(TAG, "鸭叫: %s (%dms), 张嘴 #%d, 脖子情绪 %d", sound.name, sound.duration_ms, style,
             current_mood);
    return true;
}

int FindByName(const char* name) {
    for (int i = 0; i < kSoundCount; ++i) {
        if (std::strcmp(kSounds[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

}  // namespace

void FlashduckVoiceInit() {
    ESP_LOGI(TAG, "本地鸭叫已启用：TTS 静音，改用情绪音 + 嘴巴同步（%d 个音效）", kSoundCount);
}

void FlashduckVoiceOnSpeaking() {
    // 随机一个，但不与上一次重复：连续两次同样声音会像卡带。
    int index = static_cast<int>(esp_random() % kSoundCount);
    if (index == last_index && kSoundCount > 1) {
        index = (index + 1 + static_cast<int>(esp_random() % (kSoundCount - 1))) % kSoundCount;
    }
    PlaySoundAt(index);
}

void FlashduckVoiceOnEmotion(const char* emotion) {
    const int mood = MoodFromEmotion(emotion);
    if (mood != current_mood) {
        ESP_LOGI(TAG, "情绪: %s -> mood %d", emotion == nullptr ? "(null)" : emotion, mood);
    }
    current_mood = mood;
}

void FlashduckVoiceOnListening() {
    const int index = FindByName("mmh");
    if (index < 0) {
        return;
    }
    if (last_play_ms != 0 && NowMs() - last_play_ms < kListeningMinGapMs) {
        return;  // 刚叫过（比如回复的那一声），别再叠一个“嗯”
    }
    PlaySoundAt(index);
}

void FlashduckVoiceOnWakeWord() {
    const int index = FindByName("curious");
    if (index >= 0) {
        current_mood = DUCK_MOOD_CURIOUS;
        PlaySoundAt(index);
    }
}
