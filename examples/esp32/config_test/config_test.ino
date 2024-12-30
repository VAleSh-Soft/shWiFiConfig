/**
 * @file config_test.ino
 * @author Vladimir Shatalov (valesh-soft@yandex.ru)
 *
 * @brief Пример использования Web-интерфейса для настройки WiFi параметров на
 *        ESP32.
 *
 *        Для доступа к настройкам введите в адресной строке браузера
 *        http://your_ip/wifi_config , где your_ip - IP-адрес модуля.
 *
 *        Сохранение параметров возможно как в файловой системе модуля, так и в
 *        EEPROM. Для использования EEPROM раскомментируйте строку
 *        #define SAVE_CONFIG_TO_EEPROM
 *
 *        При сохранении параметров в файловой системе необходимо выбрать тип
 *        файловой системы. Для этого в дефайне #define FILESYSTEM укажите FFat
 *        или SPIFFS. При этом в настройках платы в Partition Scheme: нужно
 *        выбрать соответственно либо 'Default 4MB with ffat', либо
 *        'Default 4MB with SPIFFS' (если у вас плата с 4MB флеш-памяти), либо
 *        другой соответствующий вариант в зависимости от количества
 *        флеш-памяти вашей платы.
 *
 *        В примере так же показано использование шифрования паролей при
 *        сохранении параметров.
 *
 * @version 1.2
 * @date 23.10.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <WebServer.h>
#include <shWiFiConfig.h>

#define LED_PIN 4

#define SAVE_CONFIG_TO_EEPROM

String ssid = "**********"; // имя (SSID) вашей Wi-Fi сети
String pass = "**********"; // пароль для подключения к вашей Wi-Fi сети

// Web интерфейс для устройства
WebServer HTTP(80);
// конфигурация WiFi
shWiFiConfig wifi_config;

#if !defined(SAVE_CONFIG_TO_EEPROM)

// файловая система
#define FILESYSTEM FFat

#if FILESYSTEM == FFat
#include <FFat.h>
#elif FILESYSTEM == SPIFFS
#include <SPIFFS.h>
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
  wifi_config.begin(&HTTP);
#else
  // инициируем конфигурацию с сохранением в файловой системе
  wifi_config.begin(&HTTP, &FILESYSTEM);
  // ==== инициализируем файловую систему ============
  if (FILESYSTEM.begin(true))
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
  wifi_config.setUseLed(true, LED_PIN);

  // ==== устанавливаем соединение с WiFi ============
  if (!wifi_config.startWiFi())
  {
    ESP.restart();
  }

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
