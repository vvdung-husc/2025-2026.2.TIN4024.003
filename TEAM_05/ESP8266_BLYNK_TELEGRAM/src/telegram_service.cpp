#include "telegram_service.h"
#include <WiFiClientSecureBearSSL.h>
#include <UniversalTelegramBot.h>
#include <EEPROM.h>
#include "secrets.h"

// Access to sensor data from main.cpp
extern float currentTemp;
extern float currentHum;

static BearSSL::WiFiClientSecure client;
static UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);
static unsigned long lastTelegramCheck = 0;
static const unsigned long POLL_INTERVAL = 3000;

// EEPROM storage for chat id
#define EEPROM_SIZE 512
#define CHAT_MAGIC 0xAA
#define CHAT_ADDR 0

static TelegramLedCallback ledCallback = nullptr;
static String chatIdStored = "";

static String loadChatIdFromEEPROM() {
    if (EEPROM.read(CHAT_ADDR) != CHAT_MAGIC) return "";
    int addr = CHAT_ADDR + 1;
    uint8_t len = EEPROM.read(addr++);
    if (len == 0 || len > 200) return "";
    char buf[201];
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = (char)EEPROM.read(addr++);
    }
    buf[len] = '\0';
    return String(buf);
}

static void saveChatIdToEEPROM(const String &chat_id) {
    int addr = CHAT_ADDR;
    EEPROM.write(addr++, CHAT_MAGIC);
    uint8_t len = min((int)chat_id.length(), 200);
    EEPROM.write(addr++, len);
    for (uint8_t i = 0; i < len; i++) {
        EEPROM.write(addr++, chat_id[i]);
    }
    EEPROM.commit();
}

void telegramInit() {
    client.setInsecure();
    EEPROM.begin(EEPROM_SIZE);
    chatIdStored = loadChatIdFromEEPROM();
    if (chatIdStored.length() > 0) {
        Serial.print("Telegram chat_id loaded: "); Serial.println(chatIdStored);
    }
}

void telegramRegisterLedCallback(TelegramLedCallback cb) {
    ledCallback = cb;
}

void telegramSend(const String &msg) {
    if (chatIdStored.length() == 0) return;
    bot.sendMessage(chatIdStored, msg, "");
}

void telegramForgetChatId() {
    EEPROM.write(CHAT_ADDR, 0x00);
    EEPROM.commit();
    chatIdStored = "";
}

static void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String fromChatId = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        if (chatIdStored.length() == 0) {
            chatIdStored = fromChatId;
            saveChatIdToEEPROM(chatIdStored);
            bot.sendMessage(chatIdStored, "Chat registered for notifications", "");
        }

        if (text == "/ledon") {
            if (ledCallback) ledCallback(true);
            bot.sendMessage(fromChatId, "LED turned ON", "");
        } else if (text == "/ledoff") {
            if (ledCallback) ledCallback(false);
            bot.sendMessage(fromChatId, "LED turned OFF", "");
        } else if (text == "/status") {
            bot.sendMessage(fromChatId, "Use device UI to view status", "");
        } else if (text == "/get_weather") {
            char weatherMsg[128];
            snprintf(weatherMsg, sizeof(weatherMsg), 
                    "🌡️ Current Weather:\nTemp: %.1f°C\nHumidity: %.1f%%", 
                    currentTemp, currentHum);
            bot.sendMessage(fromChatId, weatherMsg, "");
        } else if (text == "/forget_chat") {
            telegramForgetChatId();
            bot.sendMessage(fromChatId, "Chat forgotten", "");
        } else {
            bot.sendMessage(fromChatId, "Commands: /ledon /ledoff /status /get_weather /forget_chat", "");
        }
    }
}

void telegramTick() {
    unsigned long now = millis();
    if (now - lastTelegramCheck < POLL_INTERVAL) return;
    lastTelegramCheck = now;
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) {
        handleNewMessages(numNewMessages);
    }
}
