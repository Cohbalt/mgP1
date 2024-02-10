int _columnPins[] = { 8, 6, 4, 2, A1, A3, A5, A7};
int _rowPins[] = { 9, 7, 5, 3, A0, A2, A4, A6 };
long long happy = 0x86a41e2c86a41e2c;
long long letterO = 0x7EE7C3C3C3C3E77E;
long long letterK = 0x63666C786C666361;

void setPins(int arr[], int size, bool out) {
  for (int i = 0; i < size; i++){
    pinMode(arr[i], out);
  }
}

void resetAll(int rows[], int cols[], int size) {
    for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      digitalWrite(_columnPins[i], HIGH);
      digitalWrite(_rowPins[i], LOW);
    }
  }
}

void writeRow(char pattern, int cols[], int row, int size) {
  digitalWrite(row, HIGH);
  for (int i = 0; i < size; i++) {
    digitalWrite(cols[i], !((pattern >> i) & 1));
  }
}

void writeScreen(long long pattern, int cols[], int rows[], int size, int refresh) {
  for (int i = 0; i < size; i++) {
    writeRow(((pattern >> (size * i)) & 255), cols, rows[i], size);
    delay(refresh);
    digitalWrite(rows[i], LOW);
  }
}

void writeTime(long long pattern, int cols[], int rows[], int size, int refresh, int timespan = 1000) {
  unsigned long StartTime = millis();
  while (millis() - StartTime < timespan) {
    writeScreen(pattern, cols, rows, size, refresh);
  }
}

void setup() {
  setPins(_columnPins, 8, OUTPUT);
  setPins(_rowPins, 8, OUTPUT);
  resetAll(_rowPins, _columnPins, 8);
}

void loop() {
  // put your main code here, to run repeatedly:
  writeTime(letterO, _columnPins, _rowPins, 8, 1, 1000);
  writeTime(letterK, _columnPins, _rowPins, 8, 1, 1000);
  writeTime(~letterO, _columnPins, _rowPins, 8, 1, 1000);
  writeTime(~letterK, _columnPins, _rowPins, 8, 1, 1000);
}
