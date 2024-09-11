#include <WebServer.h>
#include <shWiFiConfig.h>

/*
 Пример использования Web-интерфейса для настройки WiFi параметров;
 Для доступа к настройкам введите в адресной строке браузера

 http://your_ip/wifi_config

 где your_ip - IP-адрес модуля
*/

#define LED_BUILTIN 4

// файловая система
#define FILESYSTEM FFat

#if FILESYSTEM == FFat
#include <FFat.h>
#elif FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

String ssid = "**********"; // имя (SSID) вашей Wi-Fi сети
String pass = "**********"; // пароль для подключения к вашей Wi-Fi сети

// Web интерфейс для устройства
WebServer HTTP(80);
// конфигурация WiFi
shWiFiConfig wifi_config;

void setup()
{ 
  Serial.begin(115200);
  Serial.println();

  wifi_config.setStaSsidData(ssid, pass);
  wifi_config.begin(&HTTP, &FILESYSTEM);

  // ==== инициализируем файловую систему ============
  if (FILESYSTEM.begin(true))
  {

    // ==== восстанавливаем настройки WiFi ========
    wifi_config.loadConfig();
  }

//  wifi_config.setUseComboMode(true); // раскомментируйте строку, если хотите использовать комбинированный режим WiFi (AP + STA)
  wifi_config.setUseLed(true, LED_BUILTIN);
  if (!wifi_config.startWiFi())
  {
    ESP.restart();
  }

  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. File not found.")); });

  Serial.println(F("Starting the web server"));
  HTTP.begin();
}

void loop()
{
  wifi_config.tick();
}
