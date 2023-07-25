#pragma once

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#else
#error "Библиотека предназначена для использования в среде ESP8266 или ESP32"
#endif
#include <FS.h>
#include <ArduinoJson.h>

// ==== Настройки точки доступа ======================
static const String AP_SSID = "WIFI_AP_";
static const String AP_PASS = "12345678";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

// ===== Настройки внешней WiFi-сети =================
static const String STA_SSID = "TP-Link_F6D6_";
static const String STA_PASS = "47439252";
// static const String STA_SSID = "WIFI_SSID";
// static const String STA_PASS = "password";
static const IPAddress STA_IP(192, 168, 0, 50);
static const IPAddress STA_GATEWAY(192, 168, 0, 1);
static const IPAddress STA_MASK(255, 255, 255, 0);

// ===================================================

class shWiFiConfig
{
private:
  bool _fsOK = false;

public:
  shWiFiConfig();

  void setLogOnState(bool log_on);
  void setCurMode(WiFiMode _mode);
  void setApSsid(String ap_ssid);
  void setApPass(String ap_pass);
  void setApIP(IPAddress ap_ip);
  void setApMask(IPAddress ap_mask);
  void setStaSsid(String sta_ssid);
  void setStaPass(String sta_pass);
  void setStaIP(IPAddress sta_ip);
  void setStaGateway(IPAddress sta_gateway);
  void setStaMask(IPAddress sta_mask);
  void setStaticIpMode(bool static_ip);
  void setConfigFileName(String file_name);
  void setApStaMode(bool mode_on);
  void setUseComboMode(bool mode_on);

  bool getLogOnState();
  WiFiMode_t getCurMode();
  String getApSsid();
  String getApPass();
  IPAddress getApIP();
  IPAddress getApMask();
  String getStaSsid();
  String getStaPass();
  IPAddress getStaIP();
  IPAddress getStaGateway();
  IPAddress getStaMask();
  bool getStaticIpMode();
  String getConfigFileName();
  bool getApStaMode();
  bool getUseComboMode();
  bool fsOK() { return (_fsOK); }

  void setApConfig();
  void setApConfig(IPAddress ip);
  void setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask);
  void setStaConfig();
  void setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask);

#if defined(ARDUINO_ARCH_ESP32)
  bool begin(WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#else
  bool begin(ESP8266WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#endif
  bool loadConfig();

  bool startSoftAP();
  bool startSTA();
  bool startSTA(String ssid, String pass);
  bool findSavedAp();
  bool findAp(String ssid);

  void checkStaConnection();
};
