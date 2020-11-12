// ====================
//   Settings
// ====================

const int NUMROWS = 3;
const int NUMCOLS = 8;

// pin numbers for rows
const int row[NUMROWS] = {
  16, // top row
  15,
  14  // bottom row
};

// pin numbers for columns
const int col[NUMCOLS] = {
  8, 7, 10, 6, 5, 4, 3, 2 // 8 = far left, 2 = far right
};

// pin number for button used to set time
const int timeSetButton = 19;

// if true, clock will be 24 hour
// if false, clock will be 12 hour
boolean is24Hour = true;
// TODO add switch to control this



// ====================
//   Setup
// ====================

// the time that the user selects is the "base time"
// until the user seets it, the base time is 12:00 AM (0 hours, 0 minutes, 0 seconds)
unsigned long baseHours = 0;
unsigned long baseMinutes = 0;
unsigned long baseSeconds = 0;

// TODO consider using `unsigned long long` to fix the clock rolling over at
// 49.7 days (since max unsigned long value is roughly 49.7 days in
// milliseconds
unsigned long timeThatTimeWasSetByUser = 0;

// 0 = display the time as normal
// 1 = accept input for hours
// 2 = accept input for minutes
// 3 = accept input for seconds
int currentMode = 0;

unsigned long timeThatModeWasLastChanged = 0;

int screen[NUMROWS][NUMCOLS];

void setup() {
  // initialize every "pixel" (LED) in the "screen" to off
  for (int i = 0; i < NUMROWS; i++) {
    for (int j = 0; j < NUMCOLS; j++) {
      screen[i][j] = 0;
    }
  }

  for (int r = 0; r < NUMROWS; r++) { // for each row,
    pinMode(row[r], OUTPUT);       // set mode to output
    digitalWrite(row[r], LOW);     // then open it up to input
  }

  for (int c = 0; c < NUMCOLS; c++) { // for each col,
    pinMode(col[c], OUTPUT);       // set mode to output
    digitalWrite(col[c], LOW);     // then make sure the whole column is off
  }

  pinMode(timeSetButton, INPUT_PULLUP);

  Serial.begin(9600);
}



// ====================
//   Loop
// ====================

void loop() {
  handleDisplay();

  handleButtonInput();
}



// ====================
//   Display
// ====================

void handleDisplay() {
  unsigned long millisecondsSinceModeWasLastChanged = millis() - timeThatModeWasLastChanged;

  switch (currentMode) {
    case 0:
      // display time as usual
      setCurrentTimeOnScreen();

      break;

    case 1:
      // flash baseHours
      if ((millisecondsSinceModeWasLastChanged / 250) % 2 > 0) {
        // show for 250 ms and hide for 250 ms
        setHoursOnScreen(0);

      } else {
        setHoursOnScreen(baseHours);
      }

      setMinutesOnScreen(0);
      setSecondsOnScreen(0);

      break;

    case 2:
      // display baseHours & flash baseMinutes
      setHoursOnScreen(baseHours);

      if ((millisecondsSinceModeWasLastChanged / 250) % 2 > 0) {
        // show for 250 ms and hide for 250 ms
        setMinutesOnScreen(0);

      } else {
        setMinutesOnScreen(baseMinutes);
      }

      setSecondsOnScreen(0);

      break;

    case 3:
      // display baseHours & baseMinutes & flash baseSeconds
      setHoursOnScreen(baseHours);
      setMinutesOnScreen(baseMinutes);

      if ((millisecondsSinceModeWasLastChanged / 250) % 2 > 0) {
        // show for 250 ms and hide for 250 ms
        setSecondsOnScreen(0);

      } else {
        setSecondsOnScreen(baseSeconds);
      }

      break;
  }

  drawScreen();
}

void setCurrentTimeOnScreen() {
  unsigned long currentTimeInSeconds = getCurrentTimeInSeconds();
  unsigned long currentHours         = getCurrentHours(currentTimeInSeconds);
  unsigned long currentMinutes       = getCurrentMinutes(currentTimeInSeconds);
  unsigned long currentSeconds       = getCurrentSeconds(currentTimeInSeconds);

  setHoursOnScreen(currentHours);
  setMinutesOnScreen(currentMinutes);
  setSecondsOnScreen(currentSeconds);
}

void setHoursOnScreen(unsigned long hours) {
  setNumberAsBinaryOnRow(hours, 0);
}

void setMinutesOnScreen(unsigned long minutes) {
  setNumberAsBinaryOnRow(minutes, 1);
}

void setSecondsOnScreen(unsigned long seconds) {
  setNumberAsBinaryOnRow(seconds, 2);
}

void setNumberAsBinaryOnRow(unsigned long number, int row) {
  for (int c = 1; c <= NUMCOLS; c++) {
    screen[row][NUMCOLS - c] = number % 2;

    number >>= 1;
  }
}

void clearAllLEDs() {
  for (int r = 0; r < NUMROWS; r++) {
    digitalWrite(row[r], HIGH);
  }

  for (int c = 0; c < NUMCOLS; c++) {
    digitalWrite(col[c], LOW);
  }
}

void drawScreen() {
  for (int r = 0; r < NUMROWS; r++) {
    // turn off all LEDs before displaying each row
    clearAllLEDs();

    // open up only the current row for displaying
    digitalWrite(row[r], LOW);

    for (int c = 0; c < NUMCOLS; c++) {
      if (screen[r][c] > 0) {
        digitalWrite(col[c], HIGH);
      }
    }

    delay(5);
  }
}



// ====================
//   Button Input
// ====================

// 0 = button down
// 1 = button up
int previousButtonState = 1;

// time button switched from up to down
unsigned long buttonDownTime = 0;

// time button switched from down to up
unsigned long buttonUpTime = 0;

void handleButtonInput() {
  int newButtonState = digitalRead(timeSetButton);
  boolean isShortPress = false;
  boolean isLongPress = false;

  if (previousButtonState == 1 && newButtonState == 0) { // on button down
    buttonDownTime = millis();

  } else if (previousButtonState == 0 && newButtonState == 1) {      // on button up
    buttonUpTime = millis();
  }

  int pressDuration = buttonUpTime - buttonDownTime;

  if (pressDuration > 0 && pressDuration < 1000) {
    // if button went down and back up in under 1 second
    isShortPress = true;

  } else if (buttonDownTime > 0 && millis() - buttonDownTime > 1000) {
    // if button is held down for longer than 1 second
    isLongPress = true;
  }

  switch (currentMode) {
    case 0:
      // if long press, switch to mode 1 and set base time to current time
      if (isLongPress) {
        setMode(1);

        unsigned long currentTimeInSeconds = getCurrentTimeInSeconds();
        unsigned long currentHours         = getCurrentHours(currentTimeInSeconds);
        unsigned long currentMinutes       = getCurrentMinutes(currentTimeInSeconds);
        unsigned long currentSeconds       = getCurrentSeconds(currentTimeInSeconds);

        baseHours   = currentHours;
        baseMinutes = currentMinutes;
        baseSeconds = currentSeconds;
      }

      break;

    case 1:
      // if short press, increment baseHours
      // if long press, switch to mode 2
      if (isShortPress) {
        baseHours += 1;

        if ((is24Hour && baseHours > 23) ||
            (!is24Hour && baseHours > 11)) {
          baseHours = 0;
        }
      }

      if (isLongPress) {
        setMode(2);
      }

      break;

    case 2:
      // if short press, increment baseMinutes
      // if long press, switch to mode 3
      if (isShortPress) {
        baseMinutes += 1;

        if (baseMinutes > 59) {
          baseMinutes = 0;
        }
      }

      if (isLongPress) {
        setMode(3);
      }

      break;

    case 3:
      // if short press, increment baseSeconds
      // if long press, switch to mode 0
      if (isShortPress) {
        baseSeconds += 1;

        if (baseSeconds > 59) {
          baseSeconds = 0;
        }
      }

      if (isLongPress) {
        setMode(0);

        timeThatTimeWasSetByUser = millis();
      }

      break;
  }

  if (isShortPress || isLongPress) {
    // reset times after each button press
    buttonDownTime = 0;
    buttonUpTime = 0;
  }

  previousButtonState = newButtonState;
}



// ====================
//   Helper Functions
// ====================

void setMode(int mode) {
  currentMode = mode;

  timeThatModeWasLastChanged = millis();
}

String getBinaryString(int number) {
  return String(number, BIN);
}

unsigned long getCurrentTimeInSeconds() {
  unsigned long secondsSinceTimeWasLastSet = (millis() - timeThatTimeWasSetByUser) / 1000;

  unsigned long currentTimeInSeconds = (baseHours   * 60 * 60) +
                                       (baseMinutes * 60)      +
                                        baseSeconds            +
                                       secondsSinceTimeWasLastSet;

  return currentTimeInSeconds;
}

unsigned long getCurrentHours(unsigned long currentTimeInSeconds) {
  return (currentTimeInSeconds / 60 / 60) % 24;
}

unsigned long getCurrentMinutes(unsigned long currentTimeInSeconds) {
  return (currentTimeInSeconds / 60) % 60;
}

unsigned long getCurrentSeconds(unsigned long currentTimeInSeconds) {
  return currentTimeInSeconds % 60;
}
