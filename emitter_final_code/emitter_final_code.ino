/*
 * Author: Armin Austin
 * Date: 1/8/2023
 * This program will collect data from an original Xbox controller and sent it to another esp32
 * module using the esp-now protocol
*/
 
// Include the Libraries
#include <esp_now.h>
#include <WiFi.h>
#include <XBOXOLD.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

//Initializing the USB protocol
USB Usb;
USBHub  Hub1(&Usb); // The controller has a built in hub, so this instance is needed
XBOXOLD Xbox(&Usb);

//Defining variables for max travel of the analog sticks
int max_negative = -31869; //Maximum value for travel in negative direction
int max_positive = 31869; //Maximum value for travel in the positive direction

//Variables for controller sticks and triggers
uint8_t lt_val;
uint8_t rt_val;
int16_t ls_val_x;
int16_t ls_val_y;
int16_t rs_val_x;
int16_t rs_val_y;

//Variables for the controller buttons
uint8_t a_btn;
uint8_t b_btn;
uint8_t x_btn;
uint8_t y_btn;
uint8_t blk_btn;
uint8_t wht_btn;
bool start_btn;
bool back_btn;
bool ls_btn;
bool rs_btn;

//Variables for the dpad
bool dpad_up;
bool dpad_down;
bool dpad_left;
bool dpad_right;
 
//MAC Address of the receiver ESP32 module
uint8_t receiver_address[] = {0xEC, 0x62, 0x60, 0x99, 0xC4, 0xCC};
 
//Defining a data structure containing all controller values
typedef struct button_data {
  struct digital_buttons{
    uint8_t dpad_up    : 1;
    uint8_t dpad_down  : 1;
    uint8_t dpad_left  : 1;
    uint8_t dpad_right : 1;
    uint8_t start_btn  : 1;
    uint8_t back_btn   : 1;
    uint8_t ls_btn     : 1;
    uint8_t rs_btn     : 1;
  };
  
  uint8_t a_btn;
  uint8_t b_btn;
  uint8_t x_btn;
  uint8_t y_btn;
  uint8_t blk_btn;
  uint8_t wht_btn;
  uint8_t lt_val;
  uint8_t rt_val;
  int16_t ls_val_x;
  int16_t ls_val_y;
  int16_t rs_val_x;
  int16_t rs_val_y;
} button_data;
 
// Create a structured object
button_data sentData;
 
// Peer info
esp_now_peer_info_t peerInfo;
 
// Callback function called when data is sent
void OnDataSent(const uint8_t * mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t"); //Send status description
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"); //Packet send status
}

/*
// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t * incomingData, int len) {
  memcpy(&sentData, incomingData, sizeof(sentData));
  Serial.print("Left Trigger: ");
  Serial.println(sentData.lt_val); //Displaying value of the left trigger
  Serial.print("Right Trigger: ");
  Serial.println(sentData.rt_val); //Displaying the value of the right trigger
  Serial.print("Left Stick X: ");
  Serial.println(sentData.ls_val_x); //Displaying the value of the left stick in the x-axis
  Serial.print("Left Stick Y: ");
  Serial.println(sentData.ls_val_y); //Displaying the value of the left stick in the y-axis
  Serial.print("LS Button: ");
  Serial.println(sentData.ls_btn); //Displaying the value of the left stick button
  Serial.print("Right Stick X: ");
  Serial.println(sentData.rs_val_x); //Displaying the value of the right stick in the x-axis
  Serial.print("Right Stick Y: ");
  Serial.println(sentData.rs_val_y); //Displaying the value of the right stick in the y-axis
  Serial.print("RS Button: ");
  Serial.println(sentData.rs_btn); //Displaying the value of the right stick button
  Serial.print("A: ");
  Serial.println(sentData.a_btn); //Displaying the value of the "A" button
  Serial.print("B: ");
  Serial.println(sentData.b_btn); //Displaying the value of the "B" button
  Serial.print("Y: ");
  Serial.println(sentData.y_btn); //Displaying the value of the "Y" button
  Serial.print("X: ");
  Serial.println(sentData.x_btn); //Displaying the value of the "X" button
  Serial.print("BLACK: ");
  Serial.println(sentData.blk_btn); //Displaying the value of the BLACK button
  Serial.print("WHITE: ");
  Serial.println(sentData.wht_btn); //Displaying the value of the WHITE button
  Serial.print("DPAD UP: ");
  Serial.println(sentData.dpad_up); //Displaying the value of DPAD UP
  Serial.print("DPAD RIGHT: ");
  Serial.println(sentData.dpad_right); //Displaying the value of DPAD RIGHT
  Serial.print("DPAD DOWN: ");
  Serial.println(sentData.dpad_down); //Displaying the value of DPAD DOWN
  Serial.print("DPAD LEFT: ");
  Serial.println(sentData.dpad_left); //Displaying the value of DPAD LEFT
  Serial.print("Start: ");
  Serial.println(sentData.start_btn); //Displaying the value of the START button
  Serial.print("Back: ");
  Serial.println(sentData.back_btn); //Displaying the value of the BACK button
  Serial.println(); //Creating a new line
}
*/

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
 
 #if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // halt
  }
  Serial.print(F("\r\nXBOX Library Started"));

  WiFi.disconnect();
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, receiver_address, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  //Initializing USB task
  Usb.Task();
  if (Xbox.XboxConnected) { //If an original Xbox controller is detected

    // //////////Controller Directional Pad//////////
    //////////Controller Buttons//////////
    sentData.digital_buttons.dpad_up = Xbox.getButtonClick(UP); //Setting the value of dpad_up
    sentData.digital_buttons.dpad_down = Xbox.getButtonClick(DOWN); //Setting the value of dpad_down
    sentData.digital_buttons.dpad_left = Xbox.getButtonClick(LEFT); //Setting the value of dpad_left
    sentData.digital_buttons.dpad_right = Xbox.getButtonClick(RIGHT); //Setting the value of dpad_right
    sentData.digital_buttons.start_btn = Xbox.getButtonClick(START); //Setting the value of the start_button
    sentData.digital_buttons.back_btn = Xbox.getButtonClick(BACK); //Setting the value of the back_button
    sentData.digital_buttons.ls_btn = Xbox.getButtonClick(L3); //Setting the value of the left stick button
    sentData.digital_buttons.rs_btn = Xbox.getButtonClick(R3); //Setting the value of the right stick button

    sentData.blk_btn = Xbox.getButtonPress(BLACK); //Writing the value of the black button to blk_btn
    sentData.wht_btn = Xbox.getButtonPress(WHITE); //Writing the value of the white button to wht_btn
    //////////Main Controller Buttons//////////
    sentData.a_btn = Xbox.getButtonPress(A); //Writing the value of the A button to a_btn
    sentData.b_btn = Xbox.getButtonPress(B); //Writing the value of the B button to b_btn
    sentData.x_btn = Xbox.getButtonPress(X); //Writing the value of the X button to x_btn
    sentData.y_btn = Xbox.getButtonPress(Y); //Writing the value of the Y button to y_btn

    //////////Controller Trigger Values//////////
    sentData.lt_val = Xbox.getButtonPress(LT); //Writing the value of the left trigger to lt_val
    sentData.rt_val = Xbox.getButtonPress(RT); //Writing the value of the right trigger to rt_val
  

    //////////LEFT STICK X-DIRECTION//////////
    //If the LS moves in the X direction
    if (Xbox.getAnalogHat(LeftHatX) > 7500 || Xbox.getAnalogHat(LeftHatX) < -7500) { 
      ls_val_x = Xbox.getAnalogHat(LeftHatX); //Writing the X value of the left stick to ls_val_x
    }
    //If the LS reaches the end of its travel in the X direction
    else if (Xbox.getAnalogHat(LeftHatX) == 128 || Xbox.getAnalogHat(LeftHatX) == -129){
      //If the stick has maxed out in the negative direction
      if (Xbox.getAnalogHat(LeftHatX) == 128){
        ls_val_x = max_negative; //Setting value of ls_val_x to the max negative value
      }
      //If the stick has maxed out in the positive direction
      else {
        ls_val_x = max_positive; //Setting the value of ls_val_x to the max positive value
      }
    }
    //If the LS has returned back to the origin/back into the deadzone
    else {
      ls_val_x = 0; //Setting the value of ls_val_x to 0
    }

    //////////LEFT STICK Y-DIRECTION//////////
    //If the LS has moved in the Y direction
    if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
      ls_val_y = Xbox.getAnalogHat(LeftHatY); //Writing the Y value of the left to to ls_val_y
    }
    //If the LS reaches the end of its travel in the Y direction
    else if (Xbox.getAnalogHat(LeftHatY) == 128 || Xbox.getAnalogHat(LeftHatY) == -129){
      //If the stick has maxed out in the negative direction
      if (Xbox.getAnalogHat(LeftHatY) == 128){
        ls_val_y = max_negative; //Setting value of ls_val_y to the max negative value
      }
      //If the stick has maxed out in the positive direction
      else {
        ls_val_y = max_positive; //Setting the value of ls_val_y to the max positive value
      }
    } 
    //If the LS has returned back to the origin/back into the deadzone
    else {
      ls_val_y = 0; //Setting the value of ls_val_y to 0
    }   

    //////////RIGHT STICK X-DIRECTION//////////
    //If the RS has moved in the X direction
    if (Xbox.getAnalogHat(RightHatX) > 7500 || Xbox.getAnalogHat(RightHatX) < -7500) {    
      rs_val_x = Xbox.getAnalogHat(RightHatX); //Writing the X value of the right stick to rs_val_x
    }
    //If the RS reaches the end of its travel in the X direction
    else if (Xbox.getAnalogHat(RightHatX) == 128 || Xbox.getAnalogHat(RightHatX) == -129){
      //If the stick has maxed out in the negative direction
      if (Xbox.getAnalogHat(RightHatX) == 128){
        rs_val_x = max_negative; //Setting value of rs_val_x to the max negative value
      }
      //If the stick has maxed out in the positive direction
      else {
        rs_val_x = max_positive; //Setting the value of rs_val_x to the max positive value
      }
    }
    //If the RS has returned back to the origin/back into the deadzone
    else { 
      rs_val_x = 0; //Setting the value of rs_val_x to 0
    }
    
    //////////RIGHT STICK Y-DIRECTION//////////
    //If the RS has moved in the Y direction
    if (Xbox.getAnalogHat(RightHatY) > 7500 || Xbox.getAnalogHat(RightHatY) < -7500) {
      rs_val_y = Xbox.getAnalogHat(RightHatY); //Writing the Y value of the right stick to rs_val_y
    }
    //If the RS reaches the end of its travel in the Y direction
    else if (Xbox.getAnalogHat(RightHatY) == 128 || Xbox.getAnalogHat(RightHatY) == -129){
      //If the stick has maxed out in the negative direction
      if (Xbox.getAnalogHat(RightHatY) == 128){
        rs_val_y = max_negative; //Setting value of rs_val_y to the max negative value
      }
      //If the stick has maxed out in the positive direction
      else {
        rs_val_y = max_positive; //Setting the value of rs_val_y to the max positive value
      }
    }
    //If the RS has returned back to the origin/back into the deadzone
    else {
      rs_val_y = 0; //Setting the value of rs_val_y to 0
    }
  }
  
  //////////Formatting the data to be sent//////////
  // sentData.lt_val = lt_val; //Setting values of the variables in the structure to the global variables from the program
  // sentData.rt_val = rt_val;
  // sentData.ls_val_x = ls_val_x;
  // sentData.ls_val_y = ls_val_y;
  // sentData.rs_val_x = rs_val_x;
  // sentData.rs_val_y = rs_val_y;
  // sentData.a_btn = a_btn;
  // sentData.b_btn = b_btn;
  // sentData.x_btn = x_btn;
  // sentData.y_btn = y_btn;
  // sentData.blk_btn = blk_btn;
  // sentData.wht_btn = wht_btn;
  // sentData.start_btn = start_btn;
  // sentData.back_btn = back_btn;
  // sentData.ls_btn = ls_btn;
  // sentData.rs_btn = rs_btn;
  // sentData.dpad_up = dpad_up;
  // sentData.dpad_down = dpad_down;
  // sentData.dpad_left = dpad_left;
  // sentData.dpad_right = dpad_right;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(receiver_address, (uint8_t *) &sentData, sizeof(sentData));
   
  //If the packet was successfully sent
  if (result == ESP_OK) {
    Serial.println("Sending confirmed"); //Displaying confirmation status
  }
  else { //If the packet failed to send
    Serial.println("Sending error"); //Displaying error message
  }
  
  //Adding a 3ms delay
  delay(1);
}