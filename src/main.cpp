#include <arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <main.h>

#define I2C_SDA 12
#define I2C_SCL 13

boolean DEBUG_ON = true;



// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t Act6[] = {0x3C, 0x71, 0xBF, 0x89, 0x03, 0x84}; 
uint8_t Act5[] = {0x3C, 0x71, 0xBF, 0x88, 0xFE, 0xC0}; 
uint8_t Act4[] = {0xC4, 0x4F, 0x33, 0x11, 0x11, 0xC1};
//act 1-2-3 on i2c

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int Act_Numb;
  char Mode;
  float Position;
  float Val_1;
  float Val_2;
  float Val_3;
} struct_message;

float temp_vel;
float temp_acc;

String command;

// Create a struct_message called myData
struct_message Data_FromPc;
struct_message Data_ToPc;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){   
  if (DEBUG_ON){
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
}
// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&Data_ToPc, incomingData, sizeof(Data_ToPc));  
        //string type 1M10.5|66.3!85.5(78.4         
           Serial.print(Data_ToPc.Act_Numb);Serial.print(",");
           Serial.print(Data_ToPc.Mode);Serial.print(",");
           Serial.print(Data_ToPc.Position);Serial.print(",");
           Serial.print(Data_ToPc.Val_1);Serial.print(",");
           Serial.print(Data_ToPc.Val_2);Serial.print(",");
           Serial.println(Data_ToPc.Val_3);
}

 esp_now_peer_info_t peerInfoAct4;
 esp_now_peer_info_t peerInfoAct5;
 esp_now_peer_info_t peerInfoAct6;
  
void setup() {

  Wire.begin(I2C_SDA, I2C_SCL);  // join i2c bus (address optional for master)

  // Init Serial Monitor
  Serial.begin(115200);

  setCpuFrequencyMhz(80); //Set CPU clock to 80MHz fo example
  btStop();
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);  
  if (DEBUG_ON){  Serial.println(WiFi.macAddress());}
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  // Register peer



  
  memcpy(peerInfoAct4.peer_addr, Act4, 6);
  peerInfoAct4.channel = 8;  
  peerInfoAct4.encrypt = false;
  if (esp_now_add_peer(&peerInfoAct4) != ESP_OK){
    Serial.println("Failed to add peer device 4");
    return;
  }

  memcpy(peerInfoAct5.peer_addr, Act5, 6);
  peerInfoAct5.channel = 9;  
  peerInfoAct5.encrypt = false;
  if (esp_now_add_peer(&peerInfoAct5) != ESP_OK){
    Serial.println("Failed to add peer device 5");
    return;
  }

  memcpy(peerInfoAct6.peer_addr, Act6, 6);
  peerInfoAct6.channel = 10;  
  peerInfoAct6.encrypt = false;
  if (esp_now_add_peer(&peerInfoAct6) != ESP_OK){
    Serial.println("Failed to add peer device 6");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
}

 



void serialCheck() {        //Monitors serial for commands.  Must be called in routinely in loop for serial interface to work.
  if(Serial.available()){

      char c = Serial.read();

      if(c == '\n'){
          parseCommand(command);
          command = "";
      }
      else{
          command += c;
      }
  }
}
void loop() {
 serialCheck();
}

void parseCommand( String com){
  //string type 1M10.5|66.3!85.5(78.4
  Data_FromPc.Act_Numb =com.substring(0,1).toInt();   
  Data_FromPc.Mode = com.charAt(1);
  Data_FromPc.Position = com.substring(2,com.indexOf('|')).toFloat();
  Data_FromPc.Val_1 = com.substring(com.indexOf('|')+1,com.indexOf('!')).toFloat();
  Data_FromPc.Val_2 = com.substring(com.indexOf('!')+1,com.indexOf('(')).toFloat();
  Data_FromPc.Val_3 = com.substring(com.indexOf('(')+1).toFloat();
    if(DEBUG_ON){
      Serial.println(Data_FromPc.Act_Numb);
      Serial.println(Data_FromPc.Mode);
      Serial.println(Data_FromPc.Position);
      Serial.println(Data_FromPc.Val_1);
      Serial.println(Data_FromPc.Val_2);
      Serial.println(Data_FromPc.Val_3);
    }
  SendData();
}

void WireSendToAct(char in_char, float in_float){
  byte raw[] = {0,0,0,0};
  (float&)raw = in_float;
  Wire.beginTransmission(2);
  Wire.write(in_char);
  Wire.write(raw[0]);
  Wire.write(raw[1]);
  Wire.write(raw[2]);
  Wire.write(raw[3]);
  Wire.endTransmission();
  Serial.print("Wire Send: ");Serial.print(in_char);Serial.print(", ");Serial.println(in_float);
}

void ActSetVel(){
    WireSendToAct('D', temp_vel);
    delay(200);
    WireSendToAct('F', temp_acc);
}


void SendToAct4(){
  esp_err_t result = esp_now_send(Act4, (uint8_t *) &Data_FromPc, sizeof(Data_FromPc));   
    if(DEBUG_ON){
          if (result == ESP_OK) {
            Serial.println("Sent with success");
          }
          else {
            Serial.println("Error sending the data");
          } 
    }    
}

void SendToAct5(){
  esp_err_t result = esp_now_send(Act5, (uint8_t *) &Data_FromPc, sizeof(Data_FromPc));   
    if(DEBUG_ON){
          if (result == ESP_OK) {
            Serial.println("Sent with success");
          }
          else {
            Serial.println("Error sending the data");
          } 
    }    
}

void SendToAct6(){
  esp_err_t result = esp_now_send(Act6, (uint8_t *) &Data_FromPc, sizeof(Data_FromPc));   
    if(DEBUG_ON){
          if (result == ESP_OK) {
            Serial.println("Sent with success");
          }
          else {
            Serial.println("Error sending the data");
          } 
    }    
}

void SendData(){   
    if (Data_FromPc.Act_Numb == 1 || Data_FromPc.Act_Numb == 2 || Data_FromPc.Act_Numb == 3){
          if (  Data_FromPc.Mode == 'A' || Data_FromPc.Mode == 'R'){
          WireSendToAct(Data_FromPc.Mode, Data_FromPc.Position );
          }
          if (Data_FromPc.Mode == 'S' ){
          temp_vel = Data_FromPc.Val_1;
          temp_acc = Data_FromPc.Val_2;
          ActSetVel();
          };
    }
    if (Data_FromPc.Act_Numb == 4 ) SendToAct4();
    if (Data_FromPc.Act_Numb == 5 ) SendToAct5();
    if (Data_FromPc.Act_Numb == 6 ) SendToAct6();
} 
