#include "flashduck_movements.h"

#include <algorithm>
#include <cstdint>
#include <math.h>
#include <vector>

#include "freertos/idf_additions.h"
#include "oscillator.h"

static const char* TAG = "FlashduckMovements";

Flashduck::Flashduck() {
    is_otto_resting_ = false;
    has_head_ = false;
    // 初始化所有舵机管脚为-1（未连接）
    for (int i = 0; i < SERVO_COUNT; i++) {
        servo_pins_[i] = -1;
        servo_trim_[i] = 0;
    }
}

Flashduck::~Flashduck() {
    DetachServos();
}

unsigned long IRAM_ATTR millis() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

void Flashduck::Init(int left_leg, int right_leg, int left_foot, int right_foot, int neck,
                int mouth) {
    servo_pins_[LEFT_LEG] = left_leg;
    servo_pins_[RIGHT_LEG] = right_leg;
    servo_pins_[LEFT_FOOT] = left_foot;
    servo_pins_[RIGHT_FOOT] = right_foot;
    // 摄像头版：这两个管脚接脖子/嘴巴舵机（字段名沿用历史命名 left_hand_pin/right_hand_pin）
    servo_pins_[NECK] = neck;
    servo_pins_[MOUTH] = mouth;

    // 检查是否有脖子+嘴巴舵机
    has_head_ = (neck != -1 && mouth != -1);

    AttachServos();
    // 物理限位（在 Oscillator::Write 最终输出处强制钳位，任何路径都越不过）：
    // 嘴巴只能 45(闭合)~60(全开)，超出会顶坏机械限位；脖子装好后限位 45~180
    if (servo_pins_[MOUTH] != -1) {
        servo_[MOUTH].SetRange(MOUTH_MIN_ANGLE, MOUTH_MAX_ANGLE);
    }
    if (servo_pins_[NECK] != -1) {
        servo_[NECK].SetRange(NECK_LEFT_ANGLE, NECK_RIGHT_ANGLE);
    }
    // 启用舵机限速器: 防止突发跳变造成的抖动, 不影响正常振荡运动
    EnableServoLimit(700);
    is_otto_resting_ = false;
}

///////////////////////////////////////////////////////////////////
//-- ATTACH & DETACH FUNCTIONS ----------------------------------//
///////////////////////////////////////////////////////////////////
void Flashduck::AttachServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].Attach(servo_pins_[i]);
        }
    }
}

void Flashduck::DetachServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].Detach();
        }
    }
}

///////////////////////////////////////////////////////////////////
//-- OSCILLATORS TRIMS ------------------------------------------//
///////////////////////////////////////////////////////////////////
void Flashduck::SetTrims(int left_leg, int right_leg, int left_foot, int right_foot, int neck,
                    int mouth) {
    servo_trim_[LEFT_LEG] = left_leg;
    servo_trim_[RIGHT_LEG] = right_leg;
    servo_trim_[LEFT_FOOT] = left_foot;
    servo_trim_[RIGHT_FOOT] = right_foot;

    if (has_head_) {
        servo_trim_[NECK] = neck;
        servo_trim_[MOUTH] = mouth;
    }

    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetTrim(servo_trim_[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////
//-- BASIC MOTION FUNCTIONS -------------------------------------//
///////////////////////////////////////////////////////////////////
void Flashduck::MoveServos(int time, int servo_target[]) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    // 记录起始位置，用于余弦缓动
    int start_pos[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; i++) {
        start_pos[i] = (servo_pins_[i] != -1) ? servo_[i].GetPosition() : 0;
    }

    final_time_ = millis() + time;
    if (time > 10) {
        unsigned long start_millis = millis();
        // 使用余弦缓动: progress = (1 - cos(π * t)) / 2
        // t=0 与 t=time 时速度都为 0, 消除线性插值在起末点的速度突变 (抖动感来源)
        while (millis() < final_time_) {
            if (stop_requested_) return;  // 紧急停止：立即退出缓动
            float elapsed = (float)(millis() - start_millis) / (float)time;
            if (elapsed > 1.0f) elapsed = 1.0f;
            if (elapsed < 0.0f) elapsed = 0.0f;
            float eased = (1.0f - cosf((float)M_PI * elapsed)) / 2.0f;

            for (int i = 0; i < SERVO_COUNT; i++) {
                if (servo_pins_[i] != -1) {
                    int new_pos = start_pos[i] + (int)((servo_target[i] - start_pos[i]) * eased);
                    servo_[i].SetPosition(new_pos);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    } else {
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1) {
                servo_[i].SetPosition(servo_target[i]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(time));
    }

    // final adjustment to the target.
    bool f = true;
    int adjustment_count = 0;
    while (f && adjustment_count < 10) {
        f = false;
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1 && servo_target[i] != servo_[i].GetPosition()) {
                f = true;
                break;
            }
        }
        if (f) {
            for (int i = 0; i < SERVO_COUNT; i++) {
                if (servo_pins_[i] != -1) {
                    servo_[i].SetPosition(servo_target[i]);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            adjustment_count++;
        }
    };
}

void Flashduck::MoveSingle(int position, int servo_number) {
    if (position > 180)
        position = 90;
    if (position < 0)
        position = 90;

    if (GetRestState() == true) {
        SetRestState(false);
    }

    if (servo_number >= 0 && servo_number < SERVO_COUNT && servo_pins_[servo_number] != -1) {
        servo_[servo_number].SetPosition(position);
    }
}

void Flashduck::OscillateServos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                           double phase_diff[SERVO_COUNT], float cycle = 1) {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetO(offset[i]);
            servo_[i].SetA(amplitude[i]);
            servo_[i].SetT(period);
            servo_[i].SetPh(phase_diff[i]);
        }
    }

    double ref = millis();
    double end_time = period * cycle + ref;

    while (millis() < end_time) {
        if (stop_requested_) return;  // 紧急停止：立即退出振荡
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1) {
                servo_[i].Refresh();
            }
        }
        vTaskDelay(5);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Flashduck::Execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                   double phase_diff[SERVO_COUNT], float steps = 1.0) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    int cycles = (int)steps;

    //-- Execute complete cycles
    if (cycles >= 1)
        for (int i = 0; i < cycles; i++)
            OscillateServos(amplitude, offset, period, phase_diff);

    //-- Execute the final not complete cycle
    OscillateServos(amplitude, offset, period, phase_diff, (float)steps - cycles);
    vTaskDelay(pdMS_TO_TICKS(10));
}

//---------------------------------------------------------
//-- Execute2: 使用绝对角度作为振荡中心
//--  Parameters:
//--    amplitude: 振幅数组（每个舵机的振荡幅度）
//--    center_angle: 绝对角度数组（0-180度），作为振荡中心位置
//--    period: 周期（毫秒）
//--    phase_diff: 相位差数组（弧度）
//--    steps: 步数/周期数（可为小数）
//---------------------------------------------------------
void Flashduck::Execute2(int amplitude[SERVO_COUNT], int center_angle[SERVO_COUNT], int period,
                    double phase_diff[SERVO_COUNT], float steps = 1.0) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    // 将绝对角度转换为offset（offset = center_angle - 90）
    int offset[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; i++) {
        offset[i] = center_angle[i] - 90;
    }

    int cycles = (int)steps;

    //-- Execute complete cycles
    if (cycles >= 1)
        for (int i = 0; i < cycles; i++)
            OscillateServos(amplitude, offset, period, phase_diff);

    //-- Execute the final not complete cycle
    OscillateServos(amplitude, offset, period, phase_diff, (float)steps - cycles);
    vTaskDelay(pdMS_TO_TICKS(10));
}

///////////////////////////////////////////////////////////////////
//-- HOME = Otto at rest position -------------------------------//
//--   head_reset: true=脖子/嘴巴复位到中立(90/135); false=保持当前头部位置
///////////////////////////////////////////////////////////////////
void Flashduck::Home(bool head_reset) {
    if (is_otto_resting_ == false) {  // Go to rest position only if necessary
        int homes[SERVO_COUNT];
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (i == NECK) {
                homes[i] = head_reset ? NECK_CENTER : servo_[i].GetPosition();
            } else if (i == MOUTH) {
                homes[i] = head_reset ? MOUTH_CLOSED : servo_[i].GetPosition();
            } else {
                // 腿部和脚部舵机始终复位
                homes[i] = 90;
            }
        }

        MoveServos(1200, homes);
        is_otto_resting_ = true;
    }

    vTaskDelay(pdMS_TO_TICKS(200));
}

bool Flashduck::GetRestState() {
    return is_otto_resting_;
}

void Flashduck::SetRestState(bool state) {
    is_otto_resting_ = state;
}

///////////////////////////////////////////////////////////////////
//-- PREDETERMINED MOTION SEQUENCES -----------------------------//
///////////////////////////////////////////////////////////////////
//-- Otto movement: Jump
//--  Parameters:
//--    steps: Number of steps
//--    T: Period
//---------------------------------------------------------
void Flashduck::Jump(float steps, int period) {
    int up[SERVO_COUNT] = {90, 90, 150, 30, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(period, up);
    int down[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(period, down);
}

//---------------------------------------------------------
//-- Otto gait: Walking  (forward or backward)
//--  Parameters:
//--    * steps:  Number of steps
//--    * T : Period
//--    * Dir: Direction: FORWARD / BACKWARD
//--    * head_swing: 脖子摆动幅度, 0表示不摆头
//---------------------------------------------------------
void Flashduck::Walk(float steps, int period, int dir, int head_swing, int hip_offset,
                     int hip_amp, int foot_amp, int phase_deg) {
    //-- 宽站距步态：
    //-- 1) 两个髋关节先向身体中线内收 hip_offset，把支撑面收窄；
    //-- 2) 髋部大幅左右摆动，把重心移到支撑脚；
    //-- 3) 脚踝前后摆动产生推进；phase_deg 控制抬脚/推地的时序。
    hip_offset = std::clamp(hip_offset, 0, 25);
    hip_amp = std::clamp(hip_amp, 15, 50);
    foot_amp = std::clamp(foot_amp, 15, 50);
    phase_deg = std::clamp(phase_deg, 60, 120);

    int A[SERVO_COUNT] = {hip_amp, hip_amp, foot_amp, foot_amp, 0, 0};
    // 左腿 >90 内收，右腿 <90 内收（两腿镜像）
    int O[SERVO_COUNT] = {hip_offset, -hip_offset, 5, -5, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(dir * -phase_deg),
                                      DEG2RAD(dir * -phase_deg), 0, 0};

    // 如果head_swing>0且有头部舵机，脖子随步伐自然摆动（环顾）
    if (head_swing > 0 && has_head_) {
        A[NECK] = std::min(head_swing, 45);
        // 脖子与右腿同相，走路时自然环顾
        phase_diff[NECK] = phase_diff[RIGHT_LEG];
    }

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Turning (left or right)
//--  Parameters:
//--   * Steps: Number of steps
//--   * T: Period
//--   * Dir: Direction: LEFT / RIGHT
//--   * head_swing: 脖子摆动幅度, 0表示不摆头
//---------------------------------------------------------
void Flashduck::Turn(float steps, int period, int dir, int head_swing, int hip_offset,
                     int hip_amp, int foot_amp, int phase_deg) {
    //-- Same coordination than for walking (see Flashduck::walk)
    //-- The Amplitudes of the hip's oscillators are not igual
    //-- When the right hip servo amplitude is higher, the steps taken by
    //--   the right leg are bigger than the left. So, the robot describes an
    //--   left arc
    hip_offset = std::clamp(hip_offset, 0, 25);
    hip_amp = std::clamp(hip_amp, 15, 50);
    foot_amp = std::clamp(foot_amp, 15, 50);
    phase_deg = std::clamp(phase_deg, 60, 120);

    int A[SERVO_COUNT] = {hip_amp, hip_amp, foot_amp, foot_amp, 0, 0};
    int O[SERVO_COUNT] = {hip_offset, -hip_offset, 5, -5, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(-phase_deg), DEG2RAD(-phase_deg), 0, 0};

    if (dir == LEFT) {
        A[0] = hip_amp;  //-- Left hip servo
        A[1] = 0;        //-- Right hip servo
    } else {
        A[0] = 0;
        A[1] = hip_amp;
    }

    // 转向时脖子跟随同侧腿摆动，增强转向效果
    if (head_swing > 0 && has_head_) {
        A[NECK] = std::min(head_swing, 45);
        phase_diff[NECK] = phase_diff[LEFT_LEG];
    }

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Lateral bend
//--  Parameters:
//--    steps: Number of bends
//--    T: Period of one bend
//--    dir: RIGHT=Right bend LEFT=Left bend
//---------------------------------------------------------
void Flashduck::Bend(int steps, int period, int dir) {
    // Parameters of all the movements. Default: Left bend
    int bend1[SERVO_COUNT] = {90, 90, 62, 35, NECK_CENTER, MOUTH_CLOSED};
    int bend2[SERVO_COUNT] = {90, 90, 62, 105, NECK_CENTER, MOUTH_CLOSED};
    int homes[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};

    // Time of one bend, constrained in order to avoid movements too fast.
    // T=max(T, 600);
    // Changes in the parameters if right direction is chosen
    if (dir == -1) {
        bend1[2] = 180 - 35;
        bend1[3] = 180 - 60;  // Not 65. Otto is unbalanced
        bend2[2] = 180 - 105;
        bend2[3] = 180 - 60;
    }

    // Time of the bend movement. Fixed parameter to avoid falls
    int T2 = 800;

    // Bend movement
    for (int i = 0; i < steps; i++) {
        MoveServos(T2 / 2, bend1);
        MoveServos(T2 / 2, bend2);
        vTaskDelay(pdMS_TO_TICKS(period * 0.8));
        MoveServos(500, homes);
    }
}

//---------------------------------------------------------
//-- Otto gait: Shake a leg
//--  Parameters:
//--    steps: Number of shakes
//--    T: Period of one shake
//--    dir: RIGHT=Right leg LEFT=Left leg
//---------------------------------------------------------
void Flashduck::ShakeLeg(int steps, int period, int dir) {
    // This variable change the amount of shakes
    int numberLegMoves = 2;

    // Parameters of all the movements. Default: Right leg
    int shake_leg1[SERVO_COUNT] = {90, 90, 58, 35, NECK_CENTER, MOUTH_CLOSED};
    int shake_leg2[SERVO_COUNT] = {90, 90, 58, 120, NECK_CENTER, MOUTH_CLOSED};
    int shake_leg3[SERVO_COUNT] = {90, 90, 58, 60, NECK_CENTER, MOUTH_CLOSED};
    int homes[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};

    // Changes in the parameters if left leg is chosen
    if (dir == LEFT) {
        shake_leg1[2] = 180 - 35;
        shake_leg1[3] = 180 - 58;
        shake_leg2[2] = 180 - 120;
        shake_leg2[3] = 180 - 58;
        shake_leg3[2] = 180 - 60;
        shake_leg3[3] = 180 - 58;
    }

    // Time of the bend movement. Fixed parameter to avoid falls
    int T2 = 1000;
    // Time of one shake, constrained in order to avoid movements too fast.
    period = period - T2;
    period = std::max(period, 200 * numberLegMoves);

    for (int j = 0; j < steps; j++) {
        // Bend movement
        MoveServos(T2 / 2, shake_leg1);
        MoveServos(T2 / 2, shake_leg2);

        // Shake movement
        for (int i = 0; i < numberLegMoves; i++) {
            MoveServos(period / (2 * numberLegMoves), shake_leg3);
            MoveServos(period / (2 * numberLegMoves), shake_leg2);
        }
        MoveServos(500, homes);  // Return to home position
    }

    vTaskDelay(pdMS_TO_TICKS(period));
}

//---------------------------------------------------------
//-- Otto movement: Sit (坐下)
//---------------------------------------------------------
void Flashduck::Sit() {
    int target[SERVO_COUNT] = {120, 60, 0, 180, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(600, target);
}

//---------------------------------------------------------
//-- Otto movement: up & down
//--  Parameters:
//--    * steps: Number of jumps
//--    * T: Period
//--    * h: Jump height: SMALL / MEDIUM / BIG
//--              (or a number in degrees 0 - 90)
//---------------------------------------------------------
void Flashduck::UpDown(float steps, int period, int height) {
    //-- Both feet are 180 degrees out of phase
    //-- Feet amplitude and offset are the same
    //-- Initial phase for the right foot is -90, so that it starts
    //--   in one extreme position (not in the middle)
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {FLASHDUCK_WALK_HIP_OFFSET, -FLASHDUCK_WALK_HIP_OFFSET, height, -height,
                          NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(-90), DEG2RAD(90), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto movement: swinging side to side
//--  Parameters:
//--     steps: Number of steps
//--     T : Period
//--     h : Amount of swing (from 0 to 50 aprox)
//---------------------------------------------------------
void Flashduck::Swing(float steps, int period, int height) {
    //-- Both feets are in phase. The offset is half the amplitude
    //-- It causes the robot to swing from side to side
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {FLASHDUCK_WALK_HIP_OFFSET, -FLASHDUCK_WALK_HIP_OFFSET, height / 2,
                          -height / 2, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(0), DEG2RAD(0), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto movement: swinging side to side without touching the floor with the heel
//--  Parameters:
//--     steps: Number of steps
//--     T : Period
//--     h : Amount of swing (from 0 to 50 aprox)
//---------------------------------------------------------
void Flashduck::TiptoeSwing(float steps, int period, int height) {
    //-- Both feets are in phase. The offset is not half the amplitude in order to tiptoe
    //-- It causes the robot to swing from side to side
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height, -height, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Jitter
//--  Parameters:
//--    steps: Number of jitters
//--    T: Period of one jitter
//--    h: height (Values between 5 - 25)
//---------------------------------------------------------
void Flashduck::Jitter(float steps, int period, int height) {
    //-- Both feet are 180 degrees out of phase
    //-- Feet amplitude and offset are the same
    //-- Initial phase for the right foot is -90, so that it starts
    //--   in one extreme position (not in the middle)
    //-- h is constrained to avoid hit the feets
    height = std::min(25, height);
    int A[SERVO_COUNT] = {height, height, 0, 0, 0, 0};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(-90), DEG2RAD(90), 0, 0, 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Ascending & turn (Jitter while up&down)
//--  Parameters:
//--    steps: Number of bends
//--    T: Period of one bend
//--    h: height (Values between 5 - 15)
//---------------------------------------------------------
void Flashduck::AscendingTurn(float steps, int period, int height) {
    //-- Both feet and legs are 180 degrees out of phase
    //-- Initial phase for the right foot is -90, so that it starts
    //--   in one extreme position (not in the middle)
    //-- h is constrained to avoid hit the feets
    height = std::min(13, height);
    int A[SERVO_COUNT] = {height, height, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height + 4, -height + 4, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(-90), DEG2RAD(90), DEG2RAD(-90), DEG2RAD(90), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Moonwalker. Otto moves like Michael Jackson
//--  Parameters:
//--    Steps: Number of steps
//--    T: Period
//--    h: Height. Typical valures between 15 and 40
//--    dir: Direction: LEFT / RIGHT
//---------------------------------------------------------
void Flashduck::Moonwalker(float steps, int period, int height, int dir) {
    //-- This motion is similar to that of the caterpillar robots: A travelling
    //-- wave moving from one side to another
    //-- The two Otto's feet are equivalent to a minimal configuration. It is known
    //-- that 2 servos can move like a worm if they are 120 degrees out of phase
    //-- In the example of Otto, the two feet are mirrored so that we have:
    //--    180 - 120 = 60 degrees. The actual phase difference given to the oscillators
    //--  is 60 degrees.
    //--  Both amplitudes are equal. The offset is half the amplitud plus a little bit of
    //-   offset so that the robot tiptoe lightly

    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height / 2 + 2, -height / 2 - 2, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    int phi = -dir * 90;
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(phi), DEG2RAD(-60 * dir + phi), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//----------------------------------------------------------
//-- Otto gait: Crusaito. A mixture between moonwalker and walk
//--   Parameters:
//--     steps: Number of steps
//--     T: Period
//--     h: height (Values between 20 - 50)
//--     dir:  Direction: LEFT / RIGHT
//-----------------------------------------------------------
void Flashduck::Crusaito(float steps, int period, int height, int dir) {
    int A[SERVO_COUNT] = {25, 25, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height / 2 + 4, -height / 2 - 4, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {90, 90, DEG2RAD(0), DEG2RAD(-60 * dir), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: Flapping
//--  Parameters:
//--    steps: Number of steps
//--    T: Period
//--    h: height (Values between 10 - 30)
//--    dir: direction: FOREWARD, BACKWARD
//---------------------------------------------------------
void Flashduck::Flapping(float steps, int period, int height, int dir) {
    int A[SERVO_COUNT] = {12, 12, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height - 10, -height + 10, NECK_CENTER - 90, MOUTH_CLOSED - 90};
    double phase_diff[SERVO_COUNT] = {
        DEG2RAD(0), DEG2RAD(180), DEG2RAD(-90 * dir), DEG2RAD(90 * dir), 0, 0};

    //-- Let's oscillate the servos!
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- Otto gait: WhirlwindLeg (旋风腿)
//--   Parameters:
//--     steps: Number of steps
//--     period: Period (建议100-800毫秒)
//--     amplitude: amplitude (Values between 20 - 40)
//---------------------------------------------------------
void Flashduck::WhirlwindLeg(float steps, int period, int amplitude) {
    int target[SERVO_COUNT] = {90, 90, 180, 90, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(100, target);
    target[RIGHT_FOOT] = 160;
    MoveServos(500, target);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 旋风腿配合脖子小幅摆动，增强表现力
    int C[SERVO_COUNT] = {90, 90, 180, 160, NECK_CENTER, MOUTH_CLOSED};
    int A[SERVO_COUNT] = {amplitude, 0, 0, 0, std::min(amplitude, 45), 0};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(20), 0, 0, 0, DEG2RAD(20), 0};
    Execute2(A, C, period, phase_diff, steps);
}

///////////////////////////////////////////////////////////////////
//-- 脖子动作 ----------------------------------------------------//
///////////////////////////////////////////////////////////////////
//-- 脖子动作: 转头
//--  Parameters:
//--    dir: 方向 LEFT=左转, RIGHT=右转, BOTH/0=回正
//--    period: 动作时间
//---------------------------------------------------------
void Flashduck::HeadTurn(int dir, int period) {
    if (!has_head_) {
        return;
    }

    int target[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};

    if (dir == LEFT) {
        target[NECK] = NECK_LEFT_ANGLE;
    } else if (dir == RIGHT) {
        target[NECK] = NECK_RIGHT_ANGLE;
    } else {
        target[NECK] = NECK_CENTER;  // BOTH: 回正
    }

    MoveServos(period, target);
}

//---------------------------------------------------------
//-- 脖子动作: 摇头（否定）
//--  Parameters:
//--    steps: 摇头次数
//--    period: 周期（毫秒）
//--    amplitude: 振荡幅度（度，内部钳到 ≤90）
//---------------------------------------------------------
void Flashduck::IdleLook(int neck_delta, int mouth_cycles, int mouth_amplitude, int move_ms,
                         int open_ms, int close_ms) {
    if (!has_head_) {
        return;
    }

    int base[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; ++i) {
        base[i] = (servo_pins_[i] != -1) ? servo_[i].GetPosition() : 90;
    }

    int target[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; ++i) {
        target[i] = base[i];
    }

    const int neck = std::clamp(NECK_CENTER + neck_delta, NECK_LEFT_ANGLE, NECK_RIGHT_ANGLE);
    target[NECK] = neck;
    target[MOUTH] = MOUTH_CLOSED;
    MoveServos(std::max(move_ms, 250), target);
    if (stop_requested_) {
        return;
    }

    const int cycles = std::clamp(mouth_cycles, 1, 5);
    const int amplitude = std::clamp(mouth_amplitude, 3, 15);
    for (int i = 0; i < cycles; ++i) {
        target[MOUTH] = std::min(MOUTH_CLOSED + amplitude, MOUTH_MAX_ANGLE);
        MoveServos(std::max(open_ms, 120), target);
        if (stop_requested_) {
            return;
        }
        target[MOUTH] = MOUTH_CLOSED;
        MoveServos(std::max(close_ms, 120), target);
        if (stop_requested_) {
            return;
        }
    }

    target[NECK] = NECK_CENTER;
    target[MOUTH] = MOUTH_CLOSED;
    MoveServos(std::max(move_ms, 300), target);
}

void Flashduck::HeadShake(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }

    amplitude = std::min(amplitude, 90);
    int A[SERVO_COUNT] = {0, 0, 0, 0, amplitude, 0};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, MOUTH_CLOSED - 90};  // 脖子@静止位, 嘴巴@闭合
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};

    Execute(A, O, period, phase_diff, steps);
}

///////////////////////////////////////////////////////////////////
//-- 嘴巴动作 ----------------------------------------------------//
///////////////////////////////////////////////////////////////////
//-- 嘴巴动作: 张嘴
//--  Parameters:
//--    period: 动作时间
//---------------------------------------------------------
void Flashduck::MouthOpen(int period) {
    if (!has_head_) {
        return;
    }

    int target[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_OPEN};
    MoveServos(period, target);
}

//---------------------------------------------------------
//-- 嘴巴动作: 闭嘴
//--  Parameters:
//--    period: 动作时间
//---------------------------------------------------------
void Flashduck::MouthClose(int period) {
    if (!has_head_) {
        return;
    }

    int target[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(period, target);
}

//---------------------------------------------------------
//-- 嘴巴动作: 开合振荡（咀嚼/说话）
//--  Parameters:
//--    steps: 振荡次数
//--    period: 周期（毫秒）
//--    amplitude: 振荡幅度（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::MouthMove(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }

    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    // 振荡中心为闭合位与张开位的中点，保证不超出开合范围
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 0, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};

    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//---------------------------------------------------------
//-- 嘴巴动作: 张嘴语汇（给叫声配音）
//--  只动 MOUTH，其余舵机保持当前位置（不抬头、不动腿）。
//
//  以前是固定 40/50ms 的死循环“张张张张”，像机器在打点。这里改成先攒一串
//  关键帧再播放：每种 style 是一种“张法”，同一 style 换 seed 还会变幅度和时长。
//  上限永远是 MOUTH_OPEN_AMOUNT（45°→60°），不会顶到机械限位。
//---------------------------------------------------------
namespace {

// 轻量 xorshift：不引 esp_random，保证 (style, seed) 可复现，动作任务里也不抢锁。
struct MouthRng {
    uint32_t state;

    explicit MouthRng(uint32_t seed) : state(seed == 0 ? 0x9E3779B9u : seed) {}

    uint32_t Next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    int Range(int lo, int hi) {
        if (hi <= lo) {
            return lo;
        }
        return lo + static_cast<int>(Next() % static_cast<uint32_t>(hi - lo + 1));
    }
};

struct MouthStep {
    int open;  // 从闭合位张开的度数
    int ms;    // 走到这个角度用时（余弦缓动，起末速度都为 0）
};

using MouthSeq = std::vector<MouthStep>;

// 一个完整音节：张开 → 停一下 → 合上
void PushSyllable(MouthSeq& seq, int open, int open_ms, int hold_ms, int close_ms) {
    seq.push_back({open, std::max(open_ms, 60)});
    if (hold_ms > 0) {
        seq.push_back({open, hold_ms});
    }
    seq.push_back({0, std::max(close_ms, 60)});
}

int SeqMs(const MouthSeq& seq) {
    int total = 0;
    for (const auto& step : seq) {
        total += step.ms;
    }
    return total;
}

// 超时就按比例压缩；慢了不强行拉长——宁可提前闭嘴，也不把“快两下”拉成慢动作。
// 压到下限（60ms）仍然超长时直接截断：声音已经停了嘴还在动，比少动两下更假。
void FitSeq(MouthSeq& seq, int budget_ms) {
    const int total = SeqMs(seq);
    if (total <= 0 || budget_ms <= 0) {
        return;
    }
    if (total > budget_ms) {
        const float scale = static_cast<float>(budget_ms) / static_cast<float>(total);
        for (auto& step : seq) {
            step.ms = std::max(60, static_cast<int>(static_cast<float>(step.ms) * scale));
        }
    }

    if (SeqMs(seq) <= budget_ms) {
        return;
    }

    MouthSeq trimmed;
    int used = 0;
    for (const auto& step : seq) {
        if (used + step.ms > budget_ms) {
            break;
        }
        trimmed.push_back(step);
        used += step.ms;
    }
    if (trimmed.empty()) {
        trimmed.push_back({0, std::max(budget_ms, 60)});
    } else if (trimmed.back().open != 0) {
        trimmed.push_back({0, 60});
    }
    seq.swap(trimmed);
}

// 18 种张法。幅度 3~15°，单次开合多在 200~700ms，最快的一档也在 4Hz 左右。
void BuildGesture(int style, int budget_ms, MouthRng& rng, MouthSeq& seq) {
    const int b = std::max(budget_ms, 200);
    switch (style) {
        case 0:  // 慢开慢合：慢慢张满，停住，再慢慢合
            PushSyllable(seq, rng.Range(11, 15), b * 35 / 100, b * 15 / 100, b * 45 / 100);
            break;
        case 1:  // 轻点两下
            for (int i = 0; i < 2; ++i) {
                PushSyllable(seq, rng.Range(6, 10), b * 20 / 100, b * 6 / 100, b * 22 / 100);
            }
            break;
        case 2:  // 先快后慢：短促一下，再慢慢张开一次
            PushSyllable(seq, rng.Range(8, 12), rng.Range(110, 150), rng.Range(20, 50),
                         rng.Range(90, 130));
            PushSyllable(seq, rng.Range(9, 13), rng.Range(320, 420), rng.Range(120, 200),
                         rng.Range(280, 380));
            break;
        case 3:  // 先小后大：先试一点缝，再张大
            PushSyllable(seq, rng.Range(4, 7), rng.Range(140, 200), rng.Range(40, 80),
                         rng.Range(120, 180));
            PushSyllable(seq, rng.Range(12, 15), rng.Range(280, 380), rng.Range(100, 180),
                         rng.Range(260, 360));
            break;
        case 4:  // 先大后小
            PushSyllable(seq, rng.Range(12, 15), rng.Range(260, 340), rng.Range(80, 150),
                         rng.Range(240, 320));
            PushSyllable(seq, rng.Range(5, 8), rng.Range(150, 220), rng.Range(40, 90),
                         rng.Range(140, 200));
            break;
        case 5:  // 急促三连（小幅度）
            for (int i = 0; i < 3; ++i) {
                PushSyllable(seq, rng.Range(6, 9), rng.Range(110, 140), rng.Range(20, 40),
                             rng.Range(100, 130));
            }
            break;
        case 6:  // 一大两小
            PushSyllable(seq, rng.Range(12, 15), rng.Range(280, 340), rng.Range(100, 160),
                         rng.Range(240, 300));
            for (int i = 0; i < 2; ++i) {
                PushSyllable(seq, rng.Range(5, 8), rng.Range(140, 190), rng.Range(30, 70),
                             rng.Range(130, 180));
            }
            break;
        case 7:  // 渐强：一次比一次张得大
            for (int i = 0; i < 3; ++i) {
                PushSyllable(seq, 5 + i * 4 + rng.Range(0, 2), rng.Range(180, 260),
                             rng.Range(40, 90), rng.Range(170, 240));
            }
            break;
        case 8:  // 渐弱：先大，再越来越轻
            for (int i = 0; i < 3; ++i) {
                PushSyllable(seq, 13 - i * 4 + rng.Range(0, 1), rng.Range(200, 280),
                             rng.Range(40, 90), rng.Range(190, 260));
            }
            break;
        case 9:  // 长开短合：慢慢张满，突然合上
            PushSyllable(seq, rng.Range(11, 15), b * 40 / 100, b * 15 / 100, rng.Range(120, 180));
            break;
        case 10:  // 短开长合：一下张开，慢慢合拢
            PushSyllable(seq, rng.Range(11, 15), rng.Range(110, 160), b * 10 / 100,
                         b * 50 / 100);
            break;
        case 11:  // 双峰：张 → 半合 → 再张 → 合（“啊—啊”）
            seq.push_back({rng.Range(10, 13), rng.Range(180, 240)});
            seq.push_back({rng.Range(2, 5), rng.Range(100, 140)});
            seq.push_back({rng.Range(11, 15), rng.Range(160, 220)});
            seq.push_back({0, rng.Range(260, 340)});
            break;
        case 12:  // 颤动：张着的时候轻轻抖
            seq.push_back({rng.Range(9, 12), rng.Range(220, 300)});
            for (int i = 0; i < 4; ++i) {
                seq.push_back({rng.Range(6, 10), rng.Range(100, 140)});
                seq.push_back({rng.Range(10, 13), rng.Range(100, 140)});
            }
            seq.push_back({0, rng.Range(260, 340)});
            break;
        case 13:  // 深呼吸式两次
            for (int i = 0; i < 2; ++i) {
                PushSyllable(seq, rng.Range(10, 14), b * 18 / 100, b * 8 / 100, b * 22 / 100);
            }
            break;
        case 14:  // 三快一慢
            for (int i = 0; i < 3; ++i) {
                PushSyllable(seq, rng.Range(5, 8), rng.Range(110, 140), rng.Range(20, 50),
                             rng.Range(100, 130));
            }
            PushSyllable(seq, rng.Range(11, 14), rng.Range(300, 380), rng.Range(100, 160),
                         rng.Range(260, 340));
            break;
        case 15:  // 随机细碎：一会儿大一会儿小
            for (int i = 0, n = rng.Range(4, 6); i < n; ++i) {
                PushSyllable(seq, rng.Range(4, 14), rng.Range(120, 260), rng.Range(20, 90),
                             rng.Range(110, 240));
            }
            break;
        case 16:  // 停一下再张大：抿一小口，愣一下，然后张满
            PushSyllable(seq, rng.Range(3, 5), rng.Range(140, 180), rng.Range(40, 80),
                         rng.Range(120, 160));
            seq.push_back({0, rng.Range(180, 260)});
            PushSyllable(seq, rng.Range(12, 15), rng.Range(320, 420), rng.Range(100, 180),
                         rng.Range(280, 360));
            break;
        default:  // 17 轻声咂嘴：多次小小地开合
            for (int i = 0, n = rng.Range(4, 5); i < n; ++i) {
                PushSyllable(seq, rng.Range(3, 6), rng.Range(150, 200), rng.Range(30, 60),
                             rng.Range(140, 190));
            }
            break;
    }
}


// ---- 说话手势：嘴巴轨道 + 脖子轨道，最后按时间轴合并播放 ----

struct MouthKey {
    int t;    // 绝对时间（ms）
    int deg;  // 从闭合位张开的度数
};

struct NeckMove {
    int t0;
    int t1;
    int from;  // 相对中位 135° 的偏移
    int to;
};

struct MoodCfg {
    int amp_lo, amp_hi;      // 摆幅
    int fast_lo, fast_hi;    // 快档时长
    int slow_lo, slow_hi;    // 慢档时长
    int pause_lo, pause_hi;  // 两次转头之间的停顿
    int moves_lo, moves_hi;  // 转头次数
};

MoodCfg MoodConfig(int mood) {
    switch (mood) {
        case DUCK_MOOD_HAPPY:
            return {12, 28, 200, 320, 320, 460, 120, 380, 3, 4};
        case DUCK_MOOD_CURIOUS:
            return {8, 20, 320, 420, 420, 650, 350, 800, 1, 2};
        case DUCK_MOOD_ANGRY:
            return {16, 34, 150, 240, 240, 360, 80, 220, 3, 4};
        case DUCK_MOOD_SAD:
            return {6, 14, 420, 520, 520, 850, 300, 800, 1, 2};
        case DUCK_MOOD_SURPRISED:
            return {22, 38, 180, 260, 260, 380, 400, 900, 1, 2};
        case DUCK_MOOD_NEUTRAL:
        default:
            return {9, 20, 260, 380, 380, 560, 200, 600, 2, 3};
    }
}

// 嘴巴轨道：长叫声切段换张法，段间留一点闭嘴的停顿。
void BuildMouthTrack(int style, int duration_ms, MouthRng& rng, std::vector<MouthKey>& out) {
    int t = 0;
    int chunk = 0;
    while (t < duration_ms - 180 && chunk < 6) {
        const int left = duration_ms - t;
        const int slot = left > 2400 ? rng.Range(900, 1500) : left;
        const int use_style = chunk == 0 ? style : rng.Range(0, MOUTH_GESTURE_STYLE_COUNT - 1);

        MouthSeq seq;
        BuildGesture(use_style, slot, rng, seq);
        FitSeq(seq, slot);
        for (const auto& step : seq) {
            out.push_back({t, step.open});
            t += step.ms;
        }
        ++chunk;
        if (duration_ms - t > 500) {
            t += rng.Range(90, 180);  // 闭嘴停一下，下面的叫声才分得开
        }
    }
    if (out.empty() || out.back().deg != 0) {
        out.push_back({t, 0});
    }
}

// 脖子轨道：小角度、方向来回、快慢混着来，中间留停顿，最后回正。
void BuildNeckTrack(int mood, int duration_ms, MouthRng& rng, std::vector<NeckMove>& out) {
    const MoodCfg cfg = MoodConfig(mood);
    const int reserve = 320;  // 末尾留给回正
    int t = rng.Range(0, 120);
    int pos = 0;
    int dir = rng.Range(0, 1) == 0 ? -1 : 1;
    const int moves = rng.Range(cfg.moves_lo, cfg.moves_hi);

    for (int i = 0; i < moves; ++i) {
        if (i > 0 && rng.Range(0, 3) != 0) {
            dir = -dir;  // 多数时候来回转，偶尔连着同向
        }
        const int amp = rng.Range(cfg.amp_lo, cfg.amp_hi);
        const int target = std::clamp(dir * amp, -40, 40);
        const int dur = rng.Range(cfg.fast_lo, cfg.slow_hi);
        if (t + dur > duration_ms - reserve) {
            break;
        }
        out.push_back({t, t + dur, pos, target});
        pos = target;
        t += dur;
        if (i + 1 < moves) {
            const int pause = rng.Range(cfg.pause_lo, cfg.pause_hi);
            if (t + pause < duration_ms - reserve) {
                t += pause;
            }
        }
    }

    const int back_start = std::min(t, std::max(0, duration_ms - reserve));
    if (back_start < duration_ms && pos != 0) {
        out.push_back({back_start, duration_ms, pos, 0});
    }
}

}  // namespace

void Flashduck::SpeakGesture(int style, int duration_ms, int mood, uint32_t seed) {
    if (!has_head_) {
        return;
    }

    duration_ms = std::clamp(duration_ms, 300, 8000);
    style = std::clamp(style, 0, MOUTH_GESTURE_STYLE_COUNT - 1);
    mood = std::clamp(mood, 0, static_cast<int>(DUCK_MOOD_COUNT) - 1);

    MouthRng rng(seed);
    int target[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; ++i) {
        target[i] = (servo_pins_[i] != -1) ? servo_[i].GetPosition() : 90;
    }

    std::vector<MouthKey> mouth;
    std::vector<NeckMove> neck;
    BuildMouthTrack(style, duration_ms, rng, mouth);
    BuildNeckTrack(mood, duration_ms, rng, neck);

    // 两条轨道按时间轴合并：每段同时设好嘴和脖子的目标，再交给 MoveServos。
    std::vector<int> times;
    times.reserve(mouth.size() + neck.size() * 2 + 2);
    for (const auto& key : mouth) {
        if (key.t < duration_ms) {
            times.push_back(key.t);
        }
    }
    for (const auto& move : neck) {
        if (move.t0 < duration_ms) {
            times.push_back(move.t0);
        }
        times.push_back(std::min(move.t1, duration_ms));
    }
    times.push_back(0);
    times.push_back(duration_ms);
    std::sort(times.begin(), times.end());
    times.erase(std::unique(times.begin(), times.end()), times.end());

    size_t mi = 0;
    size_t ni = 0;
    for (size_t i = 0; i + 1 < times.size(); ++i) {
        const int a = times[i];
        const int b = times[i + 1];
        if (b <= a) {
            continue;
        }

        while (mi + 1 < mouth.size() && mouth[mi + 1].t <= a) {
            ++mi;
        }
        while (ni + 1 < neck.size() && neck[ni].t1 <= a) {
            ++ni;
        }

        const int mdeg = mouth.empty() ? 0 : mouth[mi].deg;
        int ndeg = 0;
        if (!neck.empty()) {
            const NeckMove& move = neck[ni];
            if (a < move.t0) {
                ndeg = move.from;
            } else if (a >= move.t1) {
                ndeg = move.to;
            } else {
                const float k = static_cast<float>(a - move.t0) / (move.t1 - move.t0);
                ndeg = static_cast<int>(lroundf(move.from + (move.to - move.from) * k));
            }
        }

        target[MOUTH] = MOUTH_CLOSED + std::clamp(mdeg, 0, MOUTH_OPEN_AMOUNT);
        target[NECK] = std::clamp(NECK_CENTER + ndeg, NECK_LEFT_ANGLE, NECK_RIGHT_ANGLE);
        MoveServos(b - a, target);
        if (gesture_abort_ || stop_requested_) {
            break;
        }
    }

    // 被指令打断时也要把嘴合上，别张着嘴去走路
    target[MOUTH] = MOUTH_CLOSED;
    MoveServos(140, target);
}

///////////////////////////////////////////////////////////////////
//-- 组合表情动作 ------------------------------------------------//
///////////////////////////////////////////////////////////////////
//-- 组合动作: 说话（嘴巴小幅振荡 + 脖子轻微摆动）
//--  Parameters:
//--    steps: 振荡次数
//--    period: 周期（毫秒）
//--    amplitude: 嘴巴振荡幅度（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Talk(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }

    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 15, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), 0};

    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- 组合动作: 大笑（嘴巴大幅开合 + 摇头）
//--  Parameters:
//--    steps: 振荡次数
//--    period: 周期（毫秒）
//--    amplitude: 嘴巴振荡幅度（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Laugh(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }

    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 45, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(0), 0};

    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- 组合动作: 唱歌（嘴巴振荡 + 脖子左右摆 + 身体轻摆）
//--  Parameters:
//--    steps: 振荡次数
//--    period: 周期（毫秒）
//--    amplitude: 嘴巴振荡幅度（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Sing(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }

    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    // 脚轻摆(身体摇摆) + 脖子左右摆 + 嘴巴开合
    int A[SERVO_COUNT] = {0, 0, 20, 20, 30, amplitude};
    int O[SERVO_COUNT] = {0, 0, 10, -10, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), 0};

    Execute(A, O, period, phase_diff, steps);
}

///////////////////////////////////////////////////////////////////
//-- 扩展动作 ----------------------------------------------------//
///////////////////////////////////////////////////////////////////
//-- 环顾：左→正→右→正，缓慢扫视（脖子适度幅度，嘴巴闭合）
//--  Parameters:
//--   * period: 每段移动时间（毫秒，默认1200，慢）
//---------------------------------------------------------
void Flashduck::HeadLookAround(int period) {
    if (!has_head_) {
        return;
    }
    int left[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER - 40, MOUTH_CLOSED};
    int right[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER + 40, MOUTH_CLOSED};
    int center[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(period, left);
    MoveServos(period, center);
    MoveServos(period, right);
    MoveServos(period, center);
}

//-- 打哈欠：略侧头，慢张嘴→停顿→慢闭嘴→回正
//--  Parameters:
//--   * period: 每段移动时间（毫秒，默认1500，很慢）
//---------------------------------------------------------
void Flashduck::Yawn(int period) {
    if (!has_head_) {
        return;
    }
    int side[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER - 20, MOUTH_CLOSED};
    int open[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER - 20, MOUTH_OPEN};
    int close[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER - 20, MOUTH_CLOSED};
    int home2[SERVO_COUNT] = {90, 90, 90, 90, NECK_CENTER, MOUTH_CLOSED};
    MoveServos(period, side);
    MoveServos(period + (period >> 1), open);  // 张嘴更慢
    MoveServos(period, close);
    MoveServos(period, home2);
}

//-- 思考：头略侧偏，嘴巴缓慢小幅开合（踱度状）
//--  Parameters:
//--   * steps: 振荡次数
//--   * period: 周期（毫秒，默认900，偏慢）
//--   * amplitude: 嘴巴振幅（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Think(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }
    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 0, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, (NECK_CENTER - 25) - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
    Execute(A, O, period, phase_diff, steps);
}

//-- 开心：脖子小幅摇头 + 嘴巴开合
//--  Parameters:
//--   * steps: 振荡次数
//--   * period: 周期（毫秒，默认450，中速）
//--   * amplitude: 嘴巴振幅（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Happy(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }
    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 25, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), 0};
    Execute(A, O, period, phase_diff, steps);
}

//-- 困倦：脖子慢晃小幅度 + 嘴巴慢动
//--  Parameters:
//--   * steps: 振荡次数
//--   * period: 周期（毫秒，默认1000，慢）
//--   * amplitude: 嘴巴振幅（度，内部钳到 ≤30）
//---------------------------------------------------------
void Flashduck::Sleepy(float steps, int period, int amplitude) {
    if (!has_head_) {
        return;
    }
    amplitude = std::min(amplitude, MOUTH_MAX_SWING);  // 摆幅≤15，保证不出 [15,45] 限位
    int mouth_center = (MOUTH_CLOSED + MOUTH_OPEN) / 2;
    int A[SERVO_COUNT] = {0, 0, 0, 0, 15, amplitude};
    int O[SERVO_COUNT] = {0, 0, 0, 0, NECK_CENTER - 90, mouth_center - 90};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, DEG2RAD(90)};
    Execute(A, O, period, phase_diff, steps);
}

//---------------------------------------------------------
//-- 展示动作：串联多个动作展示
//---------------------------------------------------------
void Flashduck::Showcase() {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    // 1. 往前走3步（带摆头）
    Walk(3, 1000, FORWARD, 30);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 2. 转头左→右→正
    if (has_head_) {
        HeadTurn(LEFT, 600);
        vTaskDelay(pdMS_TO_TICKS(300));
        HeadTurn(RIGHT, 600);
        vTaskDelay(pdMS_TO_TICKS(300));
        HeadTurn(BOTH, 600);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 3. 张嘴 + 说话
    if (has_head_) {
        MouthOpen(600);
        vTaskDelay(pdMS_TO_TICKS(500));
        Talk(8, 350, 12);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 4. 大笑
    if (has_head_) {
        Laugh(6, 300, 25);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 5. 唱歌
    if (has_head_) {
        Sing(10, 500, 20);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 6. 太空步
    Moonwalker(3, 900, 25, LEFT);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 7. 摇摆
    Swing(3, 1000, 30);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 8. 往后走3步
    Walk(3, 1000, BACKWARD, 30);
}

void Flashduck::SetJointRange(int index, int min_pos, int max_pos) {
    if (index < 0 || index >= SERVO_COUNT) return;
    if (servo_pins_[index] == -1) return;
    servo_[index].SetRange(min_pos, max_pos);
}

void Flashduck::EnableServoLimit(int diff_limit) {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetLimiter(diff_limit);
        }
    }
}

void Flashduck::DisableServoLimit() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].DisableLimiter();
        }
    }
}
