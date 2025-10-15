// This Arduino sketch creates a complete interactive audio learning system.
// It manages page selection, letter playback, and a new "Question Mode".

// =========================================
// INCLUDES & LIBRARIES
// =========================================
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// =========================================
// PIN DEFINITIONS
// =========================================
// Using SoftwareSerial for communication with the DFPlayer Mini.
#define DFPLAYER_RX_PIN 10 // Connects to DFPlayer's TX
#define DFPLAYER_TX_PIN 11 // Connects to DFPlayer's RX via voltage divider

// Control buttons
#define QUESTION_BUTTON_PIN 2 // Button to start a random letter question or exit quiz
#define PAGE1_BUTTON_PIN 3 // Page 1 activation
#define PAGE2_BUTTON_PIN 4 // Page 2 activation

// Letter buttons (6 total, shared between pages)
#define LETTER_1_PIN 5     // Corresponds to A or G
#define LETTER_2_PIN 8     // Corresponds to B or H
#define LETTER_3_PIN 9     // Corresponds to C or I
#define LETTER_4_PIN 12    // Corresponds to D or J
#define LETTER_5_PIN 6     // Corresponds to E or K
#define LETTER_6_PIN 7     // Corresponds to F or L

// =========================================
// GLOBAL VARIABLES & STATE MANAGEMENT
// =========================================
SoftwareSerial mySoftwareSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

// System state variables
int currentPage = 0;            // 0: no page active, 1: Page 1, 2: Page 2
int currentVoluxme = 30;         // Initial volume set to 25

// Game state variables
bool isQuestionMode = false;    // Tracks if the system is in question mode
int correctAnswer = 0;          // Stores the audio file number of the correct answer
int lastQuestion = 0;           // Prevents the same question from being asked twice in a row

// Debouncing variables
unsigned long lastPressTime = 0;
const long debounceDelay = 200; // 200 milliseconds debounce delay

// =========================================
// SETUP FUNCTION
// =========================================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  Serial.println(F("Initializing System..."));
  
  // The system starts automatically and plays the welcome audio.
  Serial.println(F("System is ON. Initializing DFPlayer..."));
  mySoftwareSerial.begin(9600);
  delay(500); // Give DFPlayer time to boot up
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("DFPlayer failed to begin. Check connections and SD card."));
    while (true); // Halt the program if initialization fails
  }
  
  Serial.println(F("DFPlayer Mini initialized."));
  myDFPlayer.volume(currentVolume); // Set initial volume
  myDFPlayer.play(13); // Play "Welcome to Touch Learn" audio (0013.mp3)
  Serial.print(F("Current Volume: "));
  Serial.println(currentVolume);
  
  Serial.println(F("System Ready. Please select a page."));

  // Configure all button pins as INPUT_PULLUP
  pinMode(PAGE1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PAGE2_BUTTON_PIN, INPUT_PULLUP);
  pinMode(QUESTION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LETTER_1_PIN, INPUT_PULLUP);
  pinMode(LETTER_2_PIN, INPUT_PULLUP);
  pinMode(LETTER_3_PIN, INPUT_PULLUP);
  pinMode(LETTER_4_PIN, INPUT_PULLUP);
  pinMode(LETTER_5_PIN, INPUT_PULLUP);
  pinMode(LETTER_6_PIN, INPUT_PULLUP);

  // Initialize the random number generator
  randomSeed(analogRead(A7));
}

// =========================================
// LOOP FUNCTION
// =========================================
void loop() {
  handlePageButtons();
  
  if (currentPage != 0) {
    if (isQuestionMode) {
      handleQuizButtons();
    } else {
      handleLetterButtons();
    }
  }
  handleQuestionButton(); // Check for the first press to start the quiz
}

// =========================================
// HELPER FUNCTIONS
// =========================================

void askNewQuestion() {
  int newQuestion;
  if (currentPage == 1) {
    do {
      newQuestion = random(1, 7);
    } while (newQuestion == lastQuestion);
  } else { // currentPage == 2
    do {
      newQuestion = random(7, 13);
    } while (newQuestion == lastQuestion);
  }
  
  correctAnswer = newQuestion;
  lastQuestion = newQuestion;

  Serial.print(F("Question: Find letter for file "));
  Serial.println(correctAnswer);
  
  // Page 1 question audio: files 21-26
  // Page 2 question audio: files 27-32
  myDFPlayer.play(correctAnswer + 20);
}

// =========================================
// FUNCTION IMPLEMENTATIONS
// =========================================

void handlePageButtons() {
  if (digitalRead(PAGE1_BUTTON_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
    if (currentPage != 1) {
      currentPage = 1;
      isQuestionMode = false; // Exit quiz mode if page is changed
      Serial.println(F("Page 1 Selected. Letters A-F are active."));
      myDFPlayer.play(14); // Play Page 1 activation sound (0013.mp3)
    }
    lastPressTime = millis();
  }

  if (digitalRead(PAGE2_BUTTON_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
    if (currentPage != 2) {
      currentPage = 2;
      isQuestionMode = false; // Exit quiz mode if page is changed
      Serial.println(F("Page 2 Selected. Letters G-L are active."));
      myDFPlayer.play(15); // Play Page 2 activation sound (0014.mp3)
    }
    lastPressTime = millis();
  }
}

void handleQuestionButton() {
  if (digitalRead(QUESTION_BUTTON_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
    if (isQuestionMode) {
        // Exit quiz mode
        isQuestionMode = false;
        myDFPlayer.play(19); // Play "Quiz Exited" audio (0019.mp3)
        Serial.println(F("Quiz exited."));
    } else {
        // Start a new quiz if a page is active
        if (currentPage != 0) {
            isQuestionMode = true;
            Serial.println(F("Question Mode Activated."));
            askNewQuestion(); // Ask the first question
        }
    }
    lastPressTime = millis();
  }
}

void handleQuizButtons() {
  int pressedLetter = 0;

  if (digitalRead(LETTER_1_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 1;
  else if (digitalRead(LETTER_2_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 2;
  else if (digitalRead(LETTER_3_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 3;
  else if (digitalRead(LETTER_4_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 4;
  else if (digitalRead(LETTER_5_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 5;
  else if (digitalRead(LETTER_6_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) pressedLetter = 6;
  
  if (pressedLetter != 0) {
    int audioFileNumber = pressedLetter;
    if (currentPage == 2) {
      audioFileNumber += 6;
    }

    if (audioFileNumber == correctAnswer) {
      Serial.println(F("Correct!"));
      myDFPlayer.play(16); // Play "Correct!" audio (0016.mp3)
      delay(3000); // Wait for the "Correct" audio to finish
      askNewQuestion(); // Immediately ask a new question
    } else {
      Serial.println(F("Wrong, try again."));
      myDFPlayer.play(17); // Play "Wrong, try again" audio (0017.mp3)
    }
    lastPressTime = millis();
  }
}

void handleLetterButtons() {
  // Page 1 logic (letters A-F)
  if (currentPage == 1) {
    if (digitalRead(LETTER_1_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0001.mp3 (A)")); myDFPlayer.play(1); lastPressTime = millis();
    }
    if (digitalRead(LETTER_2_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0002.mp3 (B)")); myDFPlayer.play(2); lastPressTime = millis();
    }
    if (digitalRead(LETTER_3_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0003.mp3 (C)")); myDFPlayer.play(3); lastPressTime = millis();
    }
    if (digitalRead(LETTER_4_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0004.mp3 (D)")); myDFPlayer.play(4); lastPressTime = millis();
    }
    if (digitalRead(LETTER_5_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0005.mp3 (E)")); myDFPlayer.play(5); lastPressTime = millis();
    }
    if (digitalRead(LETTER_6_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0006.mp3 (F)")); myDFPlayer.play(6); lastPressTime = millis();
    }
  } 
  // Page 2 logic (letters G-L)
  else if (currentPage == 2) {
    if (digitalRead(LETTER_1_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0007.mp3 (G)")); myDFPlayer.play(7); lastPressTime = millis();
    }
    if (digitalRead(LETTER_2_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0008.mp3 (H)")); myDFPlayer.play(8); lastPressTime = millis();
    }
    if (digitalRead(LETTER_3_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0009.mp3 (I)")); myDFPlayer.play(9); lastPressTime = millis();
    }
    if (digitalRead(LETTER_4_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0010.mp3 (J)")); myDFPlayer.play(10); lastPressTime = millis();
    }
    if (digitalRead(LETTER_5_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0011.mp3 (K)")); myDFPlayer.play(11); lastPressTime = millis();
    }
    if (digitalRead(LETTER_6_PIN) == LOW && (millis() - lastPressTime > debounceDelay)) {
      Serial.println(F("Playing 0012.mp3 (L)")); myDFPlayer.play(12); lastPressTime = millis();
    }
  }
}