#pragma once

#include "_eeprom.h"

#define WFC_PRINT(x)        \
  if (logOnState && serial) \
  serial->print(x)
#define WFC_PRINTLN(x)      \
  if (logOnState && serial) \
  serial->println(x)

#define EEPROM_INDEX_FOR_WRITE 0

static LedState led;

static shWebServer *http_server = NULL;

static FS *file_system = NULL;

static const int CONFIG_SIZE = 1024;

static bool use_eeprom = false;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char TEXT_HTML[] PROGMEM = "text/html";
static const char TEXT_JSON[] PROGMEM = "text/json";

// ==== настройки WiFi ===============================

// ==== AP ======================================
static String apSsid = "WIFI_AP_";
static String apPass = "12345678";
static IPAddress apIP(192, 168, 4, 1);
static IPAddress apGateway(192, 168, 4, 1);
static IPAddress apMask(255, 255, 255, 0);
// ==== STA =====================================
static String staSsid = "";
static String staPass = "";
static bool staticIP = false;
static IPAddress staIP(192, 168, 0, 50);
static IPAddress staGateway(192, 168, 0, 1);
static IPAddress staMask(255, 255, 255, 0);
// ==== AP + STA ================================
static bool ap_sta_mode = false;
static bool useComboMode = false;
// ==== Other ===================================
static bool useAdmPass = false;
static String admName = "";
static String admPass = "";
static bool useLed = true;
static bool ledOn = true;
static int8_t ledPin = -1;

static bool badPassword = false;
static WiFiMode_t curMode = WIFI_OFF;
static String fileName = "/wifi.json";

static Print *serial = NULL;
static bool logOnState = true;

// ==== Имена JSON-параметров ========================
static const String ap_ssid_str = "ap_ssid";
static const String ap_pass_str = "ap_pass";
static const String ap_ip_str = "ap_ip";
static const String ap_gateway_str = "ap_gateway";
static const String ap_mask_str = "ap_mask";
static const String ssid_str = "ssid";
static const String pass_str = "pass";
static const String static_ip_str = "static_ip";
static const String ip_str = "ip";
static const String gateway_str = "gateway";
static const String mask_str = "mask";
static const String use_combo_mode_str = "use_combo";
static const String ap_sta_mode_str = "ap_sta";
static const String use_adm_pass_str = "use_adm_pass";
static const String a_name_str = "a_name";
static const String a_pass_str = "a_pass";
static const String use_led_str = "use_led";
static const String led_on_off = "led_on";

// ===================================================

static void handleGetConfigPage();
static void handleReadSetting();
static void handleWriteSetting();
static void handleGetApList();

static bool save_config();
static bool load_config();
static void readJsonSetting(StaticJsonDocument<CONFIG_SIZE> &doc);
static void writeSettingInJson(StaticJsonDocument<CONFIG_SIZE> &doc);

static bool find_ap(String ssid);
static void set_cur_mode(WiFiMode_t _mode);
static void set_sta_config(const IPAddress &ip,
                           const IPAddress &gateway,
                           const IPAddress &mask);
static void set_ap_config(const IPAddress &ip,
                          const IPAddress &gateway,
                          const IPAddress &mask);
static bool start_wifi();
static void stop_wifi();
static bool start_sta(String &ssid, String &pass, bool search_ssid = true);
static bool start_ap(String &ssid, String &pass, bool combo_mode = false);

// ===================================================

static void set_config()
{
  WiFi.mode(curMode);

  if (&Serial != NULL)
  {
    serial = &Serial;
  }
}

static void _begin(shWebServer *_server, const String &_config_page = "/wifi_config")
{
  WiFi.mode(curMode);

  http_server = _server;

  // вызов страницы настройки WiFi
  http_server->on(_config_page, HTTP_GET, &handleGetConfigPage);
  // заполнение полей страницы настройки WiFi
  http_server->on("/wifi_getconfig", HTTP_GET, handleReadSetting);
  // сохранение настроек
  http_server->on("/wifi_setconfig", HTTP_POST, handleWriteSetting);
  // получение списка доступных точек доступа
  http_server->on("/wifi_getaplist", HTTP_GET, handleGetApList);
}

// ==== реакции сервера ==============================

static void handleGetConfigPage()
{
  if (useAdmPass &&
      admName != emptyString &&
      admPass != emptyString &&
      !http_server->authenticate(admName.c_str(), admPass.c_str()))
  {
    return http_server->requestAuthentication();
  }
  http_server->send(200, FPSTR(TEXT_HTML), FPSTR(config_page));
}

static void handleReadSetting()
{
  StaticJsonDocument<CONFIG_SIZE> doc;

  writeSettingInJson(doc);
  doc[use_combo_mode_str] = (byte)useComboMode;
  doc[use_led_str] = (byte)useLed;

  String json = "";
  serializeJson(doc, json);

  http_server->send(200, FPSTR(TEXT_JSON), json);
}

static void handleWriteSetting()
{
  if (http_server->hasArg("plain") == false)
  {
    http_server->send(200, FPSTR(TEXT_PLAIN), F("Body not received"));
    WFC_PRINTLN(F("Failed to save configuration data, no data"));
    return;
  }

  String json = http_server->arg("plain");

  StaticJsonDocument<CONFIG_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    WFC_PRINTLN(F("Failed to save configuration data, invalid json data"));
    WFC_PRINTLN(error.f_str());
  }
  else
  {
    badPassword = false;

    // определить необходимость перезагрузки модуля
    bool reconnect = (staSsid != doc[ssid_str].as<String>()) ||
                     (staPass != doc[pass_str].as<String>());
    if (!reconnect)
    {
      reconnect = staticIP != (bool)doc[static_ip_str].as<byte>();
      if (!reconnect && staticIP)
      {
        IPAddress ip;
        reconnect = (ip.fromString(doc[ip_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)staIP)) ||
                    (ip.fromString(doc[gateway_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)staGateway)) ||
                    (ip.fromString(doc[mask_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)staMask));
      }
    }
    if (!reconnect && (curMode == WIFI_AP || curMode == WIFI_AP_STA))
    {
      reconnect = (apSsid != doc[ap_ssid_str].as<String>()) ||
                  (apPass != doc[ap_pass_str].as<String>());
      if (!reconnect)
      {
        IPAddress ip;
        reconnect = (ip.fromString(doc[ap_ip_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)apIP)) ||
                    (ip.fromString(doc[ap_mask_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)apMask));
      }
    }
    if (!reconnect)
    {
      reconnect = ap_sta_mode != (bool)doc[ap_sta_mode_str].as<byte>();
    }

    readJsonSetting(doc);
    save_config();
    const String successResponse0 =
        F("<META http-equiv=\"refresh\" content=\"5;URL=/\"><p align=\"center\">The module will be reconnected, wait...</p>");
    const String successResponse1 =
        F("<META http-equiv=\"refresh\" content=\"1;URL=/\"><p align=\"center\">Save settings...</p>");
    http_server->client().setNoDelay(true);
    // Если изменили опции, требующие переподключения, переподключить модуль
    if (reconnect)
    {
      WFC_PRINTLN("");
      WFC_PRINTLN(F("The module will be reconnected, wait..."));
      WFC_PRINTLN("");
      http_server->send(200, FPSTR(TEXT_HTML), successResponse0);
      delay(100);
      stop_wifi();
      start_wifi();
    }
    else
    {
      http_server->send(200, FPSTR(TEXT_HTML), successResponse1);
    }
    led.startLed();
  }
}

static void handleGetApList()
{
  int n = WiFi.scanNetworks();

  StaticJsonDocument<2048> doc;
  if (n > 0)
  {
    for (byte i = 0; i < n; ++i)
    {
      doc["aps"][i] = WiFi.SSID(i);
    }
  }
  else
    doc["list"] = "empty";

  String json = "";
  serializeJson(doc, json);

  http_server->send(200, FPSTR(TEXT_JSON), json);
}

static void handleShowSavePage()
{
  const String successResponse =
      F("<META http-equiv=\"refresh\" content=\"1;URL=/\">Save settings...");
  http_server->client().setNoDelay(true);
  http_server->send(200, FPSTR(TEXT_HTML), successResponse);
}

// ===================================================

static bool save_config_to_eeprom(StaticJsonDocument<CONFIG_SIZE> &doc)
{
  WFC_PRINT(F("Save WiFi settings to EEPROM"));

  bool result = false;
  uint16_t size = measureJson(doc);

  char *str;
  result = serializeJson(doc, str, size + 1);
  write_string_to_eeprom(str, EEPROM_INDEX_FOR_WRITE);
  free(str);

  return (result);
}

static bool save_config_to_file(StaticJsonDocument<CONFIG_SIZE> &doc)
{
  File configFile;

  WFC_PRINT(F("Save WiFi settings to file "));
  WFC_PRINTLN(fileName);

  bool result = false;

  // удалить существующий файл, иначе конфигурация будет добавлена ​​к файлу
  file_system->remove(fileName);

  // Открыть файл для записи
  configFile = file_system->open(fileName, "w");
  if (!configFile)
  {
    WFC_PRINTLN(F("Failed to create WiFi configuration file"));
    return (result);
  }
  // сериализовать JSON-файл
  result = serializeJson(doc, configFile);
  configFile.close();

  return result;
}

static bool save_config()
{
  bool result = false;

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<CONFIG_SIZE> doc;

  // задать данные для сохранения
  writeSettingInJson(doc);

  result = (use_eeprom) ? save_config_to_eeprom(doc)
                        : save_config_to_file(doc);

  if (result)
  {
    WFC_PRINTLN(F("OK"));
  }
  else
  {
    WFC_PRINTLN(F("Failed to write WiFi configuration"));
  }

  return (result);
}

static bool load_from_eeprom(StaticJsonDocument<CONFIG_SIZE> &doc,
                             DeserializationError &error)
{
  WFC_PRINT(F("Load WiFi settings from EEPROM"));

  char *str = read_string_from_eeprom(EEPROM_INDEX_FOR_WRITE);
  error = deserializeJson(doc, str);
  free(str);

  return (true);
}

static bool load_from_file(StaticJsonDocument<CONFIG_SIZE> &doc,
                           DeserializationError &error)
{
  WFC_PRINT(F("Load WiFi settings from file "));
  WFC_PRINTLN(fileName);

  File configFile;
  // находим и открываем для чтения файл конфигурации
  bool result = file_system->exists(fileName) &&
                (configFile = file_system->open(fileName, "r"));

  // если файл конфигурации не найден, сохранить настройки по умолчанию
  if (!result)
  {
    WFC_PRINTLN(F("WiFi config file not found, default config used."));
    save_config();
    return (result);
  }

  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > CONFIG_SIZE)
  {
    WFC_PRINTLN(F("WiFi configuration file size is too large."));
    configFile.close();
    return (false);
  }
  // Deserialize the JSON document
  error = deserializeJson(doc, configFile);
  configFile.close();

  return (true);
}

static bool load_config()
{
  bool result = false;

  StaticJsonDocument<CONFIG_SIZE> doc;
  DeserializationError error;

  if (use_eeprom)
  {
    if (!load_from_eeprom(doc, error))
    {
      return false;
    }
  }
  else
  {
    if (!load_from_file(doc, error))
    {
      return false;
    }
  }

  if (error)
  {
    WFC_PRINT("Data serialization error: ");
    WFC_PRINTLN(error.f_str());
    WFC_PRINTLN(F("Failed to read WiFi config, default config is used"));
    result = false;
  }
  else
  // Теперь можно получить значения из doc
  {
    readJsonSetting(doc);
  }

  if (result)
  {
    WFC_PRINTLN(F("OK"));
  }

  return (result);
}

static void readJsonSetting(StaticJsonDocument<CONFIG_SIZE> &doc)
{
  String _str[] = {ap_ssid_str, ap_pass_str, ssid_str, pass_str, a_name_str, a_pass_str,
                   ap_ip_str, ap_gateway_str, ap_mask_str, ip_str, gateway_str, mask_str};
  String *str_val[] = {&apSsid, &apPass, &staSsid, &staPass, &admName, &admPass};

  IPAddress *ip_val[] = {&apIP, &apGateway, &apMask, &staIP, &staGateway, &staMask};

  staticIP = doc[static_ip_str].as<bool>();
  ap_sta_mode = doc[ap_sta_mode_str].as<bool>();
  useAdmPass = doc[use_adm_pass_str].as<bool>();
  ledOn = doc[led_on_off].as<bool>();
  led.setUseLed(ledOn);

  for (byte i = 0; i < 6; i++)
  {
    String s = doc[_str[i]].as<String>();
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (s != "null")
    {
      *str_val[i] = s;
    }
  }

  for (byte i = 6; i < 12; i++)
  {
    IPAddress ip;
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (ip.fromString(doc[_str[i]].as<String>()))
    {
      *ip_val[i - 6] = ip;
    }
  }
}

static void writeSettingInJson(StaticJsonDocument<CONFIG_SIZE> &doc)
{
  doc[ap_ssid_str] = apSsid;
  doc[ap_pass_str] = apPass;
  doc[ap_ip_str] = apIP.toString();
  doc[ap_gateway_str] = apGateway.toString();
  doc[ap_mask_str] = apMask.toString();
  doc[ssid_str] = staSsid;
  doc[pass_str] = staPass;
  doc[static_ip_str] = (byte)staticIP;
  doc[ip_str] = staIP.toString();
  doc[gateway_str] = staGateway.toString();
  doc[mask_str] = staMask.toString();
  doc[ap_sta_mode_str] = (byte)ap_sta_mode;
  doc[use_adm_pass_str] = (byte)useAdmPass;
  doc[a_name_str] = admName;
  doc[a_pass_str] = admPass;
  doc[led_on_off] = (byte)ledOn;
}

// ===================================================

static bool find_ap(String ssid)
{
  bool result = false;

  if (staSsid != emptyString)
  {
    led.init(100, true);
    WFC_PRINT(F("Searche for access point "));
    WFC_PRINTLN(ssid);
    int8_t n = WiFi.scanNetworks();
    if (n > 0)
    {
      for (byte i = 0; i < n; ++i)
      {
        if (WiFi.SSID(i) == ssid)
        {
          result = true;
          break;
        }
      }
    }

    if (result)
    {
      WFC_PRINTLN(F("OK"));
    }
    else
    {
      WFC_PRINTLN(F("not found"));
    }
  }
  return (result);
}

static void set_cur_mode(WiFiMode_t _mode)
{
  curMode = _mode;
  WiFi.mode(_mode);
}

static void set_sta_config(const IPAddress &ip,
                           const IPAddress &gateway,
                           const IPAddress &mask)
{
  WiFi.config(ip, gateway, mask);
}

static void set_ap_config(const IPAddress &ip,
                          const IPAddress &gateway,
                          const IPAddress &mask)
{
  WiFi.softAPConfig(ip, gateway, mask);
}

static bool start_wifi()
{
  bool result = start_sta(staSsid, staPass);
  if (!result)
  {
    stop_wifi();
    result = start_ap(apSsid, apPass);
  }

  return (result);
}

static void stop_wifi()
{
  if (curMode == WIFI_STA || curMode == WIFI_AP_STA)
  {
    WiFi.disconnect();
  }
  if (curMode == WIFI_AP || curMode == WIFI_AP_STA)
  {
    WiFi.softAPdisconnect();
  }
  set_cur_mode(WIFI_OFF);
}

static bool start_sta(String &ssid, String &pass, bool search_ssid)
{
  bool result = false;

  (useComboMode && ap_sta_mode) ? set_cur_mode(WIFI_AP_STA)
                                : set_cur_mode(WIFI_STA);
  if (curMode > WIFI_STA)
  {
    start_ap(apSsid, apPass, true);
  }

  if (!search_ssid || find_ap(ssid))
  {
    WiFi.hostname(apSsid);
    if (staticIP)
    {
      set_sta_config(staIP, staGateway, staMask);
    }
    else
    {
      WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }
    led.init(100, true);
    WFC_PRINT(F("Connecting to WiFi network "));
    WFC_PRINTLN(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    result = WiFi.waitForConnectResult() == WL_CONNECTED;
    if (result)
    {
      WFC_PRINT(F("Connected: "));
      WFC_PRINTLN(WiFi.SSID());
      WFC_PRINT(F("IP: "));
      WFC_PRINTLN(WiFi.localIP());
#if defined(ARDUINO_ARCH_ESP8266)
      WFC_PRINT(F("HostName: "));
      WFC_PRINTLN(WiFi.hostname());
#endif

      badPassword = false;
      led.init();
    }
    else
    {
      WFC_PRINT(F("Failed to connect to "));
      WFC_PRINTLN(ssid);
#if defined(ARDUINO_ARCH_ESP8266)
      if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
      {
        WFC_PRINTLN(F("Incorrect password!"));
        badPassword = true;
      }
#else
      badPassword = true;
#endif
    }
  }
  WFC_PRINTLN("");

  return (result);
}

static bool start_ap(String &ssid, String &pass, bool combo_mode)
{
  bool result = false;
  WFC_PRINT(F("Create WiFi access point "));
  WFC_PRINTLN(ssid);

  led.init(100, true);
  (combo_mode) ? set_cur_mode(WIFI_AP_STA) : set_cur_mode(WIFI_AP);
  set_ap_config(apIP, apGateway, apMask);
  result = WiFi.softAP(ssid.c_str(), pass.c_str());
  if (result)
  {
    WFC_PRINT(F("Access point SSID: "));
    WFC_PRINTLN(WiFi.softAPSSID());
    WFC_PRINT(F("Access point IP: "));
    WFC_PRINTLN(WiFi.softAPIP());
    led.init(500);
  }
  else
  {
    WFC_PRINTLN(F("Access point failed"));
  }
  WFC_PRINTLN("");

  return (result);
}
