#include "pinout.h"

void setup() {
  // put your setup code here, to run once:
  int a = BIT0;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A4, OUTPUT);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(A4, HIGH);
  delay(30000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(A4, LOW);
  delay(30000);
}
