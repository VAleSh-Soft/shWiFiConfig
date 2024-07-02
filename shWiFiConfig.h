#pragma once

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
// #include <user_interface.h>
#else
#error "The library is designed for use in the ESP8266 or ESP32 environment."
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <Ticker.h>

// ==== shWiFiConfig class ===========================
class shWiFiConfig
{
private:
  uint32_t checkTimer = 20000;

public:
  /**
   * @brief конструктор
   *
   */
  shWiFiConfig();

  /**
   * @brief конструктор
   *
   * @param adm_name логин для входа в Web-интерфейс
   * @param adm_pass пароль для входа в Web-интерфейс
   */
  shWiFiConfig(String adm_name, String adm_pass);

  /**
   * @brief установка интервала проверки состояния соединения; значение по умолчанию - 30 сек (30000 мсек)
   *
   * @param _timer новое значение интервала в милисекундах
   */
  void setCheckTimer(uint32_t _timer);

  /**
   * @brief включение/отключение выдачи сообщений о работе модуля через Serial
   *
   * @param log_on true - включено; false - выключено
   * @param _serial интерфейс для вывода сообщений
   */
#if ARDUINO_USB_CDC_ON_BOOT // Serial используется для USB CDC
  void setLogOnState(bool log_on, HWCDC *_serial = &Serial);
#else
  void setLogOnState(bool log_on, HardwareSerial *_serial = &Serial);
#endif

  /**
   * @brief установить имя точки доступа, создаваемой модулем
   *
   * @param ap_ssid новое имя (SSID)
   */
  void setApSsid(String ap_ssid);

  /**
   * @brief установить пароль для подключения к точке доступа, создаваемой модулем
   *
   * @param ap_pass новый пароль
   */
  void setApPass(String ap_pass);

  /**
   * @brief установка IP-адреса точки доступа, создаваемой модулем
   *
   * @param ap_ip новый IP-адрес
   */
  void setApIP(IPAddress ap_ip);

  /**
   * @brief установка адреса шлюза точки доступа, создаваемой модулем
   *
   * @param ap_gateway IP-адрес шлюза
   */
  void setApGateway(IPAddress ap_gateway);

  /**
   * @brief установка маски сети точки доступа, создаваемой модулем
   *
   * @param ap_mask маска сети
   */
  void setApMask(IPAddress ap_mask);

  /**
   * @brief установки имени WiFi-сети (SSID) для подключения
   *
   * @param sta_ssid SSID
   */
  void setStaSsid(String sta_ssid);

  /**
   * @brief установка пароля для подключения к заданной WiFi-сети
   *
   * @param sta_pass пароль
   */
  void setStaPass(String sta_pass);

  /**
   * @brief установить статический IP, с которым модуль будет подключаться к заданной WiFi-сети
   *
   * @param sta_ip IP
   */
  void setStaIP(IPAddress sta_ip);

  /**
   * @brief установить адрес шлюза, с которым модуль будет подключаться к заданной WiFi-сети
   *
   * @param sta_gateway адрес шлюза
   */
  void setStaGateway(IPAddress sta_gateway);

  /**
   * @brief установить маску сети, с которой модуль будет подключаться к заданной WiFi-сети
   *
   * @param sta_mask маска сети
   */
  void setStaMask(IPAddress sta_mask);

  /**
   * @brief включить/отключить ручную установку IP-адреса, с которой модуль будет подключаться к заданной WiFi-сети
   *
   * @param static_ip true - включить, false- отключить
   */
  void setStaticIpMode(bool static_ip);

  /**
   * @brief установить имя файла для сохранения настроек
   *
   * @param file_name
   */
  void setConfigFileName(String file_name);

  /**
   * @brief включить/отключить комбинированный режим (AP + STA)
   *
   * @param mode_on true - включить, false- отключить
   */
  void setApStaMode(bool mode_on);

  /**
   * @brief включить возможность использования комбинированного режима (AP + STA); при ключении этой опции в Web-интерфейсе появляется дополнительная вкладка, в которой можно управлять режимом
   *
   * @param mode_on true - включить, false- отключить
   */
  void setUseComboMode(bool mode_on);

  /**
   * @brief использовать или нет парольный доступ к Web-интерфейсу
   *
   * @param pass_on true - включить, false- отключить
   */
  void setUseAdminPass(bool pass_on);

  /**
   * @brief включить возможность использования светодиода для индикации режима работы модуля; при включении опции во вкладке "Разное" Web-интерфейса появляется соответствующий чек-бокс
   *
   * @param _use true - включить, false- отключить
   * @param _pin номер пина, к которому подключается светодиод; светодиод управляется низким уровнем, т.е. подключается к пину катодом
   */
  void setUseLed(bool _use, int8_t _pin = -1);

  /**
   * @brief включить/отключить индикаторный светодиод
   *
   * @param mode_on true - включить, false- отключить
   */
  void setLedOnMode(bool mode_on);

  /**
   * @brief установить данные WiFi-сети, к которой будет подключаться модуль
   *
   * @param ssid имя сети (SSID)
   * @param pass пароль для подключения к сети
   */
  void setStaSsidData(String ssid, String pass);

  /**
   * @brief установить данные точки доступа, которую будет создавать модуль
   *
   * @param ssid имя точки доступа (SSID)
   * @param pass пароль для подключения к точке доступа
   */
  void setApSsidData(String ssid, String pass);

  /**
   * @brief установить логин и пароль для доступа к Web-тнтерфейсу модуля
   *
   * @param name логин
   * @param pass пароль
   */
  void setAdminData(String name, String pass);

  /**
   * @brief получение текущего интервала проверки состояния соединения
   *
   * @return uint32_t
   */
  uint32_t getCheckTimer();

  /**
   * @brief включена или нет выдача сообщений о работе модуля через Serial
   *
   * @return true
   * @return false
   */
  bool getLogOnState();

  /**
   * @brief получение текущего режима работы WiFi-модуля
   *
   * @return WiFiMode_t
   */
  WiFiMode_t getCurMode();

  /**
   * @brief получение текущего имени точки доступа, создаваемой модулем
   *
   * @return String
   */
  String getApSsid();

  /**
   * @brief получение текущего пароль для подключения к точке доступа, создаваемой модулем
   *
   * @return String
   */
  String getApPass();

  /**
   * @brief подключение заданного IP-адреса точки доступа, создаваемой модулем
   *
   * @return IPAddress
   */
  IPAddress getApIP();

  /**
   * @brief получение адреса шлюза точки доступа, создаваемой модулем
   *
   * @return IPAddress
   */
  IPAddress getApGateway();

  /**
   * @brief получение маски сети точки доступа, создаваемой модулем
   *
   * @return IPAddress
   */
  IPAddress getApMask();

  /**
   * @brief получение имени сохраненной WiFi-сети (SSID)
   *
   * @return String
   */
  String getStaSsid();

  /**
   * @brief получение пароля для подлкючения к сохраенной WiFi-сети
   *
   * @return String
   */
  String getStaPass();

  /**
   * @brief получение статического IP-адреса, заданного для модуля
   *
   * @return IPAddress
   */
  IPAddress getStaIP();

  /**
   * @brief получение адреса шлюза, заданного для модуля
   *
   * @return IPAddress
   */
  IPAddress getStaGateway();

  /**
   * @brief получение маски сети, заданной для модуля
   *
   * @return IPAddress
   */
  IPAddress getStaMask();

  /**
   * @brief используется или нет статический адрес при подключении модуля к WiF-сети
   *
   * @return true
   * @return false
   */
  bool getStaticIpMode();

  /**
   * @brief получение текущего имени файла для сохранения настроек модуля
   *
   * @return String
   */
  String getConfigFileName();

  /**
   * @brief включен или нет комбинированный режим работы модуля (AP + STA)
   *
   * @return true
   * @return false
   */
  bool getApStaMode();

  /**
   * @brief включена или нет возможность использования комбинированного режима работы модуля (AP + STA)
   *
   * @return true
   * @return false
   */
  bool getUseComboMode();

  /**
   * @brief включен или нет парольный доступ к Web-интерфейсу модуля
   *
   * @return true
   * @return false
   */
  bool getUseAdminPass();

  /**
   * @brief получить текущий пароль администратора для доступа к Web-интерфейсу модуля
   *
   * @return String
   */
  String getAdminPass();

  /**
   * @brief получить текущий логин для доступа к Web-интерфейсу модуля
   *
   * @return String
   */
  String getAdminName();

  /**
   * @brief разрешено или нет использование индикаторного светодиода
   *
   * @return true
   * @return false
   */
  bool getUseLed();

  /**
   * @brief включена или нет светодиодная индикация режимов работы модуля
   *
   * @return true
   * @return false
   */
  bool getLedOnMode();

  /**
   * @brief получение номера пина для подключения светодиода
   *
   * @return int8_t
   */
  int8_t getLedPin();

  /**
   * @brief установить параметры точки доступа, создаваемой модулем; будут установлены IP и адрес шлюза 192.168.4.1, маска сети 255.255.255.0
   *
   */
  void setApConfig();

  /**
   * @brief установить параметры точки доступа, создаваемой модулем
   *
   * @param ip IP-адрес точки доступа; адрес шлюза будет установлен такой же, маска сети 255.255.255.0
   */
  void setApConfig(IPAddress ip);

  /**
   * @brief установить параметры точки доступа, создаваемой модулем
   *
   * @param ip IP-адрес точки доступа
   * @param gateway адрес шлюза
   * @param mask маска сети
   */
  void setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask);

  /**
   * @brief установить параметры сети, будут установлены параметры, сохраненные в настройках
   *
   */
  void setStaConfig();

  /**
   * @brief установить параметры сети
   *
   * @param ip IP-адрес модуля
   * @param gateway адрес шлюза
   * @param mask маска сети
   */
  void setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask);

#if defined(ARDUINO_ARCH_ESP32)
  /**
   * @brief инициализация модуля
   *
   * @param _server ссылка на экземпляр Web-сервера (WebServer), с которым будет работать конфиг
   * @param _file_system ссылка на экземпляр файловой системы модуля для сохранения файла с настройками
   * @param _config_page адрес страницы Web-интерфейса модуля
   */
  void begin(WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#else
  /**
   * @brief инициализация модуля
   *
   * @param _server ссылка на экземпляр Web-сервера (ESP8266WebServer), с которым будет работать конфиг
   * @param _file_system ссылка на экземпляр файловой системы модуля для сохранения файла с настройками
   * @param _config_page адрес страницы Web-интерфейса модуля
   */
  void begin(ESP8266WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#endif

  /**
   * @brief обработка событий модуля
   *
   */
  void tick();

  /**
   * @brief загрузка настроек, сохраненных в файле
   *
   * @return true
   * @return false
   */
  bool loadConfig();

  /**
   * @brief сохранение настроек модуля в файл
   *
   * @return true
   * @return false
   */
  bool saveConfig();

  /**
   * @brief подключение модуля к WiFi-сети
   *
   * @return true
   * @return false
   */
  bool startWiFi();

  /**
   * @brief отключение модуля от WiFI-сети
   *
   */
  void stopWiFi();

  /**
   * @brief запуск подключения в режиме точки доступа с именем точки доступа и паролем, сохраненными в настройках
   *
   * @return true
   * @return false
   */
  bool startSoftAP();

  /**
   * @brief запуск подключения в режиме точки доступа
   *
   * @param ssid имя точки доступа
   * @param pass пароль для подключения к точке доступа
   * @return true
   * @return false
   */
  bool startSoftAP(String ssid, String pass);

  /**
   * @brief подключение к WiFi-сети, сохраненной в настройках
   *
   * @return true
   * @return false
   */
  bool startSTA();

  /**
   * @brief подключение к WiFi-сети
   *
   * @param ssid имя сети (SSID)
   * @param pass пароль для подключения к сети
   * @return true
   * @return false
   */
  bool startSTA(String ssid, String pass);

  /**
   * @brief поиск WiF-сети сохраненной в настройках
   *
   * @return true
   * @return false
   */
  bool findSavedAp();

  /**
   * @brief поиск WiFi-сети
   *
   * @param ssid имя сети (SSID)
   * @return true
   * @return false
   */
  bool findAp(String ssid);

  /**
   * @brief проверка состояния соединения
   *
   */
  void checkStaConnection();
};

// ==== LedState class ===============================

class LedState
{
private:
  Ticker blink;
  int16_t pwr_value = 280;
  bool toUp = false;
  int8_t pin = -1;;
  bool use_led = true;
#if defined(ARDUINO_ARCH_ESP32)
  int16_t max_pwm = 1023;
  int16_t min_pwm = 0;
#else
  int16_t max_pwm = 280;
  int16_t min_pwm = 0;
#endif
public:
  LedState();
  void setPin(int8_t _pin);
  void init(uint32_t interval, bool force = false);
  void init();
  void stopLed();
  void startLed();
  void digitalCheck();
  void analogCheck();
  void setUseLed(bool _use);
};
