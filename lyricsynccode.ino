#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <string.h>

// Auto-detect I2C LCD
hd44780_I2Cexp lcd;

const int startButton = 2;
const unsigned long debounceDelay = 60;

// Custom characters
byte heart[8] = {
0b00000, 0b01010, 0b11111, 0b11111,
0b01110, 0b00100, 0b00000, 0b00000
};

byte note[8] = {
0b00100, 0b00110, 0b00101, 0b00101,
0b00100, 0b11100, 0b11100, 0b00000
};

// Lyric structure
struct LyricFrame {
unsigned long startTime;
const char* line1;
const char* line2;
bool heartIcon;
bool noteIcon;
};

// Lyrics timing
LyricFrame lyrics[] = {
{1000,  "Please tell me",   "not to go",        true,  true},
{7000,  "We've been here",  "long before",      false, false},
{14000, "I live under",     "your eyelids",     true,  false},
{20000, "I'll always",      "be yours",         true,  false},
{27000, "I'll lay on your", "rooftop",          true,  false},
{30000, "In the freezing",  "cold",             false, false},
{33000, "And I'll watch",   "the sunset",       false, false},
{37000, "Wearing all",      "your clothes",     true,  false},
{40000, "I can feel you",   "with me",          true,  false},
{43000, "Like I did",       "before",           false, false},
{46000, "Like when I sang", "you a love song",  false, false},
{50000, "By Norah",         "Jones",            true,  true},
{56000, "",                 "",                 false, false}
};

const int totalLyrics = sizeof(lyrics) / sizeof(lyrics[0]);

// State variables
bool started = false;
int currentIndex = -1;
unsigned long songStart = 0;

int lastButtonReading = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// Clear row
void clearRow(int row) {
lcd.setCursor(0, row);
lcd.print("                    ");
}

// Print padded text
void printPadded(const char* text, int row, int col = 0) {
lcd.setCursor(col, row);
lcd.print(text);

int len = strlen(text);
for (int i = col + len; i < 20; i++) {
lcd.print(" ");
}
}

// Show frame
void showFrame(const LyricFrame &f) {
clearRow(0);
clearRow(1);

if (f.noteIcon) {
lcd.setCursor(0, 0);
lcd.write(1);
printPadded(f.line1, 0, 1);
} else {
printPadded(f.line1, 0, 0);
}

printPadded(f.line2, 1, 0);

if (f.heartIcon) {
int len = strlen(f.line2);
if (len > 18) len = 18;
lcd.setCursor(len, 1);
lcd.write(0);
}
}

// Button debounce
bool startButtonPressed() {
int reading = digitalRead(startButton);

if (reading != lastButtonReading) {
lastDebounceTime = millis();
}

if ((millis() - lastDebounceTime) > debounceDelay) {
if (reading != stableButtonState) {
stableButtonState = reading;
if (stableButtonState == LOW) {
lastButtonReading = reading;
return true;
}
}
}

lastButtonReading = reading;
return false;
}

// Screens
void showWaitingScreen() {
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Press button when");
lcd.setCursor(0, 1);
lcd.print("song starts");
lcd.setCursor(0, 2);
lcd.print("Synced LCD lyrics");
lcd.setCursor(0, 3);
lcd.print("Ready...");
}

void showStartingScreen() {
lcd.clear();
lcd.setCursor(0, 1);
lcd.print("Starting...");
delay(500);
lcd.clear();
}

void showFinishedScreen() {
lcd.clear();
lcd.setCursor(0, 1);
lcd.print("Part complete");
lcd.setCursor(0, 2);
lcd.print("Press to replay");
}

void setup() {
pinMode(startButton, INPUT_PULLUP);

// Initialize LCD (auto detect address)
int status = lcd.begin(20, 4);
if (status) {
// If LCD fails, stop here
while (1);
}

lcd.backlight();

lcd.createChar(0, heart);
lcd.createChar(1, note);

showWaitingScreen();
}

void loop() {
if (!started) {
if (startButtonPressed()) {
showStartingScreen();
started = true;
songStart = millis();
currentIndex = -1;
}
return;
}

unsigned long elapsed = millis() - songStart;
int newIndex = -1;

for (int i = totalLyrics - 1; i >= 0; i--) {
if (elapsed >= lyrics[i].startTime) {
newIndex = i;
break;
}
}

if (newIndex != currentIndex) {
currentIndex = newIndex;


if (currentIndex >= 0 && currentIndex < totalLyrics - 1) {
  showFrame(lyrics[currentIndex]);
} else if (currentIndex == totalLyrics - 1) {
  started = false;
  showFinishedScreen();
}


}
}
