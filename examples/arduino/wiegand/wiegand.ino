#include "wiegand.h"

#define PIN_D0 0
#define PIN_D1 1

wiegandItf w;

uint64_t data;
int bits;
int noise;

char buff[20];

unsigned long wiegandMicros() {
  return micros();
}

void isrData0() {
  wiegandOnData(&w, 0, digitalRead(PIN_D0));
}

void isrData1() {
  wiegandOnData(&w, 1, digitalRead(PIN_D1));
}

void setup() {
  Serial.begin(9600);
  while(!Serial);

  pinMode(PIN_D0, INPUT); // use INPUT_PULLUP if not using external pull-up
  pinMode(PIN_D1, INPUT); // use INPUT_PULLUP if not using external pull-up

  attachInterrupt(digitalPinToInterrupt(PIN_D0), isrData0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), isrData1, CHANGE);

  wiegandSetup(&w,
    700,  // min pulse interval (usec)
    2700, // max pulse interval (usec)
    10,   // min pulse width (usec)
    150   // max pulse width (usec)
  );

  Serial.println("== Ready ==");
}

void loop() {
  bits = wiegandGetData(&w, &data);
  noise = wiegandGetNoise(&w);

  if (bits > 0) {
    Serial.print("Bits: ");
    Serial.println(bits);
    Serial.print("Data: ");
    println64bitHex(data);
    Serial.println();
  }
  if (noise != 0) {
    Serial.print("Noise: ");
    Serial.println(noise);
    Serial.println();
  }

  delay(100);
}

void println64bitHex(uint64_t val) {
  sprintf(buff, "0x%08lx%08lx",
    ((uint32_t) ((val >> 32) & 0xFFFFFFFF)),
    ((uint32_t) (val & 0xFFFFFFFF)));
  Serial.println(buff);
}
