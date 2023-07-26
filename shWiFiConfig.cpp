#include "shWiFiConfig.h"
#include "extras/c_page.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
static WebServer *http_server = NULL;
#else
static ESP8266WebServer *http_server = NULL;
#endif

static FS *file_system;

static const int confSize = 1024;

static void handleGetConfigPage();
static void handleReadSetting();
static void handleWriteSetting();
static void handleReadApList();
static bool saveConfig();
static void println(String msg);
static void print(String msg);
static bool readSetting(StaticJsonDocument<confSize> &doc);
static void writeSetting(StaticJsonDocument<confSize> &doc);

// ==== настройки WiFi ===============================
static String apSsid = AP_SSID;
static String apPass = AP_PASS;
static IPAddress apIP = AP_IP;
static IPAddress apMask = AP_MASK;
static String staSsid = STA_SSID;
static String staPass = STA_PASS;
static IPAddress staIP = STA_IP;
static IPAddress staGateway = STA_GATEWAY;
static IPAddress staMask = STA_MASK;
static bool staticIP = false;
static String fileName = "/wifi.json";
static bool badPassword = false;
static bool ap_sta_mode = false;
static bool useComboMode = false;
static WiFiMode_t curMode = WIFI_OFF;
static bool logOnState = true;

// ==== Имена JSON-параметров ========================
static const String ssid_ap_str = "ap_ssid";
static const String ap_pass_str = "ap_pass";
static const String ap_ip_str = "ap_ip";
static const String ap_mask_str = "ap_mask";
static const String ssid_str = "ssid";
static const String pass_str = "pass";
static const String static_ip_str = "static_ip";
static const String ip_str = "ip";
static const String gateway_str = "gateway";
static const String mask_str = "mask";
static const String use_combo_mode_str = "use_combo";
static const String ap_sta_mode_str = "ap_sta";

// ==== shWiFiConfig class ===========================

shWiFiConfig::shWiFiConfig()
{
  WiFi.mode(curMode);
}

void shWiFiConfig::setLogOnState(bool log_on) { logOnState = log_on; }

void shWiFiConfig::setCurMode(WiFiMode _mode)
{
  curMode = _mode;
  WiFi.mode(_mode);
}

void shWiFiConfig::setApSsid(String ap_ssid) { apSsid = ap_ssid; }

void shWiFiConfig::setApPass(String ap_pass) { apPass = ap_pass; }

void shWiFiConfig::setApIP(IPAddress ap_ip) { apIP = ap_ip; }

void shWiFiConfig::setApMask(IPAddress ap_mask) { apMask = ap_mask; }

void shWiFiConfig::setStaSsid(String sta_ssid) { staSsid = sta_ssid; }

void shWiFiConfig::setStaPass(String sta_pass) { staPass = sta_pass; }

void shWiFiConfig::setStaIP(IPAddress sta_ip) { staIP = sta_ip; }

void shWiFiConfig::setStaGateway(IPAddress sta_gateway) { staGateway = sta_gateway; }

void shWiFiConfig::setStaMask(IPAddress sta_mask) { staMask = sta_mask; }

void shWiFiConfig::setStaticIpMode(bool static_ip) { staticIP = static_ip; }

void shWiFiConfig::setConfigFileName(String file_name) { fileName = file_name; }

void shWiFiConfig::setApStaMode(bool mode_on) { ap_sta_mode = mode_on; }

void shWiFiConfig::setUseComboMode(bool mode_on) { useComboMode = mode_on; }

bool shWiFiConfig::getLogOnState() { return (logOnState); }

WiFiMode_t shWiFiConfig::getCurMode() { return (curMode); }

String shWiFiConfig::getApSsid() { return (apSsid); }

String shWiFiConfig::getApPass() { return (apPass); }

IPAddress shWiFiConfig::getApIP() { return (apIP); }

IPAddress shWiFiConfig::getApMask() { return (apMask); }

String shWiFiConfig::getStaSsid() { return (staSsid); }

String shWiFiConfig::getStaPass() { return (staPass); }

IPAddress shWiFiConfig::getStaIP() { return (staIP); }

IPAddress shWiFiConfig::getStaGateway() { return (staGateway); }

IPAddress shWiFiConfig::getStaMask() { return (staMask); }

bool shWiFiConfig::getStaticIpMode() { return (staticIP); }

String shWiFiConfig::getConfigFileName() { return (fileName); }

bool shWiFiConfig::getApStaMode() { return (ap_sta_mode); }

bool shWiFiConfig::getUseComboMode() { return (useComboMode); }

void shWiFiConfig::setApConfig()
{
  setApConfig(apIP, apIP, (IPAddress(255, 255, 255, 0)));
}

void shWiFiConfig::setApConfig(IPAddress ip)
{
  setApConfig(ip, ip, (IPAddress(255, 255, 255, 0)));
}

void shWiFiConfig::setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.softAPConfig(ip, gateway, mask);
}

void shWiFiConfig::setStaConfig()
{
  setStaConfig(staIP, staGateway, staMask);
}

void shWiFiConfig::setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.config(ip, gateway, mask);
}

#if defined(ARDUINO_ARCH_ESP32)
bool shWiFiConfig::begin(WebServer *_server, FS *_file_system, String _config_page)
#else
bool shWiFiConfig::begin(ESP8266WebServer *_server, FS *_file_system, String _config_page)
#endif
{
  http_server = _server;
  file_system = _file_system;
  _fsOK = file_system->begin();

  // вызов страницы настройки WiFi
  http_server->on(_config_page, HTTP_GET, &handleGetConfigPage);
  http_server->on("/getconfig", HTTP_GET, handleReadSetting);
  http_server->on("/setconfig", HTTP_POST, handleWriteSetting);
  http_server->on("/getaplist", HTTP_GET, handleReadApList);

  return (_fsOK);
}

bool shWiFiConfig::loadConfig()
{
  File configFile;
  // находим и открываем для чтения файл конфигурации
  bool result = file_system->exists(fileName) &&
                (configFile = file_system->open(fileName, "r"));

  // если файл конфигурации не найден, сохранить настройки по умолчанию
  if (!result)
  {
    println(F("WiFi config file not found, default config used."));
    saveConfig();
    return (result);
  }

  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024)
  {
    println(F("WiFi configuration file size is too large."));
    configFile.close();
    return (false);
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<confSize> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    print("Data serialization error: ");
    println(error.f_str());
    println(F("Failed to read WiFi config file, default config is used"));
    result = false;
  }
  else
  // Теперь можно получить значения из doc
  {
    readSetting(doc);
  }

  return (result);
}

bool shWiFiConfig::startSoftAP()
{
  bool result = false;
  println(F("Create WiFi access point ") + apSsid);

  setCurMode(WIFI_AP);
  setApConfig();
  result = WiFi.softAP(apSsid, apPass);
  if (result)
  {
    println(F("Access point SSID: ") + WiFi.softAPSSID());
    println(F("Access point IP: ") + WiFi.softAPIP().toString() + "\n");
  }
  else
  {
    println(F("Access point failed"));
  }

  return (result);
}

bool shWiFiConfig::startSTA()
{
  return (startSTA(staSsid, staPass));
}

bool shWiFiConfig::startSTA(String ssid, String pass)
{
  bool result = false;
  println(F("Connecting to WiFi network ") + ssid);
  (ap_sta_mode) ? setCurMode(WIFI_AP_STA) : setCurMode(WIFI_STA);
  WiFi.hostname(apSsid);
  if (staticIP)
  {
    setStaConfig();
  }
  WiFi.begin(ssid, pass);
  result = WiFi.waitForConnectResult() == WL_CONNECTED;
  if (result)
  {
    println(F("Connection: ") + WiFi.SSID());
    println(F("IP: ") + WiFi.localIP().toString());
    println(F("HostName: ") + WiFi.hostname() + "\n");

    badPassword = false;
  }
  else
  {
    println(F("Failed to connect to ") + ssid);
    if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
    {
      println(F("Incorrect password!"));
      badPassword = true;
    }
    println("");
  }

  return (result);
}

bool shWiFiConfig::findSavedAp()
{
  return (findAp(staSsid));
}

bool shWiFiConfig::findAp(String ssid)
{
  bool result = false;

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
  return (result);
}

void shWiFiConfig::checkStaConnection()
{
  if (curMode == WIFI_STA || curMode == WIFI_AP_STA)
  {
    // если мы в режиме точки доступа
    if (!WiFi.isConnected())
    {
      println(F("WiFi connection lost"));
      // если пропала связь с роутером, перейти в режим точки доступа
      WiFi.disconnect();
      if (curMode == WIFI_AP_STA)
      {
        WiFi.softAPdisconnect();
      }
      if (!startSoftAP())
      {
        ESP.restart();
      }
    }
  }
  else
  {
    // пытаться искать и подключаться к сети только если пароль не помечен неверным
    if (!badPassword && findSavedAp())
    {
      // если наша сеть найдена, подключиться к ней
      if (curMode == WIFI_AP)
      {
        WiFi.softAPdisconnect();
      }
      // если не удалось, перейти в режим точки доступа
      if (!startSTA())
      {
        startSoftAP();
      }
    }
  }
}

// ==== реакции сервера ==============================

void handleGetConfigPage()
{
  http_server->send(200, "text/html", FPSTR(config_page));
}

void handleReadSetting()
{
  StaticJsonDocument<confSize> doc;

  writeSetting(doc);
  doc[use_combo_mode_str] = (byte)useComboMode;

  String json = "";
  serializeJson(doc, json);

  http_server->send(200, "text/json", json);
}

void handleWriteSetting()
{
  if (http_server->hasArg("plain") == false)
  {
    http_server->send(200, "text/plain", F("Body not received"));
    println(F("Failed to save configuration data, no data"));
    return;
  }

  String json = http_server->arg("plain");

  http_server->send(200, "text/plain");
  Serial.println(json);

  StaticJsonDocument<confSize> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    println(F("Failed to save configuration data, invalid json data"));
    println(error.f_str());
  }
  else
  {
    badPassword = false;

    readSetting(doc);

    bool reboot = ((ap_sta_mode != (http_server->arg("ap_sta") == "on")) ||
                   (curMode == WIFI_AP &&
                    (apSsid != http_server->arg("ap_ssid") || apPass != http_server->arg("ap_pass"))) ||
                   (staSsid != http_server->arg("ssid") || staPass != http_server->arg("pass")));
    // staSsid = http_server->arg("ssid");
    // staPass = http_server->arg("pass");
    // apSsid = http_server->arg("ap_ssid");
    // apPass = http_server->arg("ap_pass");
    // ap_sta_mode = http_server->arg("ap_sta") == "true";
    saveConfig();
    // Если изменили опции, требующие перезагрузки, перезапустить модуль
    // if (reboot)
    // {
    //   // redirectPath(0);
    //   ESP.restart();
    // }
    // else
    // {
    //   // redirectPath(1);
    // }}
  }
}

void handleReadApList()
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

  http_server->send(200, "text/json", json);
}

// ===================================================

bool saveConfig()
{
  File configFile;

  bool result = false;

  // удалить существующий файл, иначе конфигурация будет добавлена ​​к файлу
  file_system->remove(fileName);

  // Открыть файл для записи
  configFile = file_system->open(fileName, "w");
  if (!configFile)
  {
    println(F("Failed to create WiFi configuration file"));
    return (result);
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<confSize> doc;

  // задать данные для сохранения
  writeSetting(doc);

  // сериализовать JSON-файл
  result = serializeJson(doc, configFile);

  if (!result)
  {
    println(F("Failed to write WiFi configuration file"));
  }

  configFile.close();
  return (result);
}

bool readSetting(StaticJsonDocument<confSize> &doc)
{
  String _str[] = {ssid_ap_str, ap_pass_str, ssid_str, pass_str,
                   ap_ip_str, ap_mask_str, ip_str, gateway_str, mask_str};
  String *str_val[] = {&apSsid, &apPass, &staSsid, &staPass};

  IPAddress *ip_val[] = {&apIP, &apMask, &staIP, &staGateway, &staMask};

  staticIP = doc[static_ip_str].as<bool>();
  ap_sta_mode = doc[ap_sta_mode_str].as<bool>();

  for (byte i = 0; i < 4; i++)
  {
    String s = doc[_str[i]].as<String>();
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (s != "null")
    {
      *str_val[i] = s;
    }
  }

  for (byte i = 4; i < 9; i++)
  {
    IPAddress ip;
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (ip.fromString(doc[_str[i]].as<String>()))
    {
      *ip_val[i - 4] = ip;
    }
  }
  return true;
}

void writeSetting(StaticJsonDocument<confSize> &doc)
{
  doc[ssid_ap_str] = apSsid;
  doc[ap_pass_str] = apPass;
  doc[ap_ip_str] = apIP.toString();
  doc[ap_mask_str] = apMask.toString();
  doc[ssid_str] = staSsid;
  doc[pass_str] = staPass;
  doc[static_ip_str] = (byte)staticIP;
  doc[ip_str] = staIP.toString();
  doc[gateway_str] = staGateway.toString();
  doc[mask_str] = staMask.toString();
  doc[ap_sta_mode_str] = (byte)ap_sta_mode;
}

void println(String msg)
{
  if (logOnState)
  {
    Serial.println(msg);
  }
}

void print(String msg)
{
  if (logOnState)
  {
    Serial.print(msg);
  }
}
