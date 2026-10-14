# Simple-Wifi-Connect-ESP32C3-C6
very simple and lightweighted library to connect your ESP32c3 or esp32c6 to your homenetwork (wifi)

The nice thing about it is that is tells you the ip when the connection is made.

There also an option to set an admin password so that your webui can be secured.
Another saved value is security level. This can be used to determin how many characters
should match with the ip of your router. This is handy when you want to hide the menu
when you are outside your own home network.
How does it work:
When you power up your esp, the onboard led goes on, then you know the ap is onened. Open de network page on your pc or phone, and you will see a network like ESP32-12436587. Connect to this network. Then you get this screen:<br>
<img width="492" height="429" alt="ui2" src="https://github.com/user-attachments/assets/ef86853f-1c74-4591-b2c7-84f89f6b54f3" />
<br>When you filled up the data and submit, you get this screen:<br>
<img width="478" height="350" alt="ui3" src="https://github.com/user-attachments/assets/8bf61a41-70d4-48f8-9fa3-f94658599d15" /><br>
Note down the ip and click the reset button. 


How to install it:<br>
you need to have 'preferences' as the data is saved in flash.<br>

copy the zip to your sketchbook/libraries<br>
in arduino IDE 'include library' -> 'add zip library'<br>
in your program in the global part above setup add:<br>
  - #include <wifiConnect.h><br>
  - wifiConnect my_wifiConnect;<br>
  - in setup: my_wifiConnect.start_wifi();<br>

if you want use the admin passwd and/or security level you have to read it into your program,<br>
you need something like this reading function:<br>
void readAdminPasswd()<br>
{<br>
  prefs.begin("wifi_conf", true);<br>
  String temp = prefs.getString("pswd", "0000");<br>
  strncpy(pswd, temp.c_str(), sizeof(pswd) - 1);<br>
  pswd[sizeof(pswd) - 1] = '\0'; // null-terminator <br> 
  securityLevel  = prefs.getInt("seclvl", 6);<br>
  prefs.end();<br>
}<br>
