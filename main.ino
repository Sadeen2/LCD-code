// Manual LCD connection without using a library
#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2

#define DHTPIN A0
#define LDRPIN A1

// ---------------- LCD Functions ----------------

void pulseEnable() {
  digitalWrite(EN, LOW);
  delayMicroseconds(1);
  digitalWrite(EN, HIGH);
  delayMicroseconds(1);
  digitalWrite(EN, LOW);
  delayMicroseconds(100);
}

void send4Bits(byte data) {
  digitalWrite(D4, (data >> 0) & 0x01);
  digitalWrite(D5, (data >> 1) & 0x01);
  digitalWrite(D6, (data >> 2) & 0x01);
  digitalWrite(D7, (data >> 3) & 0x01);
}

void sendCommand(byte cmd) {
  digitalWrite(RS, LOW);
  send4Bits(cmd >> 4);
  pulseEnable();
  send4Bits(cmd);
  pulseEnable();
  delay(2);
}

void sendData(byte data) {
  digitalWrite(RS, HIGH);
  send4Bits(data >> 4);
  pulseEnable();
  send4Bits(data);
  pulseEnable();
  delay(2);
}

void lcdClear() {
  sendCommand(0x01);
  delay(2);
}

void lcdSetCursor(byte col, byte row) {
  byte addr = col + (row == 1 ? 0x40 : 0x00);
  sendCommand(0x80 | addr);
}

void lcdPrint(const char *str) {
  while (*str) {
    sendData(*str++);
  }
}

void lcdCreateChar(byte location, byte charmap[]) {
  location &= 0x07;
  sendCommand(0x40 | (location << 3));
  for (int i = 0; i < 8; i++) {
    sendData(charmap[i]);
  }
}

void lcdInit() {
  pinMode(RS, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  delay(50);
  send4Bits(0x03); pulseEnable(); delay(5);
  send4Bits(0x03); pulseEnable(); delay(5);
  send4Bits(0x03); pulseEnable(); delay(5);
  send4Bits(0x02); pulseEnable();
  sendCommand(0x28); // 4-bit mode, 2 lines
  sendCommand(0x0C); // Display ON
  sendCommand(0x06); // Cursor increment
  lcdClear();
}

// ---------------- Custom Icons ----------------

byte icon_temp[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00000
};

byte icon_light[8] = {
  0b00100,
  0b10101,
  0b01110,
  0b11111,
  0b01110,
  0b10101,
  0b00100,
  0b00000
};

// ---------------- Reading DHT11 Without Library ----------------

uint8_t readDHT() {
  uint8_t bits[5];
  uint8_t cnt = 7;
  uint8_t idx = 0;
  memset(bits, 0, sizeof(bits));

  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  delay(20);
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHTPIN, INPUT);

  if (digitalRead(DHTPIN) == HIGH) return 0; // Error
  
  while (digitalRead(DHTPIN) == LOW);
  while (digitalRead(DHTPIN) == HIGH);

  for (int i = 0; i < 40; i++) {
    while (digitalRead(DHTPIN) == LOW);
    unsigned long t = micros();
    while (digitalRead(DHTPIN) == HIGH);
    if ((micros() - t) > 40)
      bits[idx] |= (1 << cnt);
    if (cnt == 0) {
      cnt = 7;
      idx++;
    } else {
      cnt--;
    }
  }
  return bits[2]; // Only return temperature
}

// ---------------- General Setup ----------------

unsigned long prevMillis = 0;
bool showTemp = true;

void setup() {
  lcdInit();
  lcdCreateChar(0, icon_temp);
  lcdCreateChar(1, icon_light);
}

// ---------------- Main Loop ----------------

void loop() {
  if (millis() - prevMillis >= 2000) {
    prevMillis = millis();
    lcdClear();
    lcdSetCursor(0, 0);

    if (showTemp) {
      sendData(0); // Temperature icon
      lcdPrint(" Temp: ");
      uint8_t temp = readDHT();
      lcdSetCursor(0, 1);
      char buf[8];
      sprintf(buf, "%d C", temp);
      lcdPrint(buf);
    } else {
      sendData(1); // Light icon
      lcdPrint(" Light:");
      int val = analogRead(LDRPIN);
      lcdSetCursor(0, 1);
      char buf[8];
      sprintf(buf, "%d", val);
      lcdPrint(buf);
    }

    showTemp = !showTemp;
  }
}
