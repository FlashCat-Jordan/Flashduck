#pragma once

#include "config.h"

struct FlashduckGaitParams {
    int hip_offset;
    int hip_amp;
    int foot_amp;
    int phase_deg;
    int speed;
    int head_swing;
};

void InitializeFlashduckController(const HardwareConfig& hw_config);

// 局域网调参页/测试按钮使用的轻量入口。
void FlashduckStartWalk(int steps, int speed, int direction, int hip_offset, int hip_amp,
                        int foot_amp, int phase_deg, int head_swing);
void FlashduckStop();

// 叫声同步：嘴巴（style 张法）+ 脖子（mood 情绪）一起动，铺满 duration_ms。
// 低优先级：真指令入队时会提前收场；有指令在等时直接返回 false。
bool FlashduckSpeakGesture(int style, int duration_ms, int mood, uint32_t seed = 1);

// 立刻收场（被打断、或离开“说话中”状态时用）。
void FlashduckAbortSpeakGesture();
bool FlashduckIsMoving();

FlashduckGaitParams FlashduckGetGaitDefaults();
void FlashduckSaveGaitDefaults(const FlashduckGaitParams& gait);
