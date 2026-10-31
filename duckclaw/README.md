# DuckClaw on Flashduck

这是跑在 **Flashduck** 上的第二套交互系统：**DuckClaw**。

它和根目录的语音固件不是同一套程序，而是同一块 ESP32-S3 上的另一种玩法：

- **不说话**：通过钉钉消息交流，安静、不打扰；
- **本地 Agent**：纯 C / FreeRTOS，记忆、会话和技能存在本地；
- **22 种情绪**：开心、害羞、思考、惊讶、无聊等，由模型自主触发；
- **关系成长**：从陌生到熟络，屏幕右上角红心表示关系阶段；
- **AI 自主控制舵机**：模型可以决定 6 个关节的角度，做即兴姿势；
- **Captive Portal 配网**：连接设备热点后在浏览器完成 Wi-Fi、钉钉和模型配置。

> 两套系统**不能同时运行**。烧录 DuckClaw 会覆盖根目录的语音固件；想换回来重新烧录语音固件即可。

## 硬件适配

DuckClaw 使用 Flashduck 的官方摄像头板路由：

| 关节 | GPIO | 说明 |
|---|---|---|
| 左腿 | 5 | 前进/后退摆动 |
| 右腿 | 43 | 前进/后退摆动 |
| 左脚 | 6 | 抬脚 |
| 右脚 | 44 | 抬脚 |
| 嘴巴 | 4 | 原左手舵机位，机械限位 **45–60°** |
| 脖子 | 7 | 原右手舵机位，**45–180°**，135° 为正前方 |

- 屏幕：ST7789 240×240，SPI3；
- BOOT：GPIO0；
- 腿部/脚部舵机保持 90° 中立；
- 嘴巴超过 60° 会被限幅，避免损坏结构；
- 原来的“举手/挥手/风车”等人形手臂动作已替换为 **转头、摇头、四处看、嘎嘎、叽叽喳喳、开心摇摆、整理羽毛、啄一啄** 等鸭子动作。

## 动作

`self.otto.action` 支持：

`walk`、`walk_backward`、`turn`、`jump`、`swing`、`updown`、`sit`、
`head_turn`、`head_shake`、`look_around`、`quack`、`chatter`、
`happy_waddle`、`preen`、`peck`、`stretch`、`home`

`self.otto.pose` 可以直接控制六个关节：

- 腿/脚：0–180，90 中立；
- 嘴巴：45–60；
- 脖子：45–180，135 正前；
- 过渡时间：100–3000ms。

## 配置

1. 首次开机或按住 BOOT 上电，进入 `DuckClaw-XXXX` 热点；
2. 手机连接后打开配置页面；
3. 填写 Wi-Fi、钉钉机器人、模型 API 等信息；
4. 保存后设备重启并连接网络。

构建时也可以复制 `main/duckclaw_secrets.h.example` 为 `main/duckclaw_secrets.h`，填入固定配置；该文件已被 `.gitignore` 忽略。

## 编译与烧录

需要 ESP-IDF **6.0.1+**：

```bash
cd duckclaw
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash
```

分区表 `partitions.csv` 和 `spiffs_data/` 已包含在目录内。SPIFFS 里放的是 Agent 提示词、网页配置页和记忆文件。

## 许可

`duckclaw/` 目录沿用上游 **CC BY-NC-SA 4.0**，见 [LICENSE](LICENSE)；根目录的语音固件是 MIT。两套代码的许可证不同，商用或二次分发前请分别确认。

上游项目：<https://github.com/FlashCat-Jordan/DuckClaw>
