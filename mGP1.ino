///ENUMS - Primarily used as "State Machines"
//Keeps track of where the player is in their jump.
enum JumpState {
  SIT,
  JUMP,
  HANG,
  FALL
};

//Keeps track of the current gamemode
//Flipper mode works similar to a pinball launcher
//Direct mode works similar to a steering wheel
enum GameMode {
  DIRECT,
  FLIP
};

///OTHER GLOBALS - Globals that don't quite fit anywhere else
bool debug = false;

//GAME GLOBALS - Globals directly affecting game environment
GameMode gameMode = FLIP;
long long pillars;
int pillarCooldown = 0;
int pillarRNG = 20;
int tickRate = 300;
int lastTick = millis();

//INPUT GLOBALS - Globals tied to the input
bool held = false;
int readBuffer[30];

//OUTPUT GLOBALS - Globals tied to the output
int _columnPins[] = { 8, 6, 4, 2, 11, 13, A1, A3 };
int _rowPins[] = { 9, 7, 5, 3, 10, 12, A0, A2 };

//CHARACTER GLOBALS - Globals tied to the player character
long long character;
int currentHeight;
int jumpHeight;
int hangStart;
int hangTime = tickRate * 3;
JumpState jumpState = SIT;

//GAMEPLAY CONSTANTS - Constans affecting the gameplay
int minCooldown = 5;
int minimumTickRate = 100;

//CHARACTER DEFAULTS - Constants used for resetting the player character
long long characterDefault = 0x0000000000004000;
int jumpHeightDefault;

//GAMEPLAY DEFAULTS - Constants used for resetting the environment
long long floorCeiling = 0xFF000000000000FF;
int pillarCooldownDefault = 20;
int pillarRNGDefault = 0;
int tickRateDefault = 300;

//DISPLAY DEFAULTS - Constants used for resetting/setting the display
long long defaultPillar = 0x0001010101010100;
long long lastMask = 0x7F7F7F7F7F7F7F7F;
long long letterA = 0x3C7E66667E666666;
long long letterE = 0x7E7E607C7C607E7E;
long long letterG = 0x3C7E606E6E667E3E;
long long letterM = 0x42667E5A42424242;
long long letterO = 0x3C7E666666667E3C;
long long letterR = 0x7C7E627E7C666666;
long long letterV = 0x6666666666263C18;
long long gameOverText[] = {letterG, letterA, letterM, letterE, letterO, 
letterV, letterE, letterR};



///INPUT FUNCTIONS - Functions affecting data input
//Replaces the delay function in order for us to read data while also 
//displaying pixels simultaneously. Since the requeuing takes so long,
//it functions without needing to use millis()
void readWhileDelay() {
  bufferInsert(analogRead(A7));
}

//Allows us to store any data read in while outputting the screen
//In addition, while moving data forward in the queue, checks for
//any zeros and maximum values. If there are no zeros, the user 
//is holding the paddle, which then sets a global variable held. 
//If there are zeros, and held was previously set, that means the user 
//let go of the paddle, and we use the stored maximum value to calculate
//the jump height.
void bufferInsert(int data) {
  bool zero = false;
  int max = 0;

  for (int i = 29; i >= 0; i--) {
    readBuffer[i+1] = readBuffer[i];
    max = (max > readBuffer[i]) ? max : readBuffer[i];
    if (readBuffer[i] == 0) {
      zero = true;
    }
  }
  
  if (held && zero) {
    jumpHeight = max * 5 / 500;
    
    if (jumpHeight > 5) {
      jumpHeight = 5;
    }

    if (jumpState == SIT) jumpState = JUMP;
    held = false;
  }

  else if (!zero && !held) {
    held = true;
  }

  readBuffer[0] = data;
}

//Checks the input state. If the user performs the jumping action (e.g)
//the paddle is brought to 0 where the band normally holds the flipper, 
//the game starts in flip mode. If the paddle is brought to 4095, 
//the opposite end, the game starts in direct mode.
bool checkInput() {
  if (jumpState == JUMP) {
    gameMode = FLIP;
    Serial.print("Starting FLIP\n");
    return 1;
  }
  else if (readBuffer[0] >= 4000) {
    gameMode = DIRECT;
    return 1;
  }
  return 0;
}

///DISPLAY FUNCTIONS - Functions affecting data output
//Sets pins to output.
void setPins(int arr[], int size, bool out) {
  for (int i = 0; i < size; i++){
    pinMode(arr[i], out);
  }
}

//Resets all display pins to show nothing.
void resetAll(int rows[], int cols[], int size) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      digitalWrite(_columnPins[i], HIGH);
      digitalWrite(_rowPins[i], LOW);
    }
  }
}

//Writes an individual row of the 8x8.
void writeRow(char pattern, int cols[], int row, int size) {
  digitalWrite(row, HIGH);
  for (int i = 0; i < size; i++) {
    digitalWrite(cols[i], !((pattern >> i) & 1));
  }
}

//Using writeRow, goes through the 8 rows and writes them with a small
//delay between each write.
void writeScreen(long long pattern, int cols[], int rows[], int size) {
  for (int i = 0; i < size; i++) {
    writeRow(((pattern >> (size * i)) & 255), cols, rows[i], size);
    readWhileDelay();
    digitalWrite(rows[i], LOW);
  }
}

//Writes a screen for an exact length of time using writeScreen
void writeTime(long long pattern, int cols[], int rows[], int size, int timespan = 1000) {
  unsigned long StartTime = millis();
  while (millis() - StartTime < timespan) {
    writeScreen(pattern, cols, rows, size);
  }
}

///GAME FUNCTIONS - Directly affecting aspects of gameplay
//Moves the pillar entities to the left, destroys what would
//fall off the screen.
long long movePillars(long long pillars) {
  return (pillars & lastMask) << 1;
}

//Creates a pillar based on a bit sequence held by a character
long long generatePillar(char type) {
  long long pillar = 0;
  for (int i = 0; i < 6; i++) {
    pillar |= ((long long)(type >> i) & 1) << (8 * (i + 1));
  }
  return pillar;
}

//Reset all modifiable globals to their defaults.
void resetDefaults() {
  while(debug) { writeScreen(letterE, _columnPins, _rowPins, 8); }
  pillarCooldown = pillarCooldownDefault;
  pillarRNG = pillarRNGDefault;
  tickRate = tickRateDefault;
  jumpState = SIT;
  jumpHeight = 0;
  currentHeight = 0;
  character = characterDefault;
}

//If in direct mode, just do simple input division to get position.
//Otherwise, use a state machine to modify the characters position
//and return afterwards.
//SIT - Position and Movement = 0
//JUMP - Position++ until height is reached
//HOLD - Position static until 3 * ticks has pased
//FALL - Position-- until 0 is reached.
int getPosition() {
  if (gameMode == DIRECT) {
    return readBuffer[0] * 6 / 4096;
  }

  switch(jumpState) {
    case SIT: 
      jumpHeight = 0;
      currentHeight = 0;
      break;
    case JUMP:
      if (held) {
        jumpState = SIT;
        jumpHeight = 0;
      }
      else if (currentHeight == jumpHeight) {
        jumpState = HANG;
        hangStart = millis();
      }
      else {
        currentHeight++;
      }
      break;
    case HANG:
      if (millis() - hangStart > hangTime) {
        jumpState = FALL;
      }
      break;
    case FALL:
      if (currentHeight > 0) {
        currentHeight--;
      }
      else {
        jumpState = SIT;
      }
      break;
  }
  return currentHeight;
}

//Infinite loop that doesn't break until the character and columns overlap.
//Places the character based on getPosition. Checks if the tick time has
//been reached. If so, moves the pillars, and adds to the RNG factor
//After a certain threshold is reached, the game has a chance to generate
//a pillar. If the condition occurs, current pillars get shuffled to the
//left and the game generates a random 5 bit number that has a gap in it 
//and sends it to generate pillar. Ors the pillar into the pillars entity.
//Tick rate gets reduced to make the game faster, and hangtime is adjusted
//to the new tick rate. lastTick is set to millis() for timekeeping.
//After, writes the screen.
long long gamePlay() {
  resetDefaults();
  while(!(character & pillars)) {
    character = characterDefault << (getPosition() * 8);
    if (millis() - lastTick > tickRate) {
      pillars = movePillars(pillars);
      if (random(0, pillarRNG++) > pillarCooldown) {
        pillars |= generatePillar(random(1, 62));
        pillarRNG = 0;
        tickRate -= (tickRate > minimumTickRate) ? 20 : 0;
        hangTime = tickRate * 3;
      }
      lastTick = millis();
    }
    writeScreen((floorCeiling | character | pillars), _columnPins, _rowPins, 8);
  }
}

//Infinite loop until the player moves the flipper to one end or the other.
//Displays an outro sequence where the character gets covered by 
//undodgeable columns and displays a sequence of GAMEOVER. Once a paddle
//input is detected, selects the gamemode and releases the player with a
//brief outro.
void gameOver() {
  for (int i = 0; i < 8; i++) {
    pillars = movePillars(pillars);
    pillars |= defaultPillar;
    writeTime((floorCeiling | characterDefault | pillars), _columnPins, _rowPins, 8, 100);
  }

  int letter = 0;
  bool pressed = false;
  int lastDisplay = millis();
  jumpState = SIT;
  
  while (!checkInput()) {
    if (millis() - lastDisplay > 333) {
      lastDisplay = millis();
      letter = (letter + 1) & 7;
    }
    writeScreen(~gameOverText[letter], _columnPins, _rowPins, 8);
  }

  while (pillars) {
    pillars = movePillars(pillars);
    writeTime((floorCeiling | pillars), _columnPins, _rowPins, 8, 100);
  }
}

//Sets the read resolution higher, seeds the random generator with noise,
//and sets output pins to display nothing.
void setup() {
  analogReadResolution(12);
  randomSeed(analogRead(0));
  setPins(_columnPins, 8, OUTPUT);
  setPins(_rowPins, 8, OUTPUT);
  resetAll(_rowPins, _columnPins, 8);
}

//Gameplay and GameOver continuously between one another.
void loop() {
  gameOver();
  gamePlay();
}