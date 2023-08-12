#include <ESP8266WiFi.h>
#include <shWiFiConfig.h>

/*
 Пример использования Web-интерфейса для настройки WiFi параметров;
 Для доступа к настройкам введите в адресной строке браузера

 http://your_ip/wifi_config

 где your_ip - IP-адрес модуля
*/

// Web интерфейс для устройства
ESP8266WebServer HTTP(80);
// конфигурация WiFi
shWiFiConfig wifi_config;
// файловая система
FS *fileSystem = &SPIFFS;

void setup()
{ 
  Serial.begin(115200);
  Serial.println();

  wifi_config.begin(&HTTP, fileSystem);

  // ==== инициализируем файловую систему ============
  if (fileSystem->begin())
  {

    // ==== восстанавливаем настройки WiFi ========
    Serial.println(F("Считывание настроек WiFi..."));
    wifi_config.loadConfig();
  }

  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. File not found.")); });

  Serial.println(F("Старт WEB-сервера"));
  HTTP.begin();

//  wifi_config.setUseComboMode(true); // раскомментируйте, если хотите использовать комбинированный режим WiFi (AP + STA)
  wifi_config.setUseLed(true, LED_BUILTIN);
  if (!wifi_config.startWiFi())
  {
    ESP.restart();
  }
}

void loop()
{
  wifi_config.tick();

  HTTP.handleClient();

  delay(1);
}
