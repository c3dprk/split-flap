#include <Ticker.h>

#define PIN_TRIAC_N D3
#define PIN_STROBE D2
#define PIN_EC_0 D5
#define PIN_EC_1 D1
#define PIN_EC_2 D7
#define PIN_EC_3 D0
#define PIN_EC_4 D6
#define PIN_EC_5 D4

uint8_t encoder_val = 0;
uint8_t encoder_desired_val = 0;
volatile int strobe_int = 0;
int timer_running = 0;
Ticker timerReadEncoder;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIAC_N, OUTPUT);
  pinMode(PIN_STROBE, INPUT);
  pinMode(PIN_EC_0, INPUT);
  pinMode(PIN_EC_1, INPUT);
  pinMode(PIN_EC_2, INPUT);
  pinMode(PIN_EC_3, INPUT);
  pinMode(PIN_EC_4, INPUT);
  pinMode(PIN_EC_5, INPUT);
  digitalWrite(PIN_TRIAC_N, 1);
  digitalWrite(PIN_STROBE, 0);
  attachInterrupt(digitalPinToInterrupt(PIN_STROBE), isr_strobe, FALLING);
}

void readEncoder(){
  int prev_val = encoder_val;
  pinMode(PIN_STROBE, OUTPUT);
  digitalWrite(PIN_STROBE, 0);
  
  encoder_val = digitalRead(PIN_EC_0);
  encoder_val += digitalRead(PIN_EC_1) << 1;
  encoder_val += digitalRead(PIN_EC_2) << 2;
  encoder_val += digitalRead(PIN_EC_3) << 3;
  encoder_val += digitalRead(PIN_EC_4) << 4;
  encoder_val += digitalRead(PIN_EC_5) << 5;
  
  pinMode(PIN_STROBE, INPUT);

  if(encoder_val == 63 || encoder_val != prev_val + 1){
    timerReadEncoder.once_ms(1, readEncoder);
    return;
  }

  if(encoder_val == encoder_desired_val){
    digitalWrite(PIN_TRIAC_N, 1); 
  }
   
  timer_running = 0;
  strobe_int = 0;
}

void isr_strobe(){
  strobe_int++;
}

void loop() {
  static int prev_val = 0;
  static int display_pos = 1;
  
  if(strobe_int > 0 && timer_running < 1){
    timer_running++;
    timerReadEncoder.once_ms(8, readEncoder);
  }
  
  if(encoder_val != prev_val && encoder_val != 63){ 
    Serial.println(encoder_val);
    prev_val = encoder_val;
  }

  if(encoder_val != encoder_desired_val){
    digitalWrite(PIN_TRIAC_N, 0);
  }else{
    digitalWrite(PIN_TRIAC_N, 1);
  }
  
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if(ch == 'a')
      encoder_desired_val++;
    if(ch == 's')
      encoder_desired_val--;
    if(encoder_desired_val < 1)encoder_desired_val=62;
    if(encoder_desired_val > 62)encoder_desired_val=1;
    if(ch == 'x')
      digitalWrite(PIN_TRIAC_N, 1);
    if(ch == 'y')
      digitalWrite(PIN_TRIAC_N, 0);
  }
}

