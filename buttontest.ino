#define LED1 2 //Defining the red LED pin
#define LED2 4 //Defining the green LED pin
#define BTN 5 //Defining the button pin
 
hw_timer_t *Timer0_Cfg = NULL; //Main button press timer
hw_timer_t *Timer1_Cfg = NULL; //LED shutoff timer
bool timer_btn_flag = false, timer_pulse_flag = false, button_flag = false;
bool long_press = false; //Flag to register long press

//Main button timer method
void IRAM_ATTR Timer0_ISR() {
  //Setting timer_btn_flag true = button was held for 1.5s
  timer_btn_flag = true;
}

//LED Shutoff timer method
void IRAM_ATTR Timer1_ISR() {
  //timer_pulse_flag = Time has reached shutoff time
  timer_pulse_flag = true;
}

//Interrupt method for button press
void BTN_ISR(){
  //button_flag = If the button has been pressed 
  button_flag = true;
}

void setup() {
    Serial.begin(9600); //Starting serial monitor
    pinMode(LED1, OUTPUT); //Setting red LED as output
    pinMode(LED2, OUTPUT); //Setting green LED as output
    pinMode(BTN, INPUT); //Setting button pin as input
    attachInterrupt(digitalPinToInterrupt(BTN), BTN_ISR, CHANGE); //Adding a change interrupt to the button pin

    // Button press timer
    Timer0_Cfg = timerBegin(0, 80, true); //Assigning tick value
    timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
    timerAlarmWrite(Timer0_Cfg, 1500000, true); //Max value of 1.5s
    timerAlarmEnable(Timer0_Cfg);
    timerStop(Timer0_Cfg);

    // Pulse timer
    Timer1_Cfg = timerBegin(1, 80, true); //Assigning tick value
    timerAttachInterrupt(Timer1_Cfg, &Timer1_ISR, true);
    timerAlarmWrite(Timer1_Cfg, 500000, true); //Max value of 0.5s
    timerAlarmEnable(Timer1_Cfg);
    timerStop(Timer1_Cfg);
}
void loop()  {
  //If the button has been pressed
    if(button_flag){
      button_flag = false; //Resetting the flag
      if(digitalRead(BTN) == HIGH && !long_press){ //If the button is released and has not been long pressed
        digitalWrite(LED1, HIGH); //Turning on red LED
        timerRestart(Timer1_Cfg); //Resetting pulse timer
        timerStart(Timer1_Cfg); //Starting pulse timer
        timerStop(Timer0_Cfg); //Stopping main timer
        Serial.println("Button released");
      } 
      
      //If the button is held
      else if(digitalRead(BTN) == LOW){
        timerRestart(Timer0_Cfg); //Resetting the main timer
        timerStart(Timer0_Cfg); //Restarting the main timer
        long_press = false; //Resetting "long_press" flag
      }
    }
    
    // LED green triggered
    if(timer_btn_flag){
      timer_btn_flag = false;
      long_press = true;
      digitalWrite(LED2, HIGH);
      timerRestart(Timer1_Cfg);
      timerStart(Timer1_Cfg);
      timerStop(Timer0_Cfg);
      Serial.println("Timer triggered");
    }

    //If the 0.5s delay is reached
    if(timer_pulse_flag){
      timer_pulse_flag = false; //Resetting the "timer_pulse_flag"
      //Turning off both LEDs
      digitalWrite(LED1, LOW); 
      digitalWrite(LED2, LOW);

      timerStop(Timer1_Cfg); //Stopping the pulse timer
      Serial.println("Turned off");
    }
}