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

// ===================================================
class shWiFiConfig
{
private:
  bool _fsOK = false;

public:
  shWiFiConfig();
  shWiFiConfig(String adm_name, String adm_pass);

  void setLogOnState(bool log_on);
  void setCurMode(WiFiMode _mode);
  void setApSsid(String ap_ssid);
  void setApPass(String ap_pass);
  void setApIP(IPAddress ap_ip);
  void setApGateway(IPAddress ap_gateway);
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
  void setUseAdminPass(bool pass_on);
  
  void setAdminNameEndPass(String a_name, String a_pass);

  bool getLogOnState();
  WiFiMode_t getCurMode();
  String getApSsid();
  String getApPass();
  IPAddress getApIP();
  IPAddress getApGateway();
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
  bool getUseAdminPass();
  String getAdminPass();
  String getAdminName();

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
