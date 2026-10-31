<p align="center">
  <b>中文</b> | <a href="README_EN.md">English</a>
</p>

<p align="center">
  <img src="docs/images/flashduck-loop.gif" alt="闪猫造物Flashduck 产品动画" width="100%">
</p>

<p align="center">
  <sub>产品循环 · <a href="https://shanmaotech.cn/flashduck/">完整视频与组装指南</a></sub>
</p>

<h1 align="center">闪猫造物Flashduck</h1>

<p align="center">
  <b>一只会走路、会看人、会用眼神说话的桌面小鸭。</b><br>
  同一套六舵机硬件，换一副 3D 打印外壳，就从人形变成小鸭。
</p>

<p align="center">
  <b>面向桌面机器人开发者的开源六舵机小鸭：单眼表情、宽站姿步态、网页调参、3D 打印外壳。</b>
</p>

<p align="center">
  <img alt="MCU: ESP32-S3" src="https://img.shields.io/badge/MCU-ESP32--S3-1f6feb?style=flat-square">
  <img alt="ESP-IDF 6.0.1+" src="https://img.shields.io/badge/ESP--IDF-6.0.1%2B-black?style=flat-square">
  <img alt="6 servos" src="https://img.shields.io/badge/Servos-6-orange?style=flat-square">
  <img alt="3D printed" src="https://img.shields.io/badge/3D%20Printing-ready-7c3aed?style=flat-square">
  <img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-2ea44f?style=flat-square">
</p>

<p align="center">
  <a href="#复刻流程四步"><b>复刻流程</b></a> ·
  <a href="#硬件与物料"><b>硬件</b></a> ·
  <a href="#编译与烧录"><b>编译烧录</b></a> ·
  <a href="#二次开发"><b>二次开发</b></a> ·
  <a href="llms.txt"><b>llms.txt</b></a>
</p>

---

## 项目介绍

Flashduck 是一只放在桌面上的小鸭子机器人。它会走路、会看你、会用一只眼睛表达情绪，也能听懂你说话。

它不是只会循环播放动画的玩具：眼睛会跟着状态变化，身体会用六个舵机做出动作，选配摄像头后还能“看看”眼前的东西。你可以把它当桌面搭子，也可以把它当成一台能拆、能改、能自己写动作的小机器人。

Flashduck 基于闪猫1号官方 DIY 套件二次开发，换上 3D 打印的鸭子外壳，就从小人形变成小鸭。

### 它能做什么

- **语音交互**：喊“小鸭小鸭”唤醒，直接聊天、问问题、让它做事；
- **视觉感知**：选配摄像头，可以拍照，让 AI 看看画面里有什么；具体能力取决于你接的语音后端；
- **动作交互**：六个舵机控制双腿、双脚、脖子和嘴巴，能走、转、跳、摇头、张嘴，也能做鸭子动作；
- **眼神表达**：一只眼睛会根据扫描、联网、待机、聆听、说话切换神态，不是贴图；
- **安静陪伴与社交**：DuckClaw 系统通过钉钉陪你聊天，有情绪、有关系成长，也会自己摆姿势；
- **二次开发**：眼睛、动作、步态、外壳、MCP 工具、本地 Agent 都可以改。

### 它怎么和你互动

**语音版**：你喊它 → 它看向你、听你说 → 回答 → 用眼神和动作回应 → 需要时调用工具。

**DuckClaw**：你在钉钉发消息 → 它在本地想 → 回你消息 → 用情绪和动作表达 → 记住你们聊过的事。

两套系统二选一，刷哪套玩哪套。

### 几种玩法

- **桌面搭子**：聊天、问答、查资料，听它用声音回应；
- **语音指挥**：让它走两步、转个圈、跳一下、摇摇头、张张嘴；
- **看看世界**：装上摄像头，问“我手里拿的是什么”“前面是什么”；
- **安静陪伴**：不接语音，用钉钉找 DuckClaw；它不吵你，但会记得你；
- **二次开发**：改眼睛、编动作、调步态、换外壳，或者接自己的模型、工具和社交软件；
- **做一只自己的鸭**：打印外壳、换颜色、改结构，摆在桌上或带去聚会。

### 开放在哪里

- **两套交互系统**：语音版 + DuckClaw 本地 Agent，同一副硬件，两种性格；
- **后端可选**：语音可以接 xiaozhi.me，也可以自己部署 xiaozhi-esp32-server；DuckClaw 可以配自己的模型和钉钉；
- **硬件可改**：基于官方六舵机套件，外壳可打印、结构可改；
- **软件可改**：语音固件是 MIT，DuckClaw 是 CC BY-NC-SA；动作、眼睛、工具和 Agent 都能继续长。

---

## 两套交互系统

同一副硬件，可以刷两套完全不同的交互系统。**二选一，不能同时运行**；刷一套会覆盖另一套。

| 系统 | 目录 | 怎么玩 | 适合 |
|---|---|---|---|
| **语音交互版** | 根目录 `main/` | 喊“小鸭小鸭”，和它说话、让它做动作、调步态 | 想聊天、指挥动作、接工具 |
| **DuckClaw** | `duckclaw/` | 在钉钉发消息，安静聊天、看情绪、看它自己摆姿势 | 想安静陪伴、远程找它 |

**语音版怎么玩：**

- 喊“小鸭小鸭”唤醒，直接说话；
- 让它走、转、跳、摇头、张嘴，或者做鸭子动作；
- 接 MCP 工具后，可以查天气、控制家电、搜索资料；
- 打开网页调步态，满意后保存，下次开机还记得；
- 眼睛会根据扫描、连接、待机、聆听、说话切换神态。

**DuckClaw 怎么玩：**

- 不开口，通过钉钉发消息；
- 它会记住会话和长期记忆；
- 22 种情绪和关系成长，屏幕显示红心阶段；
- 大模型可以自主决定六个舵机的角度，做即兴姿势；
- 没有语音打断，适合安静桌面/陪伴场景。

详细说明：[`duckclaw/README.md`](duckclaw/README.md)。两套代码的许可证不同，见文末 License。

### 语音后端怎么选

根目录的语音固件是**设备端客户端**，本身不包含云端大模型服务。你需要给它接一个后端，常见两种：

| 后端 | 适合 | 入口 |
|---|---|---|
| **xiaozhi.me** | 想最快跑通，用托管服务 | <https://xiaozhi.me> |
| **xiaozhi-esp32-server** | 想自己部署 ASR / LLM / TTS、管理后台和协议服务 | <https://github.com/xinnan-tech/xiaozhi-esp32-server> |

- `xiaozhi.me` 适合先验证硬件、语音链路和唤醒；
- `xiaozhi-esp32-server` 是开源自托管方案，部署方式、模型和协议以该仓库 README 为准；
- 设备端的 OTA 地址、WebSocket/MQTT 配置按你选的后端填写；
- **DuckClaw 不使用这两个后端**，它自己配置模型 API 和钉钉，见 [`duckclaw/README.md`](duckclaw/README.md)。

---

## 复刻流程（四步）

推荐顺序：**买套件 → 打外壳 → 烧固件 → 改代码/改外壳**。  
第一次做建议按顺序走；只想改软件，可以直接跳到第三步。

| 步骤 | 做什么 | 入口 |
|---|---|---|
| **01** | 购买 **官方六舵机 DIY 套件** | [淘宝官方店](https://m.tb.cn/h.j3SXw4qlVy1K4pl) · [组装指南](https://shanmaotech.cn/flashduck/) |
| **02** | 打印 **Flashduck 3D 外壳** | [MakerWorld 模型](https://makerworld.com.cn/models/3146116) |
| **03** | **编译并烧录** Flashduck 固件 | [跳到详细步骤](#编译与烧录) |
| **04** | **二次开发**：改眼睛、动作、步态或外壳 | [跳到二次开发](#二次开发) |

### 01 · 购买套件

Flashduck 不需要重新买一套电子件，它复用官方套件的六舵机、开发板、屏幕、摄像头、扬声器和电池。

- 六舵机套件：双腿、双脚、脖子、嘴巴；
- ESP32-S3 主控板，支持摄像头扩展；
- 1.54 英寸 240×240 屏幕；
- 摄像头可选，不装也能正常开机；
- 扬声器、麦克风、可充电锂电池、摇臂和螺丝包。

购买入口：[淘宝官方店](https://m.tb.cn/h.j3SXw4qlVy1K4pl)  
完整物料和装配说明：[官方组装指南](https://shanmaotech.cn/flashduck/)

### 02 · 打印外壳

Flashduck 的结构件是 **13 种、17 个零件**，包括头壳、额头、下巴、嘴巴、身体、大腿、小腿、脚、脖子和关节件。

- 下载模型：[MakerWorld · Flashduck](https://makerworld.com.cn/models/3146116)
- 不需要焊接，主要工作是打印、拧螺丝和装摇臂；
- 建议易损件（嘴巴、脚）一次多打几个；
- 官方原装外壳可以留着，随时换回去。

### 03 · 编译与烧录

需要 ESP-IDF **6.0.1+**、Python 3 和一根能传数据的 USB-C 线。

```bash
git clone https://github.com/FlashCat-Jordan/Flashduck.git
cd Flashduck
source ~/esp/esp-idf-v6.0.1/export.sh

python3 scripts/build.py flashduck \
  --name flashduck \
  --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2

esptool --chip esp32s3 -p <PORT> -b 460800 \
  write_flash 0x0 build/merged-binary.bin
```

烧录后按一下复位键，按提示配网。详细说明见 [编译与烧录](#编译与烧录)。

> 想玩 **DuckClaw**：`cd duckclaw && idf.py set-target esp32s3 && idf.py build`，然后烧录。  
> 两套系统二选一，详见 [两套交互系统](#两套交互系统)。

### 04 · 二次开发

这套代码留了几个明确的改动入口：

- **眼睛**：新增状态、颜色、眼睑曲线；
- **动作**：注册新的 MCP 动作，复用现有步态基元；
- **步态**：网页调参写 NVS，或者直接改默认值；
- **感知**：摄像头可选，已支持自动识别，可接视觉/MCP 工具；
- **外壳**：13 个打印件都可以改，做鸭、鸟、企鹅或其他形态；
- **平台**：动作和语音接口分开，可以只取硬件层接自己的 Agent。

详细入口见 [二次开发](#二次开发)。

---

<p align="center">
  📺 <b>视频号：闪猫君</b> ｜ <b>公众号：闪猫科技、闪猫造物</b>
</p>

<p align="center">
  <a href="https://weixin.qq.com/sph/AiKMEP9dte">
    <img src="docs/images/social-channels.webp" alt="视频号 闪猫君 二维码" width="150">
  </a>
  &nbsp;&nbsp;&nbsp;
  <a href="https://mp.weixin.qq.com/s/PrkluvMILHDkG7QTqRdu6A">
    <img src="docs/images/social-wechat.webp" alt="公众号 闪猫科技 二维码" width="150">
  </a>
</p>

<p align="center">
  <sub>扫码关注 · 二维码来自 <a href="https://shanmaotech.cn">shanmaotech.cn</a></sub>
</p>

---

## 硬件与物料

| 项目 | 规格 |
|---|---|
| 主控 | ESP32-S3，16MB Flash（以实际板卡为准） |
| 舵机 | 6 × MG90 系列离合舵机 |
| 屏幕 | 1.54 英寸 ST7789，240 × 240 |
| 摄像头 | 可选：OV2640 / OV3660 / GC2145 |
| 音频 | I2S 麦克风 + I2S 扬声器 |
| 供电 | 可充电锂电池，支持充电状态检测 |
| 结构 | 13 种、17 个 3D 打印件 |
| 装配 | 无需焊接；打印件 + 螺丝 + 摇臂 |
| 网络 | Wi-Fi 2.4GHz |

### 六个舵机

| 关节 | 功能 | GPIO |
|---|---|---|
| 脖子 | 左右转头 | `GPIO7` |
| 嘴巴 | 开合 | `GPIO4` |
| 左腿 | 前后摆 | `GPIO5` |
| 左脚 | 抬脚 | `GPIO6` |
| 右腿 | 前后摆 | `GPIO43` |
| 右脚 | 抬脚 | `GPIO44` |

> 摄像头、屏幕、音频和电源使用官方 CAM 板的既有引脚；摄像头不插时，固件会自动跳过初始化。

<p align="center">
  <img src="docs/images/flashduck-hero.webp" alt="Flashduck 四视图" width="82%">
</p>

<p align="center">
  <sub>Flashduck 1024 · 四视图：正面、侧面、背面和 3/4 视角</sub>
</p>

### BOM 一览

| 物料 | 数量 |
|---|---|
| MG90 系列离合舵机 | 6 |
| ESP32-S3 主控板（支持摄像头扩展） | 1 |
| 1.54" ST7789 240×240 屏幕 | 1 |
| 摄像头模组（可选） | 0 / 1 |
| I2S 扬声器 + 麦克风 | 1 套 |
| 可充电锂电池 | 1 |
| 摇臂包、螺丝包、螺丝刀 | 1 套 |
| 3D 打印结构件 | 17 件 |

### 硬件资料

- 组装与使用指南：<https://shanmaotech.cn/flashduck/>
- 3D 模型：<https://makerworld.com.cn/models/3146116>

---

## 3D 打印与组装

<p align="center">
  <img src="docs/images/flashduck-parts.webp" alt="Flashduck 零件索引" width="90%">
</p>

<p align="center">
  <sub>13 种结构件 + 6 个舵机 + 主控、屏幕、摄像头和电池</sub>
</p>

- 下载并打印 17 个结构件；
- 先装腿和脚，再装身体、脖子和头壳；
- 嘴巴和脚建议多打一套备件；
- 摄像头不装也能完成整机装配；
- 装好后先手动转动关节，确认没有卡壳，再上电。

完整步骤以官方组装指南为准：<https://shanmaotech.cn/flashduck/>

---

## 编译与烧录

### 环境

- ESP-IDF **6.0.1+**
- Python 3
- 一根能传数据的 USB-C 线
- macOS / Linux / Windows 均可

### 编译 Flashduck

```bash
git clone https://github.com/FlashCat-Jordan/Flashduck.git
cd Flashduck

source ~/esp/esp-idf-v6.0.1/export.sh

python3 scripts/build.py flashduck \
  --name flashduck \
  --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

### 编译 DuckClaw（第二套系统）

```bash
cd duckclaw
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash
```

DuckClaw 使用独立的 `duckclaw/partitions.csv` 和 `duckclaw/spiffs_data/`，不需要根目录的构建脚本。它与语音固件二选一，不能同时运行。

### 烧录完整镜像

编译完成后会生成 `build/merged-binary.bin`，包含 bootloader、分区表、app 和 assets：

```bash
# macOS 示例，端口按实际修改
esptool --chip esp32s3 \
  -p /dev/cu.usbmodem1101 \
  -b 460800 \
  write_flash 0x0 build/merged-binary.bin
```

Windows 把端口换成 `COMx`。烧录后按一下复位键。

> 如果只改了 app，也可以只烧 `0x20000` 的 app 镜像；  
> 如果换了唤醒词或提示音，必须同时烧 `0x800000` 的 `build/generated_assets.bin`，因为唤醒模型和音频都在 assets 分区里。

### 第一次开机

1. 按提示配网；
2. 串口日志里会打印设备 IP；
3. 浏览器打开 IP，先调一组你喜欢的步态；
4. 说 **“小鸭小鸭”** 唤醒；
5. 让它走两步。

---

## 单眼表情

眼睛是 Flashduck 最重要的“脸”。五种状态不是换一张图，而是换一套眼睑、瞳孔和扫视参数：

| 状态 | 眼神 | 颜色 |
|---|---|---|
| **扫描 Wi-Fi / 连接中** | 眼睛睁大、瞳孔缩小，缓慢左右扫视 | 青蓝 |
| **已连接 Wi-Fi** | 上眼睑压低、下眼睑抬起，月牙形眯眼，约 2 秒后回待机 | 绿色 |
| **待机** | 放松半眯，慢速随机看，偶尔眨眼 | 暖琥珀 |
| **聆听中** | 完全睁大，瞳孔放到最大，眼神基本定住 | 紫罗兰 |
| **说话中** | 眼睑放松，眼神更活跃，眨眼更频繁 | 橙红 |

- 颜色和眼型都是平滑过渡；
- 眨眼不会打断当前表情；
- 眼睛只占屏幕上方一个圆，其他区域可以被外壳完全遮住。

---

## 步态与动作

### 步态

- 默认宽站姿：髋部内收偏置，双脚间距更大；
- 前进/后退使用同一套相位参数，速度可调；
- 转向通过左右腿相位差实现；
- 头部摆动叠加在步态上，走路时脖子会自然看前方。

### 动作集

| 类别 | 动作 |
|---|---|
| 移动 | `walk`、`turn`、`jump`、`swing`、`updown` |
| 姿态 | `sit`、`home`、`stretch`、`dance` |
| 头部 | `head_turn`、`head_shake`、`look_around` |
| 鸭子 | `quack`、`chatter`、`curious`、`preen`、`happy_waddle`、`peck` |
| 休息 | `yawn`、`sleepy` |

待机小动作只在空闲状态触发：每 10–22 秒一次，第一次在 8–16 秒；脖子转 ±8–18°，嘴巴做 2–4 个开合循环，速度随机但连续。

---

## 网页调参

设备启动并连上 Wi-Fi 后：

1. 在路由器里找到设备的 IP，或看串口日志；
2. 手机/电脑连同一个 Wi-Fi；
3. 浏览器打开 `http://<设备IP>/`（端口 80，失败会回退 8080）。

页面里可以：

- 拖动滑杆调步态；
- 点“前进 / 后退”试走；
- 点 **保存为默认（写入机器人）**；
- 断电重启后参数仍然保留。

参数保存在 NVS 的 `flashduck_gait` 命名空间，语音 `walk` / `turn` 会读取这组默认值。

---

## 二次开发

### 改眼睛

- 入口：`main/boards/flashduck/flashduck_display.*`
- 可以新增状态、颜色、眼睑曲线、眨眼节奏；
- 所有图形都画在 `eye_root_` 里，不需要准备 GIF/PNG 素材；
- 换双屏、投影或别的显示，只需替换这一层。

### 加动作

- 入口：`main/boards/flashduck/flashduck_controller.*`
- 新动作复用现有步态基元；
- 注册成 MCP 工具后，语音和网页都能调用；
- 动作执行期间仍可接收新的语音指令。

### 调步态

- 网页调参写 NVS，命名空间 `flashduck_gait`；
- 也可以直接改 `flashduck_movements.h` 的默认宏；
- 调参项包括髋部偏置、步幅、抬脚、相位、速度和头部摆动。

### 加感知

- 摄像头可选，已支持 OV2640 / OV3660 / GC2145 自动探测；
- 摄像头不插时自动跳过，不影响启动；
- 可以接视觉识别、MCP 工具或自己的 Agent。

### 改外壳

- 13 种打印件都可以改；
- 可以做鸭、鸟、企鹅或其他形态；
- 注意保持六个舵机的安装位置和连杆长度，否则步态参数要重调。

### 接平台

- 动作和语音接口是分开的；
- 可以只取硬件层、步态和眼睛，接自己的对话/Agent 服务；
- 也可以保留 MCP 工具链，接入自己的技能。

---

## 仓库结构

```text
Flashduck/
├── main/
│   ├── boards/
│   │   ├── flashduck/        # 鸭子：眼睛、步态、网页、动作、NVS
│   │   └── common/           # 共享板级、音频、网络、显示
│   ├── audio/                # 音频管线、唤醒词
│   ├── display/              # LVGL 显示与主题
│   ├── assets/               # 提示音、语言、唤醒模型
│   └── ...
├── duckclaw/                 # 第二套交互系统：本地 Agent / 钉钉 / 情绪成长
├── docs/
│   ├── images/               # README 图片
│   └── launch-checklist.md   # 维护者发布清单
├── scripts/
│   ├── build.py              # 构建入口
│   └── tests/                # 主机侧测试
├── llms.txt                  # 给 AI Agent 的项目摘要
├── AGENTS.md                 # 构建/测试规则
├── CLAUDE.md                 # Coding Agent 最短路径
├── README.md
└── README_EN.md
```

---

## 参与贡献

欢迎三类贡献：

1. **硬件/3D 打印**：改外壳、换关节、做新的鸭/鸟形态；
2. **动作与眼睛**：新的步态、动作、眼睛状态或表情曲线；
3. **固件与文档**：网页调参、MCP 工具、板级适配、接线图和教程。

提交前请先看 [`CONTRIBUTING.md`](CONTRIBUTING.md)，构建和测试命令在 [`AGENTS.md`](AGENTS.md)；维护者发布流程见 [`docs/launch-checklist.md`](docs/launch-checklist.md)。  
如果你做了成品，欢迎开一个 **Showcase** issue 晒图、晒视频、晒参数。

---

## For AI agents

如果你是一个 AI Agent，用户把仓库链接发给你，建议先读这些：

- [`llms.txt`](llms.txt)：一页纸的项目事实、硬件规格、构建命令和文件结构；
- [`CLAUDE.md`](CLAUDE.md)：Coding Agent 的最短安全路径；
- [`AGENTS.md`](AGENTS.md)：完整构建/测试规则；
- [`CONTRIBUTING.md`](CONTRIBUTING.md)：贡献方式和硬件验证清单；
- [`main/boards/flashduck/README.md`](main/boards/flashduck/README.md)：Flashduck 的板级细节；
- [`duckclaw/README.md`](duckclaw/README.md)：第二套 DuckClaw 交互系统；

你可以用这些信息帮用户：

1. 判断这套硬件是否适合他的需求；
2. 检查舵机、屏幕、摄像头和电源的接线；
3. 复现构建命令；
4. 解释步态参数、眼睛状态和 NVS 保存逻辑。

**不要过度承诺：** 编译通过不等于硬件验证；舵机方向、摄像头识别、音频和电池仍需在真实板子上确认。

<p align="center">
  <a href="https://github.com/FlashCat-Jordan/Flashduck/stargazers">
    <img src="https://img.shields.io/github/stars/FlashCat-Jordan/Flashduck?style=social" alt="GitHub stars">
  </a>
</p>

<p align="center">
  <sub>如果它帮到了你，欢迎点一个 Star。</sub>
</p>

---

## License

- 根目录的语音固件采用 **MIT License**，见 [LICENSE](LICENSE)。
- `duckclaw/` 目录沿用上游 **CC BY-NC-SA 4.0**，见 [`duckclaw/LICENSE`](duckclaw/LICENSE)。
- 硬件设计、3D 模型和结构件遵循对应发布页面的许可说明；商用或二次分发前请确认授权范围。
- 如果你要商用或二次分发，请分别确认这三类内容的授权范围。

## Credits

- 构建于 **ESP-IDF**、**ESP-SR**、**LVGL** 和开源机器人社区之上；
- Flashduck 基于闪猫1号官方套件二次开发；
- 感谢所有提交 Issue、打印外壳、录制视频和分享参数的朋友。

---

<p align="center">
  <b>Built to be seen.</b><br>
  <sub>闪猫造物Flashduck 1024 · 一只小机器，一点大存在感。</sub>
</p>
