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
 *        Кроме того в примере показана возможность защиты паролем администратора
 *        не только страниц WiFi-конфигурации, но и пользовательской страницы,
 *        зарегистрированной с помощью server.on(). Адрес тестовой страницы:
 *        http://your_ip/test , где your_ip - IP-адрес модуля.
 *        Или воспользуйтесь ссылкой "Password-protected page" на
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

// закомментируйте эту строку, если хотите сохранять настройки в файловой системе модуля
// иначе они будут сохраняться в EEPROM
#define SAVE_CONFIG_TO_EEPROM

String ssid = "**********"; // имя (SSID) вашей Wi-Fi сети
String pass = "**********"; // пароль для подключения к вашей Wi-Fi сети

String adm_name = "admin";    // пароль администратора для входа на страницу WiFi-конфигурации
String adm_pass = "12345678"; // пароль администратора для входа на страницу WiFi-конфигурации

String config_page = "/wifi_config"; // адрес страницы WiFi-конфигурации

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

  // ==== установим логин и пароль администратора ====
  wifi_config.setAdminData(adm_name, adm_pass);

  // ==== отключаем спящий режим WiFi ================
  // wifi_config.setNoWiFiSleepMode(); // раскомментируйте строку, если вам не нужен спящий режим WiFi

  // ==== включаем возможность использования комбинированного режима
  // wifi_config.setUseComboMode(true); // раскомментируйте строку, если хотите использовать комбинированный режим WiFi (AP + STA)
  
  // ==== задаем использование светодиода ============
  wifi_config.setUseLed(true, LED_BUILTIN);

#if defined(SAVE_CONFIG_TO_EEPROM)

  // инициируем конфигурацию с сохранением в EEPROM
  wifi_config.eepromInit();
  wifi_config.begin(&HTTP, config_page);

#else

  // инициируем конфигурацию с сохранением в файловой системе
  wifi_config.begin(&HTTP, &FILESYSTEM, config_page);
  // ==== инициализируем файловую систему ============
  if (FILESYSTEM.begin())

#endif
  {
    // ==== включаем шифрование паролей ==============
    wifi_config.setCryptState(true);

    // == восстанавливаем сохраненные настройки WiFi =
    wifi_config.loadConfig();
  }

  // ==== устанавливаем соединение с WiFi ============
  if (!wifi_config.startWiFi())
  {
    ESP.restart();
  }

  // ==== настраиваем и запускаем HTTP-сервер ========

  // регистрируем стартовую страницу модуля со ссылками на страницу конфигурации 
  // и тестовую страницу, защищенную паролем
  HTTP.on("/", HTTP_GET, []()
          { HTTP.send(200, "text/html", "<p align='center'  style='font-size: large;'><a href='/wifi_config'>WiFi configuration page</a></p><p align='center'  style='font-size: large;margin-top: 50px;'><a href='/test'>Password-protected page</a></p>"); });
  // тестовая страница для демонстрации парольной защиты
  HTTP.on("/test", HTTP_GET, []()
          { if (!wifi_config.isAuthenticated()) return;
            HTTP.send(200, "text/html", "<p align='center' style='font-size: large;'>Test page</p>"); });
  // стандартная реакция на запрос несуществующей страницы
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
