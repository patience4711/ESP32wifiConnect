#include "wifiConnect.h"

// Initialize the globale pointer on NULL
wifiConnect* currentPortalInstance = nullptr;

// Constructor
wifiConnect::wifiConnect(uint16_t webServerPort) 
  : server(webServerPort) {
    currentPortalInstance = this; // Koppel deze instantie aan de globale pointer

  // when not rgb pin we declare the pin output
  #if !defined(RGB_BUILTIN) || defined(CONFIG_IDF_TARGET_ESP32C3)
    pinMode(LED_ONB, OUTPUT);
  #endif
}

// Dummy functions
// to return the string like esp32-12345668
String wifiConnect::getChipId(bool dummy) {
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    return "ESP32-" + String(chipId);
}

void wifiConnect::ledOn(uint8_t red, uint8_t green, uint8_t blue) {
  #if defined(RGB_BUILTIN) && !defined(CONFIG_IDF_TARGET_ESP32C3)
    rgbLedWrite(RGB_BUILTIN, red, green, blue);
  #else
    if (red > 0 || green > 0 || blue > 0) {
      digitalWrite(LED_ONB, led_on);
    }
  #endif
}

void wifiConnect::ledOff() {
  #if defined(RGB_BUILTIN) && !defined(CONFIG_IDF_TARGET_ESP32C3)
     rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  #else
     digitalWrite(LED_ONB, led_off); 
  #endif
}

void wifiConnect::ledBlink(uint8_t red, uint8_t green, uint8_t blue, int times, int duration_ms) {
  for (int i = 0; i < times; i++) {
    ledOn(red, green, blue);
    delay(duration_ms);
    ledOff();
    delay(duration_ms);
  }
}

void wifiConnect::wifiConfigsave() {
    prefs.begin("wifi_conf", false);
    prefs.putString("ssid", ssid);
    
    prefs.putString("pswd", pswd);
    prefs.putInt("seclvl", securityLevel);
    prefs.end();
}


// ===========================================================
//                         CORE LOGIC
// ===========================================================

void wifiConnect::start_wifi() {
  
prefs.begin("wifi_conf", true); // Open in READ-ONLY modus (true)

  // read the admin(default "000" if not exists
  String tempPswd = prefs.getString("pswd", "0000");
  strncpy(pswd, tempPswd.c_str(), sizeof(pswd) - 1);
  pswd[sizeof(pswd) - 1] = '\0';

  // read security level (default 6)
  securityLevel = prefs.getInt("seclvl", 6);

  //read the last used ssid (default empty "")
  String tempSsid = prefs.getString("ssid", "");
  strncpy(ssid, tempSsid.c_str(), sizeof(ssid) - 1);
  ssid[sizeof(ssid) - 1] = '\0';

  prefs.end(); // close Preferences

WiFi.mode(WIFI_STA);
  Serial.println("starting wifi ");
  delay(1000);
  WiFi.setHostname(getChipId(false).c_str());
  
  // 10 attempts to connect with the last known credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("*");
    WiFi.begin();
    event += 1;
    if (event == 10) {
      break;
    }
  }
  
  if (event > 9) {
    event = 0;
    Serial.println("\nWARNING connection failed");
    //pinMode(led_onb, OUTPUT);
    //digitalWrite(led_onb, _LED_AAN); // Onboard led on
    ledOn(0, 0, 255);
    start_portal();
    
  } else {
    Serial.print("\nconnection success");
  }
  
  Serial.println("\nconnection attempts = " + String(event));
  event = 0; 
  
    ledBlink(0, 0, 255, 3, 500);
 
}

void wifiConnect::start_portal() {
  WiFi.disconnect(false, true);
  delay(200);
  WiFi.mode(WIFI_AP);
  
  String ap_name = getChipId(false);
  WiFi.softAP(ap_name.c_str());
  
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Frontpage route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, configProcessor);
  });
   
  // Handle form route
  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
    if (currentPortalInstance && 
        request->hasParam("ssid", true) && request->hasParam("password", true) && 
        request->hasParam("pw", true) && request->hasParam("sl", true)) {
      
      String web_ssid = request->getParam("ssid", true)->value();
      strncpy(currentPortalInstance->ssid, web_ssid.c_str(), sizeof(currentPortalInstance->ssid) - 1);
      currentPortalInstance->ssid[sizeof(currentPortalInstance->ssid) - 1] = '\0';

      String web_pass = request->getParam("password", true)->value();
      strncpy(currentPortalInstance->pass, web_pass.c_str(), sizeof(currentPortalInstance->pass) - 1);
      currentPortalInstance->pass[sizeof(currentPortalInstance->pass) - 1] = '\0';

      String web_pwd = request->getParam("pw", true)->value();
      strncpy(currentPortalInstance->pswd, web_pwd.c_str(), sizeof(currentPortalInstance->pswd) - 1);
      
      currentPortalInstance->securityLevel = request->getParam("sl", true)->value().toInt();
      currentPortalInstance->wifiConfigsave();
      
      currentPortalInstance->tryConnectFlag = true; 
      request->redirect("/status");
    } else {
      request->redirect("/");
    }
  });

  // Statuspage route
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", status_html, statusProcessor);
  });

  // Reset route
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>";
    html += "<body><script>";
    html += "alert('The system is going to reset! The ESP32 starts with the new configuration.');";
    html += "window.location.href = 'about:blank';";
    html += "</script></body></html>";
    
    request->send(200, "text/html", html);
    if(currentPortalInstance) {
        currentPortalInstance->laatsteMeting = millis();
        currentPortalInstance->actionFlag = 10;
    }
  });

  // Captive Portal DNS fallback
  server.onNotFound([](AsyncWebServerRequest *request){
    if (request->host() == WiFi.softAPIP().toString()) {
      request->send_P(200, "text/html", index_html);
    } else {
      request->redirect("http://" + WiFi.softAPIP().toString() + "/");
    }
  });

  server.begin();
  Serial.println("ap server started.");
  delay(500);
  Serial.println("entering the infinitive loop with heartbeat");
  laatsteMeting = millis(); 

  static unsigned long heartbeat = 0;
  
  // infinity LOOP inside the portal
  while (millis() < laatsteMeting + 300*1000UL) 
  { 
      if (millis() > heartbeat + 10*1000UL) {
        heartbeat = millis(); 
        Serial.print("ap ");
      }

      if (tryConnectFlag) {
        Serial.println("\n[Portal] received connection request. Starting connection...");
        WiFi.disconnect(false, true); 
        delay(100);
        WiFi.mode(WIFI_AP_STA);
        delay(100);
        WiFi.begin(ssid, pass);
        
        int teller = 0;
        while (WiFi.status() != WL_CONNECTED && teller < 20) {
          delay(500);
          dnsServer.processNextRequest(); 
          teller++;
          Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\n[Portal] Youpy.. Wi-Fi successfully connected!");
          ledBlink(0, 255, 0, 3, 500);
        } else {
          Serial.println("\n[Portal] Wi-Fi connection failed.");
          WiFi.disconnect(); 
        }
        
        tryConnectFlag = false; 
      }
  
      dnsServer.processNextRequest();
      
      if (actionFlag == 10 && (millis() - laatsteMeting > 2000UL)) {
         Serial.println("[Portal] Resetting hardware now...");
         ESP.restart();
      }
      delay(5); 
  } 

  if (wifiConnected || WiFi.status() == WL_CONNECTED) {
      Serial.println("Successfully connected");
  } else {
      Serial.println("portal timed out, resetting...");
  }
  delay(100);
  ESP.restart();
}

// =============================================================
// TEMPLATE PROCESSORS
// =============================================================

String wifiConnect::statusProcessor(const String& var) {
  if (!currentPortalInstance) return String();

  if (var == "STATUS_CONTENT") {
    if (WiFi.status() == WL_CONNECTED) {
      currentPortalInstance->wifiConnected = true;
      String s = "<div class='status-box connected'>";
      s += "SUCCESFULLY CONNECTED!<br><br>";
      s += "THE OBTAINED IP: <b>" + WiFi.localIP().toString() + "</b>";
      s += "</div>";
      s += "<p><small>you can return to your normal wifi network.</small></p>";
      s += "<a href='/reset' class='btn-reset'>Reset System</a>";
      return s;
    } else if (currentPortalInstance->tryConnectFlag) {
      return "<div class='status-box disconnected'>Trying to connect...<br>one moment please.</div>";
    } else {
      String s = "<div class='status-box disconnected'>connection failed mislukt or disconnected.</div>";
      s += "<a href='/' style='color:#007bff;'>Please try again</a>";
      return s;
    }
  }

  if (var == "ADMIN_PSWD") {
    return String(currentPortalInstance->pswd); 
  }
  
  if (var == "SEC_LEVEL") {
    return String(currentPortalInstance->securityLevel); 
  }

  return String();
}

String wifiConnect::configProcessor(const String& var) {
  if (!currentPortalInstance) return "";

  if (var == "HOTSPOT") {
  return String(currentPortalInstance->ssid);
  }  

  if (var == "ADMIN_PSWD") {
    return String(currentPortalInstance->pswd);
  }
  if (var == "SEC_LEVEL") {
    return String(currentPortalInstance->securityLevel);
  }
  return ""; 
}