#include <Ticker.h>

#define FLAPCOUNT 62
//#define FLAPCOUNT 40

#define PIN_TRIAC_N D3
#define PIN_STROBE D2
#define PIN_EC_0 D5
#define PIN_EC_1 D1
#define PIN_EC_2 D7
#define PIN_EC_3 D0
#define PIN_EC_4 D6
#define PIN_EC_5 D4
#define STROBE_READ_DELAY 30
#define ENCODER_REREAD_DELAY 4

volatile int read_error_count = 0;
volatile uint8_t encoder_val = 1; // last read value
uint8_t encoder_desired_val = 1; // value to stop at
volatile int strobe_int = 0; // interrupt counter
bool timer_running = false; // a timer is active, don't start a new one
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

void readEncoder() {
  int prev_val = encoder_val;
  int oneflip = prev_val - 1;
  if (oneflip < 1)
    oneflip = FLAPCOUNT;
    
  pinMode(PIN_STROBE, OUTPUT);
  digitalWrite(PIN_STROBE, 0); // pull down strobe, this lets us read the EC pins
  
  encoder_val = digitalRead(PIN_EC_0);
  encoder_val += digitalRead(PIN_EC_1) << 1;
  encoder_val += digitalRead(PIN_EC_2) << 2;
  encoder_val += digitalRead(PIN_EC_3) << 3;
  encoder_val += digitalRead(PIN_EC_4) << 4;
  encoder_val += digitalRead(PIN_EC_5) << 5;
  
  if (encoder_val == 63 || encoder_val != oneflip) { // re-trigger a read if we read an unexpected result. this eliminates the need for real-time finetuning of the read interval.
    encoder_val = prev_val;
    timerReadEncoder.once_ms(ENCODER_REREAD_DELAY, readEncoder);
    read_error_count++;
    return;
  }

  if (encoder_val == encoder_desired_val) {
    digitalWrite(PIN_TRIAC_N, 1);  // stop the motor when we have reached the desired position.
  }
   
  pinMode(PIN_STROBE, INPUT); // strobe back to input, this allows the interrupt to be triggered
  timer_running = false;
  strobe_int = 0;
}

void isr_strobe() {
  strobe_int++;
}

void loop() {
  static int prev_val = 0; // just for printing changes
  
  if (strobe_int > 0 && !timer_running) {
    timer_running = true;
    timerReadEncoder.once_ms(STROBE_READ_DELAY, readEncoder);
  }

  if (strobe_int > 1) {
    Serial.print("b/o ");Serial.println(strobe_int); // bounce or overrun
  }
  if (read_error_count > 0) {
    Serial.print("re ");Serial.println(read_error_count);
    read_error_count = 0;
  }
  
  if (encoder_val != prev_val) { 
    Serial.println(encoder_val);
    prev_val = encoder_val;
  }
  
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == 'a')
      encoder_desired_val++;
    if (ch == 's')
      encoder_desired_val--;
    if (encoder_desired_val < 1)
      encoder_desired_val = FLAPCOUNT;
    if (encoder_desired_val > FLAPCOUNT)
      encoder_desired_val = 1;
    if (ch == 'x')
      digitalWrite(PIN_TRIAC_N, 1);
    if (ch == 'y')
      digitalWrite(PIN_TRIAC_N, 0);
  }
}

