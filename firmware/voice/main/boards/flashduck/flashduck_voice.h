#pragma once

//-- 本地鸭叫 --------------------------------------------------------------
//  把服务端下发的 TTS 语音整段丢掉，改用 emotion 音效当它的“话”：
//  一次发声 = 播一个情绪音 + 嘴巴按声音长度快速开合。
//  听（ASR）和屏幕字幕都不受影响，只是不再“说人话”。

// 初始化（目前只打印一句日志，保留给以后接 NVS 开关）。
void FlashduckVoiceInit();

// 回复开始：随机挑一个情绪音（不与上一次重复）。
void FlashduckVoiceOnSpeaking();

// 记录服务端给这一轮回复的情绪（happy/sad/angry/…），决定脖子怎么转。
void FlashduckVoiceOnEmotion(const char* emotion);

// 开始听：短促应一声（mmh）。和上一条声音挨太近时自动跳过，避免叠成一团。
void FlashduckVoiceOnListening();

// 唤醒：应一声（curious）。
void FlashduckVoiceOnWakeWord();
