/**
 * Control display TM1637 with protocol bit banging.
 * @author Gabriel P. Bezerra
 * @date 2021-12-26
 */
#define SCL 8
#define SDA 9
#define BIT_DELAY delayMicroseconds(5);

#define TM1637_INIT_COMMAND  0x40
#define TM1637_WRITE_COMMAND 0xC0
#define TM1637_BRIGHT_LOW    0x88
#define TM1637_BRIGHT_MED    0x8A
#define TM1637_BRIGHT_HIGH   0x8F

typedef struct {
  uint8_t scl_pin;
  uint8_t sda_pin;
  uint8_t bright_level;
} tm1637_t;


void tm1637_init(tm1637_t *tm, uint8_t scl, uint8_t sda, uint8_t bright_level) {
  //initialize here
  pinMode(sda, INPUT);
  pinMode(sda, INPUT);
  digitalWrite(sda, LOW);
  digitalWrite(scl, LOW);
  tm->sda_pin = sda;
  tm->scl_pin = scl;
  tm->bright_level = bright_level;
}

bool tm1637_send_data(tm1637_t *tm, uint8_t data, bool send_start, bool send_stop) {
  // start signal
  if (send_start) {
    pinMode(tm->sda_pin, OUTPUT);
    BIT_DELAY
  }
  // send data
  uint8_t b = data;
  for (uint8_t i = 0; i < 8; i++) {
    // Clock low
    pinMode(tm->scl_pin, OUTPUT);
    BIT_DELAY
    
    if (b & 0x01) {
      pinMode(tm->sda_pin, INPUT);
    } else {
      pinMode(tm->sda_pin, OUTPUT);
    }
    BIT_DELAY
    
    // Clock high
    pinMode(tm->scl_pin, INPUT);
    BIT_DELAY
    // Shift to next bit
    b = b >> 1;
  }

  // Wait for ACK
  // Clock low
  pinMode(tm->scl_pin, OUTPUT);
  // Data line high
  pinMode(tm->sda_pin, INPUT);
  BIT_DELAY
  // Clock high
  pinMode(tm->scl_pin, INPUT);
  BIT_DELAY
  // Read SDA/DIO to check for ACK
  uint8_t ack = digitalRead(tm->sda_pin);
  if (ack == 0) {
    pinMode(tm->sda_pin, OUTPUT);
  }
  BIT_DELAY
  pinMode(tm->scl_pin, OUTPUT);
  BIT_DELAY
  // stop signal
  if (send_stop) {
    pinMode(tm->sda_pin, OUTPUT);
    BIT_DELAY
    // Clock High
    pinMode(tm->scl_pin, INPUT);
    BIT_DELAY
    // SDA High
    pinMode(tm->sda_pin, INPUT);
    BIT_DELAY
  }
  
  return ack==0;
}

/**
 * Send a digit to the display TM1637
 * @param tm struct tm1637_t
 * @param digitValue digit value (0 to 9)
 * @param digitPosition digit position (0 to 3)
 * 
 * @return success
 */
bool tm1637_put_digit(tm1637_t *tm, uint8_t digitValue, uint8_t digitPosition) {
  bool send_start = true;
  bool send_stop = true;
  
  if (tm1637_send_data(tm, TM1637_INIT_COMMAND, send_start, send_stop)) {
    if (tm1637_send_data(tm, TM1637_WRITE_COMMAND + digitPosition, send_start, !send_stop)) {
      if(tm1637_send_data(tm, digitValue, !send_start, send_stop)) {
        if (tm1637_send_data(tm, tm->bright_level, send_start, send_stop)) {
          return true;
        }
      }
    }
  }
  
  return false;
}

bool tm1637_put(tm1637_t *tm, uint16_t value) {
  uint8_t t = (value / 1000) % 10;
  tm1637_put_digit(tm, encode_digit(t), 0);
  uint8_t h =  (value / 100) % 10;
  tm1637_put_digit(tm, encode_digit(h), 1);
  uint8_t d =  (value / 10) % 10;
  tm1637_put_digit(tm, encode_digit(d), 2);
  uint8_t u =  value % 10;
  tm1637_put_digit(tm, encode_digit(u), 3);
}

void tm1637_clear(tm1637_t *tm) {
  tm1637_put_digit(tm, encode_digit(0), 0);
  tm1637_put_digit(tm, encode_digit(0), 1);
  tm1637_put_digit(tm, encode_digit(0), 2);
  tm1637_put_digit(tm, encode_digit(0), 3);
}

uint8_t encode_digit(uint8_t digit) {
  uint8_t digits[16] = {
    // XGFEDCBA
    0b00111111,    // 0
    0b00000110,    // 1
    0b01011011,    // 2
    0b01001111,    // 3
    0b01100110,    // 4
    0b01101101,    // 5
    0b01111101,    // 6
    0b00000111,    // 7
    0b01111111,    // 8
    0b01101111,    // 9
    0b01110111,    // A
    0b01111100,    // b
    0b00111001,    // C
    0b01011110,    // d
    0b01111001,    // E
    0b01110001     // F
  };
  
  return digits[digit % 0x10];
}

tm1637_t screen;
uint16_t counter = 0;

void setup() {
  tm1637_init(&screen, SCL, SDA, TM1637_BRIGHT_MED);
}

void loop() {
    tm1637_put(&screen, counter++);
    counter = counter % 10000;
    delay(1);
}
