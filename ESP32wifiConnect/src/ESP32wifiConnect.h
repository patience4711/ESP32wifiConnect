#ifndef ESP32WIFICONNECT_H
#define ESP32WIFICONNECT_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> // Zorg dat de AsyncWebServer library geïnstalleerd is
#include <Preferences.h>
#include <pgmspace.h>

inline const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Wi-Fi Setup</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 40px; background-color: #f4f4f9; color: #333; }
    .container { display: inline-block; padding: 30px; background: white; border-radius: 8px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); width: 80%%; max-width: 400px; text-align: left; }
    h2 { text-align: center; }
    table { width: 100%%; border-collapse: collapse; }
    td { padding: 8px 5px; vertical-align: middle; }
    
    /* Basis styling voor alle tekst- en wachtwoordvelden */
    input[type=text], input[type=password] { width: 100%%; height: 36px; padding: 0 10px; margin: 0; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 14px; }
    
    /* Compacte styling voor het getallenveld (exact even hoog als de rest) */
    input[type=number].compact-num { width: 50px; height: 36px; padding: 0; margin: 0; text-align: center; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 14px; }
    
    input[type=submit] { width: 100%%; background-color: #007bff; color: white; padding: 14px 20px; margin: 15px 0 0 0; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>ESP32 Wi-Fi Setup</h2>
    <form action="/connect" method="POST">
      <table>
        <tr>
          <td>Wi-Fi SSID:</td>
          <td><input type="text" name="ssid" placeholder="e.g. homenetwork" value="%HOTSPOT%" required></td>
        </tr>
        <tr>
          <td>Wi-Fi Wachtwoord:</td>
          <td><input type="text" name="password" placeholder="Wachtwoord" required></td>
        </tr>
        <tr>
          <td>Admin passwd:</td>
          
          <td><input type="text" value="%ADMIN_PSWD%" name="pw" maxlength="10" placeholder="max. 10 tekens" pattern=".{4,10}" title="tussen 4 en 10 karakters" required></td>
        </tr> 
        <tr>
          <td>Security level:</td>
          <td><input type="number" value="%SEC_LEVEL%" min="0" max="9" name="sl" class="compact-num" required> (0-9)</td>
        </tr>
      </table>
      <input type="submit" value="Connect & Save">
    </form>
  </div>
</body>
</html>
)=====";

// STATUSPAGINA (shows connection, succes + ip, of resetknop)
inline const char status_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Status</title>
  <!-- refresh the page every 3 secs to get the state -->
  <meta http-equiv="refresh" content="3">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 40px; background-color: #f4f4f9; }
    .container { display: inline-block; padding: 30px; background: white; border-radius: 8px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); width: 80%%; max-width: 400px; }
    .status-box { font-size: 1.1em; font-weight: bold; padding: 15px; margin: 15px 0; border-radius: 4px; }
    .connected { background-color: #d4edda; color: #155724; }
    .disconnected { background-color: #fff3cd; color: #856404; }
    .btn-reset { display: inline-block; background-color: #dc3545; color: white; padding: 10px 20px; text-decoration: none; border-radius: 4px; margin-top: 15px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>Connection state</h2>
    %STATUS_CONTENT%
  </div>
</body>
</html>
)=====";

class ESP32wifiConnect {
  public:
    // Constructor (add the port for the webserver, standaard 80)
    ESP32wifiConnect(uint16_t webServerPort = 80);

    // Public basefunction to start the process
    void start_wifi();
    void ledBlink(uint8_t red, uint8_t green, uint8_t blue, int times, int duration_ms);
    void ledOn(uint8_t red, uint8_t green, uint8_t blue);
    void ledOff();
  
  private:
    // Internal portal- and serverfunctions
    void start_portal();
    
    void wifiConfigsave(); // save settings in Preferences
    String getChipId(bool dummy);
    
    // Template processors (must be 'static' or smart chained , in the .cpp)
    static String statusProcessor(const String& var);
    static String configProcessor(const String& var);

    // Server & DNS instanties
    AsyncWebServer server;
    DNSServer dnsServer;
    Preferences prefs;

    // Constants
    const byte DNS_PORT = 53;


    #if defined(CONFIG_IDF_TARGET_ESP32C3)
      const uint8_t LED_ONB = 8;
      const uint8_t  led_on     = LOW;   // C3 is Active Low
      const uint8_t  led_off    = HIGH;
    #else
      const uint8_t LED_ONB = 2;
      const uint8_t  led_on     = HIGH;  // Dev is Active High
      const uint8_t  led_off    = LOW;
    #endif

    // Internal variables)
    int event = 0;
    char ssid[32] = {0};
    char pass[64] = {0};
    char pswd[20] = "admin";
    int securityLevel = 0;
    char requestUrl[64] = "/";
    
    // Flags en timers
    volatile bool tryConnectFlag = false;
    bool wifiConnected = false;
    unsigned long laatsteMeting = 0;
    int actionFlag = 0;
    int random_data[3] = {0}; // Gebruikt bij je led-test
};

// Pointer to the actieve instance (needed for static processors & AsyncWebServer lambdas)
    extern ESP32wifiConnect* currentPortalInstance;

#endif