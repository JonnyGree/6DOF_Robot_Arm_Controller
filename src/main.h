#ifndef main_h
#define main_h

  #include <arduino.h>
  #include <esp_now.h>
  #include <WiFi.h>
  #include <Wire.h>

  #define I2C_SDA 12
  #define I2C_SCL 13

  void SendData();
  void SendToAct6();
  void SendToAct5();
  void SendToAct4();

  void WireSendToAct(char in_char, float in_float);
  void ActSetVel();

  void serialCheck();
  void parseCommand( String com); 

#endif

 
