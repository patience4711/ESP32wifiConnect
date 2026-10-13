# Simple-Wifi-Connect-ESP32C3-C6
very simple and lightweighted library to connect your ESP32c3 or esp32c6 to wifi

The nice thing about it is that is tells you the ip when the connection is made.

There also an option to set an admin password so that your webui can be secured.
Another saved value is security level. This can be used to determin how many characters
should match with the ip of your router. This is handy when you want to hide the menu
when you are outside your own home network.

How to use it:
you need to have 'preferences' as the data is saved in flash

copy the zip to your sketchbook/libraries
in arduino IDE 'include library'-> 'add zip library'
in your program in the global part before setup add:
  #include <wifiConnect.h>
  wifiConnect my_wifiConnect;
in setup: my_wifiConnect.start_wifi();

if you want to read the admin passwd and security level you need something like:<br>
void readAdminPasswd()
{<br>
  prefs.begin("wifi_conf", true);<br>
  String temp = prefs.getString("pswd", "0000");<br>
  strncpy(pswd, temp.c_str(), sizeof(pswd) - 1);<br>
  pswd[sizeof(pswd) - 1] = '\0'; // null-terminator <br> 
  securityLevel  = prefs.getInt("seclvl", 6);<br>
  prefs.end();<br>
}<br>
