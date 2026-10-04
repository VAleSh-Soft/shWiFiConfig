#pragma once

// страница, отдаваемая браузеру вместе с требованием аутентификации (401);
// обычно браузер показывает вместо неё своё окно ввода, но если пользователь
// отменил окно или браузер не смог его показать, он получит этот текст вместо
// пустой страницы
static const char AUTH_FAIL_MSG[] PROGMEM =
    "<!DOCTYPE html><html><head><meta charset='utf-8'> <title>Access denied</title></head> <body style='font-family:sans-serif;text-align:center;padding-top:40px'> <h2>Access denied</h2> <p>Wrong user name or password, or the login form was cancelled.</p> <p><a href=''>Try again</a></p></body></html>";

// сообщение о перезагрузке модуля
static const char SUCCESS_RESPONSE_0[] PROGMEM =
    "<META http-equiv='refresh' content='5;URL=/'><p align='center'>The module will be reconnected, wait...</p>";

// сообщение о сохранении настроек
static const char SUCCESS_RESPONSE_1[] PROGMEM =
    "<META http-equiv='refresh' content='5;URL=/'><p align='center'>Save settings...</p>";

// сообщение о неудачном сохранении настроек
static const char FAIL_RESPONSE[] PROGMEM = 
"<META http-equiv='refresh' content='5;URL=/'><p align='center'>The settings could not be saved. Try again...</p>";