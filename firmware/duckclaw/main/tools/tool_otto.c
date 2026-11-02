#include "tool_registry.h"
#include "otto/otto_movements.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "tool_otto";

extern otto_t g_otto;

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void duck_pose(int time_ms, int mouth, int neck)
{
    int target[SERVO_COUNT] = {90, 90, 90, 90, mouth, neck};
    otto_move_servos(&g_otto, time_ms, target);
}

static bool should_auto_home_action(const char *action_name)
{
    return action_name && strcmp(action_name, "home") != 0;
}

static esp_err_t duck_head_turn(int direction, int speed)
{
    int neck = (direction < 0) ? NECK_RIGHT : NECK_LEFT;
    duck_pose(clamp_int(speed, 250, 900), MOUTH_CLOSED, neck);
    return ESP_OK;
}

static esp_err_t duck_head_shake(int steps, int speed)
{
    steps = clamp_int(steps, 1, 10);
    speed = clamp_int(speed, 180, 600);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    for (int i = 0; i < steps; i++) {
        duck_pose(speed, MOUTH_CLOSED, NECK_LEFT);
        duck_pose(speed, MOUTH_CLOSED, NECK_RIGHT);
    }
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    return ESP_OK;
}

static esp_err_t duck_look_around(int speed)
{
    speed = clamp_int(speed, 250, 900);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    duck_pose(speed, MOUTH_CLOSED, NECK_LEFT);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    duck_pose(speed, MOUTH_CLOSED, NECK_RIGHT);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    return ESP_OK;
}

static esp_err_t duck_quack(int steps, int speed)
{
    steps = clamp_int(steps, 1, 12);
    speed = clamp_int(speed, 100, 500);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    for (int i = 0; i < steps; i++) {
        duck_pose(speed, MOUTH_OPEN, NECK_CENTER);
        duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    }
    return ESP_OK;
}

static esp_err_t duck_chatter(int steps, int speed)
{
    steps = clamp_int(steps, 2, 20);
    speed = clamp_int(speed, 70, 260);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    for (int i = 0; i < steps; i++) {
        duck_pose(speed, MOUTH_OPEN, NECK_CENTER);
        duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    }
    return ESP_OK;
}

static esp_err_t duck_happy_waddle(int steps, int speed)
{
    steps = clamp_int(steps, 1, 10);
    speed = clamp_int(speed, 180, 600);
    for (int i = 0; i < steps; i++) {
        int left[SERVO_COUNT]  = {85, 95, 80, 100, MOUTH_OPEN, NECK_CENTER};
        int right[SERVO_COUNT] = {95, 85, 100, 80, MOUTH_CLOSED, NECK_CENTER};
        otto_move_servos(&g_otto, speed, left);
        otto_move_servos(&g_otto, speed, right);
    }
    return ESP_OK;
}

static esp_err_t duck_preen(int speed)
{
    speed = clamp_int(speed, 250, 800);
    duck_pose(speed, MOUTH_CLOSED, NECK_LEFT);
    duck_pose(speed, MOUTH_OPEN, NECK_LEFT);
    duck_pose(speed, MOUTH_CLOSED, NECK_LEFT);
    duck_pose(speed, MOUTH_OPEN, NECK_LEFT);
    duck_pose(speed, MOUTH_CLOSED, NECK_CENTER);
    return ESP_OK;
}

static esp_err_t duck_peck(int steps, int speed)
{
    steps = clamp_int(steps, 1, 12);
    speed = clamp_int(speed, 120, 450);
    for (int i = 0; i < steps; i++) {
        int down[SERVO_COUNT] = {80, 100, 85, 95, MOUTH_CLOSED, NECK_CENTER};
        int up[SERVO_COUNT]   = {90, 90, 90, 90, MOUTH_OPEN, NECK_CENTER};
        otto_move_servos(&g_otto, speed, down);
        otto_move_servos(&g_otto, speed, up);
    }
    return ESP_OK;
}

static esp_err_t tool_otto_action_execute(const char *input_json, char *output, size_t output_size)
{
    cJSON *input = cJSON_Parse(input_json);
    if (!input) {
        snprintf(output, output_size, "Error: Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *action = cJSON_GetObjectItem(input, "action");
    if (!cJSON_IsString(action)) {
        cJSON_Delete(input);
        snprintf(output, output_size, "Error: Missing 'action' field");
        return ESP_FAIL;
    }

    const char *action_name = action->valuestring;
    int steps = 3;
    int speed = 1000;
    int direction = 1;
    int amount = 25;

    cJSON *steps_item = cJSON_GetObjectItem(input, "steps");
    if (cJSON_IsNumber(steps_item)) steps = clamp_int(steps_item->valueint, 1, 100);

    cJSON *speed_item = cJSON_GetObjectItem(input, "speed");
    if (cJSON_IsNumber(speed_item)) speed = clamp_int(speed_item->valueint, 70, 3000);

    cJSON *direction_item = cJSON_GetObjectItem(input, "direction");
    if (cJSON_IsNumber(direction_item)) direction = clamp_int(direction_item->valueint, -1, 1);

    cJSON *amount_item = cJSON_GetObjectItem(input, "amount");
    if (cJSON_IsNumber(amount_item)) amount = clamp_int(amount_item->valueint, 0, 170);

    ESP_LOGI(TAG, "Executing action: %s, steps=%d, speed=%d, direction=%d, amount=%d",
             action_name, steps, speed, direction, amount);

    if (strcmp(action_name, "home") == 0) {
        otto_home(&g_otto, true);
        snprintf(output, output_size, "已回到初始位置");
    } else if (strcmp(action_name, "walk") == 0) {
        otto_walk(&g_otto, steps, speed, direction, amount);
        snprintf(output, output_size, "我向前走了 %d 步", steps);
    } else if (strcmp(action_name, "walk_backward") == 0) {
        otto_walk(&g_otto, steps, speed, BACKWARD, amount);
        snprintf(output, output_size, "我向后退了 %d 步", steps);
    } else if (strcmp(action_name, "turn") == 0) {
        otto_turn(&g_otto, steps, speed, direction, amount);
        snprintf(output, output_size, "我向%s转了 %d 度", direction == LEFT ? "左" : "右", steps * 90);
    } else if (strcmp(action_name, "jump") == 0) {
        otto_jump(&g_otto, steps, speed);
        snprintf(output, output_size, "我跳了 %d 下", steps);
    } else if (strcmp(action_name, "swing") == 0) {
        otto_swing(&g_otto, steps, speed, amount);
        snprintf(output, output_size, "我摇摆了 %d 下", steps);
    } else if (strcmp(action_name, "updown") == 0) {
        otto_updown(&g_otto, steps, speed, amount);
        snprintf(output, output_size, "我上下运动了 %d 下", steps);
    } else if (strcmp(action_name, "sit") == 0) {
        otto_sit(&g_otto);
        snprintf(output, output_size, "我坐下了");
    } else if (strcmp(action_name, "moonwalk") == 0) {
        otto_moonwalker(&g_otto, steps, speed, amount, direction);
        snprintf(output, output_size, "我走了几步太空步");
    } else if (strcmp(action_name, "bend") == 0) {
        otto_bend(&g_otto, steps, speed, direction);
        snprintf(output, output_size, "我弯了一下身体");
    } else if (strcmp(action_name, "shake_leg") == 0) {
        otto_shake_leg(&g_otto, steps, speed, direction);
        snprintf(output, output_size, "我摇了摇腿");
    } else if (strcmp(action_name, "head_turn") == 0) {
        duck_head_turn(direction, speed);
        snprintf(output, output_size, "我转头看了看");
    } else if (strcmp(action_name, "head_shake") == 0) {
        duck_head_shake(steps, speed);
        snprintf(output, output_size, "我摇了摇头");
    } else if (strcmp(action_name, "look_around") == 0) {
        duck_look_around(speed);
        snprintf(output, output_size, "我四处看了看");
    } else if (strcmp(action_name, "quack") == 0) {
        duck_quack(steps, speed);
        snprintf(output, output_size, "嘎嘎嘎！");
    } else if (strcmp(action_name, "chatter") == 0) {
        duck_chatter(steps, speed);
        snprintf(output, output_size, "我叽叽喳喳地叫了几声");
    } else if (strcmp(action_name, "happy_waddle") == 0) {
        duck_happy_waddle(steps, speed);
        snprintf(output, output_size, "我开心地摇摇摆摆");
    } else if (strcmp(action_name, "preen") == 0) {
        duck_preen(speed);
        snprintf(output, output_size, "我整理了一下羽毛");
    } else if (strcmp(action_name, "peck") == 0) {
        duck_peck(steps, speed);
        snprintf(output, output_size, "我啄了几下");
    } else if (strcmp(action_name, "stretch") == 0) {
        otto_updown(&g_otto, steps, speed, amount);
        snprintf(output, output_size, "我伸了个懒腰");
    } else {
        snprintf(output, output_size, "未知动作: %s（可用：walk/turn/jump/swing/updown/sit/head_turn/head_shake/look_around/quack/chatter/happy_waddle/preen/peck/stretch/home）", action_name);
        cJSON_Delete(input);
        return ESP_FAIL;
    }

    if (should_auto_home_action(action_name)) {
        otto_home(&g_otto, true);
    }

    cJSON_Delete(input);
    return ESP_OK;
}

static const char *otto_tool_schema =
    "{\"type\":\"object\","
    "\"properties\":{"
        "\"action\":{\"type\":\"string\",\"description\":\"动作名称: walk/turn/jump/swing/updown/sit/head_turn/head_shake/look_around/quack/chatter/happy_waddle/preen/peck/stretch/home\"},"
        "\"steps\":{\"type\":\"number\",\"description\":\"动作次数1-100，默认3\"},"
        "\"speed\":{\"type\":\"number\",\"description\":\"速度70-3000，越小越快，默认1000\"},"
        "\"direction\":{\"type\":\"number\",\"description\":\"方向: 1=前进/左转，-1=后退/右转，0=居中\"},"
        "\"amount\":{\"type\":\"number\",\"description\":\"幅度0-170，默认25\"}"
    "},"
    "\"required\":[\"action\"]"
    "}";

static const char *otto_pose_schema =
    "{\"type\":\"object\","
    "\"properties\":{"
        "\"left_leg\":{\"type\":\"number\",\"description\":\"左腿舵机角度 0-180，默认90\"},"
        "\"right_leg\":{\"type\":\"number\",\"description\":\"右腿舵机角度 0-180，默认90\"},"
        "\"left_foot\":{\"type\":\"number\",\"description\":\"左脚舵机角度 0-180，默认90\"},"
        "\"right_foot\":{\"type\":\"number\",\"description\":\"右脚舵机角度 0-180，默认90\"},"
        "\"mouth\":{\"type\":\"number\",\"description\":\"嘴巴开合 45-60，45=闭合，60=张开，默认45\"},"
        "\"neck\":{\"type\":\"number\",\"description\":\"脖子左右 45-180，135=正前方，45=左转，180=右转，默认135\"},"
        "\"time\":{\"type\":\"number\",\"description\":\"过渡时间毫秒 100-3000，默认700\"}"
    "},"
    "\"required\":[]"
    "}";

static esp_err_t tool_otto_pose_execute(const char *input_json, char *output, size_t output_size)
{
    cJSON *input = cJSON_Parse(input_json);
    if (!input) {
        snprintf(output, output_size, "Error: Invalid JSON");
        return ESP_FAIL;
    }

    int targets[SERVO_COUNT] = {90, 90, 90, 90, MOUTH_CLOSED, NECK_CENTER};
    int time_ms = 700;

    cJSON *ll = cJSON_GetObjectItem(input, "left_leg");
    cJSON *rl = cJSON_GetObjectItem(input, "right_leg");
    cJSON *lf = cJSON_GetObjectItem(input, "left_foot");
    cJSON *rf = cJSON_GetObjectItem(input, "right_foot");
    cJSON *mouth = cJSON_GetObjectItem(input, "mouth");
    cJSON *neck = cJSON_GetObjectItem(input, "neck");
    /* Keep old hand field names as aliases for compatibility. */
    if (!cJSON_IsNumber(mouth)) mouth = cJSON_GetObjectItem(input, "left_hand");
    if (!cJSON_IsNumber(neck)) neck = cJSON_GetObjectItem(input, "right_hand");
    cJSON *t = cJSON_GetObjectItem(input, "time");

    if (cJSON_IsNumber(ll)) targets[LEFT_LEG] = clamp_int(ll->valueint, 0, 180);
    if (cJSON_IsNumber(rl)) targets[RIGHT_LEG] = clamp_int(rl->valueint, 0, 180);
    if (cJSON_IsNumber(lf)) targets[LEFT_FOOT] = clamp_int(lf->valueint, 0, 180);
    if (cJSON_IsNumber(rf)) targets[RIGHT_FOOT] = clamp_int(rf->valueint, 0, 180);
    if (cJSON_IsNumber(mouth)) targets[MOUTH] = clamp_int(mouth->valueint, MOUTH_MIN_ANGLE, MOUTH_MAX_ANGLE);
    if (cJSON_IsNumber(neck)) targets[NECK] = clamp_int(neck->valueint, NECK_LEFT, NECK_RIGHT);
    if (cJSON_IsNumber(t)) time_ms = clamp_int(t->valueint, 100, 3000);

    ESP_LOGI(TAG, "Pose: LL=%d RL=%d LF=%d RF=%d mouth=%d neck=%d time=%dms",
             targets[LEFT_LEG], targets[RIGHT_LEG], targets[LEFT_FOOT],
             targets[RIGHT_FOOT], targets[MOUTH], targets[NECK], time_ms);

    otto_move_servos(&g_otto, time_ms, targets);

    snprintf(output, output_size, "舵机姿态: 左腿%d 右腿%d 左脚%d 右脚%d 嘴巴%d 脖子%d 过渡%dms",
             targets[LEFT_LEG], targets[RIGHT_LEG], targets[LEFT_FOOT],
             targets[RIGHT_FOOT], targets[MOUTH], targets[NECK], time_ms);

    cJSON_Delete(input);
    return ESP_OK;
}

void tool_otto_register(void)
{
    ESP_LOGI(TAG, "Registering Flashduck otto tools...");

    duckclaw_tool_t tool = {
        .name = "self.otto.action",
        .description = "控制 Flashduck 执行预定义动作（走、转、跳、头部、嘴巴、鸭子动作）",
        .input_schema_json = otto_tool_schema,
        .execute = tool_otto_action_execute
    };

    duckclaw_tool_t pose_tool = {
        .name = "self.otto.pose",
        .description = "Flashduck 即兴动作工具：控制 4 个腿部舵机 + 嘴巴(45-60) + 脖子(45-180)。"
                       "嘴巴角度超过 60 会被限幅，避免损坏嘴巴结构。",
        .input_schema_json = otto_pose_schema,
        .execute = tool_otto_pose_execute
    };

    tool_registry_register(&tool);
    tool_registry_register(&pose_tool);
    ESP_LOGI(TAG, "Flashduck tools registered (action + pose)");
}
