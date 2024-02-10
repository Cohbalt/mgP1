int _ledPins[] = { 02, 03, 04, 05};

void flash(int led, int del) {
  digitalWrite(led, HIGH);
  delay(del);
  digitalWrite(led, LOW);
}

void flashMult(int led[], int size, int del) {
  int i = 0;
  for (i = 0; i < size; i++) {
    digitalWrite(led[i], HIGH);
  }
  
  delay(del);
  
  for (i = 0; i < size; i++) {
    digitalWrite(led[i], LOW);
  }
}

void displayBinary(int led[], int size, int num) {
  for (int i = 0; i < size; i++) {
    digitalWrite(led[i], (num >> i) & 1);
  }
}

void setup() {
  pinMode(_ledPins[0], OUTPUT);
  pinMode(_ledPins[1], OUTPUT);
  pinMode(_ledPins[2], OUTPUT);
  pinMode(_ledPins[3], OUTPUT);
}

void loop() {
  flash(_ledPins[0], 2000);
  flash(_ledPins[1], 1000);
  flash(_ledPins[2], 500);
  flash(_ledPins[3], 250);

    for (int i = 0; i < 16; i++) {
    displayBinary(_ledPins, 4, i);
    delay(500);
  }

  flashMult(_ledPins, 4, 1000);
  delay(1000);
  flashMult(_ledPins, 4, 1000);
  delay(1000);
  flashMult(_ledPins, 4, 1000);
  delay(1000);
  flashMult(_ledPins, 4, 1000);

  delay(500);
}