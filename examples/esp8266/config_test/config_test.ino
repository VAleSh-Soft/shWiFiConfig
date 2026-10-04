/**
 * @file config_test.ino
 * @author Vladimir Shatalov (valesh-soft@yandex.ru)
 *
 * @brief Пример использования Web-интерфейса для настройки WiFi параметров на
 *        ESP8266.
 *
 *        Для доступа к настройкам введите в адресной строке браузера
 *        http://your_ip/wifi_config , где your_ip - IP-адрес модуля.
 *        Или воспользуйтесь ссылкой "WiFi configuration page" на 
 *        главной странице модуля.
 *
 *        Сохранение параметров возможно как в файловой системе модуля, так и в
 *        EEPROM. Для использования EEPROM раскомментируйте строку
 *        #define SAVE_CONFIG_TO_EEPROM
 *
 *        В примере так же показано использование шифрования паролей при
 *        сохранении параметров.
 *
 * @version 1.3
 * @date 04.10.2026
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <ESP8266WebServer.h>
#include <shWiFiConfig.h>

#define SAVE_CONFIG_TO_EEPROM

String ssid = "**********"; // имя (SSID) вашей Wi-Fi сети
String pass = "**********"; // пароль для подключения к вашей Wi-Fi сети

// Web интерфейс для устройства
ESP8266WebServer HTTP(80);
// конфигурация WiFi
shWiFiConfig wifi_config;

#if !defined(SAVE_CONFIG_TO_EEPROM)

// файловая система
#define FILESYSTEM SPIFFS

#if FILESYSTEM == LittleFS
#include <LittleFS.h>
#elif FILESYSTEM == SPIFFS
#include <FS.h>
#endif

#endif

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // ==== установим данные подключения по умолчанию ==
  wifi_config.setStaSsidData(ssid, pass);
  // ==== отключаем спящий режим WiFi ================
  // wifi_config.setNoWiFiSleepMode(); // раскомментируйте строку, если вам не нужен спящий режим WiFi

#if defined(SAVE_CONFIG_TO_EEPROM)

  // инициируем конфигурацию с сохранением в EEPROM
  wifi_config.eepromInit();
  wifi_config.begin(&HTTP, "/wifi_config");

#else

  // инициируем конфигурацию с сохранением в файловой системе
  wifi_config.begin(&HTTP, &FILESYSTEM, "/wifi_config");
  // ==== инициализируем файловую систему ============
  if (FILESYSTEM.begin())

#endif
  {
    // ==== включаем шифрование паролей ==============
    wifi_config.setCryptState(true);
    // == восстанавливаем сохраненные настройки WiFi =
    wifi_config.loadConfig();
  }

  // ==== включаем возможность использования комбинированного режима
  // wifi_config.setUseComboMode(true); // раскомментируйте строку, если хотите использовать комбинированный режим WiFi (AP + STA)
  
  // ==== задаем использование светодиода ============
  wifi_config.setUseLed(true, LED_BUILTIN);

  // ==== устанавливаем соединение с WiFi ============
  if (!wifi_config.startWiFi())
  {
    ESP.restart();
  }

  // ==== настраиваем и запускаем HTTP-сервер ========

  // добавляем стартовую страницу модуля со ссылкой на страницу конфигурации
  HTTP.on("/", HTTP_GET, []()
          { HTTP.send(200, "text/html", "<p align='center'  style='font-size: large;'><a href='/wifi_config'>WiFi configuration page</a></p>"); });
  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. File not found")); });

  Serial.println(F("Starting the web server"));
  Serial.println();
  
  HTTP.begin();
}

void loop()
{
  wifi_config.tick();
}
