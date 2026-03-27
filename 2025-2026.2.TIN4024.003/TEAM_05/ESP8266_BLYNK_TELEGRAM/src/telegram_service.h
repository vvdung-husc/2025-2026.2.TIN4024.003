#ifndef TELEGRAM_SERVICE_H
#define TELEGRAM_SERVICE_H

#include <Arduino.h>

typedef void (*TelegramLedCallback)(bool on);

void telegramInit();
void telegramTick();
void telegramSend(const String &msg);
void telegramRegisterLedCallback(TelegramLedCallback cb);
void telegramForgetChatId();

#endif // TELEGRAM_SERVICE_H
