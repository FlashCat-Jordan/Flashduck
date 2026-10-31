#pragma once

/* DuckClaw Global Configuration */

/* Build-time secrets (highest priority, override NVS) */
#if __has_include("duckclaw_secrets.h")
#include "duckclaw_secrets.h"
#endif

#ifndef DUCKCLAW_SECRET_WIFI_SSID
#define DUCKCLAW_SECRET_WIFI_SSID       ""
#endif
#ifndef DUCKCLAW_SECRET_WIFI_PASS
#define DUCKCLAW_SECRET_WIFI_PASS       ""
#endif
#ifndef DUCKCLAW_SECRET_DINGTALK_APP_KEY
#define DUCKCLAW_SECRET_DINGTALK_APP_KEY    ""
#endif
#ifndef DUCKCLAW_SECRET_DINGTALK_APP_SECRET
#define DUCKCLAW_SECRET_DINGTALK_APP_SECRET ""
#endif
#ifndef DUCKCLAW_SECRET_API_KEY
#define DUCKCLAW_SECRET_API_KEY         ""
#endif
#ifndef DUCKCLAW_SECRET_MODEL
#define DUCKCLAW_SECRET_MODEL           ""
#endif
#ifndef DUCKCLAW_SECRET_MODEL_PROVIDER
#define DUCKCLAW_SECRET_MODEL_PROVIDER  "anthropic"
#endif
#ifndef DUCKCLAW_SECRET_API_BASE_URL
#define DUCKCLAW_SECRET_API_BASE_URL    ""
#endif
#ifndef DUCKCLAW_SECRET_PROXY_HOST
#define DUCKCLAW_SECRET_PROXY_HOST      ""
#endif
#ifndef DUCKCLAW_SECRET_PROXY_PORT
#define DUCKCLAW_SECRET_PROXY_PORT      ""
#endif
#ifndef DUCKCLAW_SECRET_SEARCH_KEY
#define DUCKCLAW_SECRET_SEARCH_KEY      ""
#endif
#ifndef DUCKCLAW_SECRET_ALLOW_FROM
#define DUCKCLAW_SECRET_ALLOW_FROM      ""
#endif
#ifndef DUCKCLAW_SECRET_BAILIAN_APP_ID
#define DUCKCLAW_SECRET_BAILIAN_APP_ID  ""
#endif

/* WiFi */
#define DUCKCLAW_WIFI_MAX_RETRY          10
#define DUCKCLAW_WIFI_RETRY_BASE_MS      1000
#define DUCKCLAW_WIFI_RETRY_MAX_MS       30000

/* DingTalk Bot */
#define DUCKCLAW_DINGTALK_POLL_INTERVAL_MS 5000
#define DUCKCLAW_DINGTALK_POLL_STACK       (12 * 1024)
#define DUCKCLAW_DINGTALK_POLL_PRIO        5
#define DUCKCLAW_DINGTALK_POLL_CORE        0

/* Agent Loop */
#define DUCKCLAW_AGENT_STACK             (12 * 1024)
#define DUCKCLAW_AGENT_PRIO              6
#define DUCKCLAW_AGENT_CORE              1
#define DUCKCLAW_AGENT_MAX_HISTORY       20
#define DUCKCLAW_AGENT_MAX_TOOL_ITER     10
#define DUCKCLAW_MAX_TOOL_CALLS          4

/* Timezone (POSIX TZ format) */
#define DUCKCLAW_TIMEZONE                "CST-8"

/* LLM */
#define DUCKCLAW_LLM_DEFAULT_MODEL       "claude-opus-4-5"
#define DUCKCLAW_LLM_PROVIDER_DEFAULT    "anthropic"
#define DUCKCLAW_LLM_MAX_TOKENS          4096
#define DUCKCLAW_LLM_API_URL             "https://api.anthropic.com/v1/messages"
#define DUCKCLAW_OPENAI_COMPAT_API_URL   "https://api.openai.com/v1/chat/completions"
#define DUCKCLAW_LLM_API_VERSION         "2023-06-01"
#define DUCKCLAW_LLM_STREAM_BUF_SIZE     (32 * 1024)

/* Message Bus */
#define DUCKCLAW_BUS_QUEUE_LEN           8
#define DUCKCLAW_OUTBOUND_STACK          (8 * 1024)
#define DUCKCLAW_OUTBOUND_PRIO           5
#define DUCKCLAW_OUTBOUND_CORE           0

/* Memory / SPIFFS */
#define DUCKCLAW_SPIFFS_BASE             "/spiffs"
#define DUCKCLAW_SPIFFS_CONFIG_DIR       "/spiffs/config"
#define DUCKCLAW_SPIFFS_MEMORY_DIR       "/spiffs/memory"
#define DUCKCLAW_SPIFFS_SESSION_DIR      "/spiffs"
#define DUCKCLAW_SPIFFS_AGENTS_DIR       "/spiffs/agents"
#define DUCKCLAW_SPIFFS_SKILLS_DIR       "/spiffs/skills"
#define DUCKCLAW_MEMORY_FILE             "/spiffs/memory/MEMORY.md"
#define DUCKCLAW_SOUL_FILE               "/spiffs/config/SOUL.md"
#define DUCKCLAW_RELATION_FILE           "/spiffs/config/RELATION.md"
#define DUCKCLAW_USER_FILE               "/spiffs/config/USER.md"
#define DUCKCLAW_AGENTS_FILE             "/spiffs/config/AGENTS.md"
#define DUCKCLAW_TOOLS_FILE              "/spiffs/config/TOOLS.md"
#define DUCKCLAW_IDENTITY_FILE           "/spiffs/config/IDENTITY.md"
#define DUCKCLAW_CONTEXT_BUF_SIZE        (16 * 1024)
#define DUCKCLAW_SESSION_MAX_MSGS        20
#define DUCKCLAW_MEMORY_LOOKBACK_DAYS    7

/* WebSocket Gateway */
#define DUCKCLAW_WS_PORT                 18789
#define DUCKCLAW_WS_MAX_CLIENTS          4

/* Serial CLI */
#define DUCKCLAW_CLI_STACK               (4 * 1024)
#define DUCKCLAW_CLI_PRIO                3
#define DUCKCLAW_CLI_CORE                0

/* NVS Namespaces */
#define DUCKCLAW_NVS_WIFI                "wifi_config"
#define DUCKCLAW_NVS_DINGTALK            "dingtalk_cfg"
#define DUCKCLAW_NVS_LLM                 "llm_config"
#define DUCKCLAW_NVS_PROXY               "proxy_config"
#define DUCKCLAW_NVS_SEARCH              "search_config"

/* NVS Keys */
#define DUCKCLAW_NVS_KEY_SSID            "ssid"
#define DUCKCLAW_NVS_KEY_PASS            "password"
#define DUCKCLAW_NVS_KEY_DINGTALK_KEY    "app_key"
#define DUCKCLAW_NVS_KEY_DINGTALK_SECRET "app_secret"
#define DUCKCLAW_NVS_KEY_API_KEY         "api_key"
#define DUCKCLAW_NVS_KEY_MODEL           "model"
#define DUCKCLAW_NVS_KEY_PROVIDER        "provider"
#define DUCKCLAW_NVS_KEY_BASE_URL        "base_url"
#define DUCKCLAW_NVS_KEY_PROXY_HOST      "host"
#define DUCKCLAW_NVS_KEY_PROXY_PORT      "port"
#define DUCKCLAW_NVS_KEY_SEARCH_KEY      "search_key"
#define DUCKCLAW_NVS_KEY_WHISPER_BASE_URL "whisper_url"
#define DUCKCLAW_NVS_KEY_BAILIAN_APP_ID   "bailian_app_id"
