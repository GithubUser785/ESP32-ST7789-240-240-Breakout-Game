/* Developer's signature:
0x7531
glhf :)

-16/9/2024
-17/9/2024
-18/9/2024
-19/9/2024
-20/9/2024
-21/9/2024
*/

#include <TFT_eSPI.h>  // Graphics and font library for ST7789 driver chip
#include <SPI.h>
// ST7789 screen width and height
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
// Create an instance of the TFT_eSPI library
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);  // Invoke library, pins defined in User_Setup.h

// Pin definitions for the joystick and buzzer
#define JOY_VRX 34  // Joystick X-axis
#define JOY_VRY 35  // Joystick Y-axis
#define JOY_SW 12   // Joystick button
#define BUZZER 14   // Buzzer PWM pin

// Game variables
int paddleX, paddleY, paddleWidth = 40, paddleHeight = 10;
int ballX, ballY, ballSpeedX = 2, ballSpeedY = 2, ballRadius = 5;
int screenWidth = 240, screenHeight = 240;
bool gameOver = false;
int score = 0;                // Initialize score
int level = 1;                // Initialize level
bool gameOverStatus = false;  // To track win/loss
bool powerUpVisible = false;

// Tick system
const int totalTicks = 500;
int currentTick = 0;

// Global variables to store previous positions
int lastPaddleX = 0;
int lastBallX = 0;
int lastBallY = 0;

// Brick variables
const int brickWidth = 30;
const int brickHeight = 10;
const int numRows = 5;
const int numCols = 8;
int bricks[numRows][numCols];  // 0 = not hit, 1 = hit

// Power-up variables
bool powerUpActive = false;                  // Track if the power-up is active
unsigned long powerUpStartTime;              // Track when the power-up was activated
const unsigned long powerUpDuration = 5000;  // Power-up duration in milliseconds


//For random speeds
float getRandomFloat(float min, float max) {
  // Generate a random integer between 0 and 10000
  long randomInt = random(0, 10001);
  // Scale it to the specified range
  return min + (randomInt / 10000.0) * (max - min);
}


//Start screen
void showStartScreen() {
  tft.fillScreen(TFT_BLACK);  // Clear the screen
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(false);  // Disable text wrapping

  // Start the text at x = 0
  int yOffset = 50;           // Starting Y position for the first line
  tft.setCursor(0, yOffset);  // Set X to 0
  tft.print("ESP32 Breakout Game");

  yOffset += 30;              // Move down for the next line
  tft.setCursor(0, yOffset);  // Set X to 0
  tft.print("By me (not you)!");

  yOffset += 30;              // Move down for the next line
  tft.setCursor(0, yOffset);  // Set X to 0
  tft.print("Push joystick button");

  yOffset += 30;              // Move down for the last line
  tft.setCursor(0, yOffset);  // Set X to 0
  tft.print("to begin.");

  // Wait for joystick button press
  while (digitalRead(JOY_SW) == HIGH) {
    delay(10);  // Busy wait until joystick button is pressed
  }

  // Clear screen after the button is pressed
  tft.fillScreen(TFT_BLACK);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  randomSeed(analogRead(0));  // Use a floating analog pin to seed randomness
  // Joystick and buzzer pin setups
  pinMode(JOY_VRX, INPUT);
  pinMode(JOY_VRY, INPUT);
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // Show the start screen
  showStartScreen();

  // Initialize paddle and ball positions
  paddleX = screenWidth / 2 - paddleWidth / 2;
  paddleY = screenHeight - 20;
  int randomX = random(-60, 60);
  int randomY = random(-15, 15);
  Serial.print("Random X: ");
  Serial.print(randomX);
  Serial.print(", Random Y: ");
  Serial.println(randomY);
  ballX = screenWidth / 2 + randomX;
  ballY = screenHeight / 2 + randomY;

  ballSpeedX *= getRandomFloat(1.01, 1.15);
  ballSpeedY *= getRandomFloat(1.01, 1.15);
  resetBricks();  // Initialize brick positions
  lastPaddleX = paddleX;
  lastBallX = ballX;
  lastBallY = ballY;

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  drawBricks();   // Draw bricks initially
  updateScore();  // Draw initial score

  // Debugging output
  Serial.println("Setup complete. Game initialized.");
}

void loop() {
  if (!gameOver) {
    currentTick++;

    // Physics every tick
    runPhysics();

    // Joystick every 2 ticks
    if (currentTick % 2 == 0) {
      runJoystick();
    }

    if (currentTick % 5 == 0) {
      drawBricks();
    }

    // Regen text every 100 ticks (if not game over)
    if (currentTick % 25 == 0 && !gameOver) {
      updateScore();
    }

    if (currentTick >= totalTicks) {
      currentTick = 0;  // Reset tick count after 500
    }

    // Check and update power-up status
    if (powerUpActive && millis() - powerUpStartTime >= powerUpDuration) {
      deactivatePowerUp();
    }

    delay(40);  // Control the speed of the game
  } else {
    // Display end screen
    delay(5000);  // 5 seconds delay before restarting
    // Restart the game if the joystick button is pressed
    if (digitalRead(JOY_SW) == LOW) {  // Button pressed when LOW
      resetGame(gameOverStatus);
    }
  }
}

void runPhysics() {
  // Save previous ball position
  lastBallX = ballX;
  lastBallY = ballY;

  // Update ball position
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Ball collision with walls
  if (ballX - ballRadius <= 0 || ballX + ballRadius >= screenWidth) {
    ballSpeedX *= -1;  // Reverse horizontal direction
    ballX = constrain(ballX, ballRadius, screenWidth - ballRadius);
    Serial.println("Ball hit horizontal wall");
  }

  if (ballY - ballRadius <= 0) {
    ballSpeedY *= -1;  // Reverse direction for top wall
    ballY = ballRadius;
    Serial.println("Ball hit top wall");
  }

  // Ball collision with paddle
  if (ballY + ballRadius >= paddleY && ballY + ballRadius <= paddleY + paddleHeight && ballX + ballRadius >= paddleX && ballX - ballRadius <= paddleX + paddleWidth) {
    ballSpeedY *= -1;              // Reverse direction for paddle
    ballY = paddleY - ballRadius;  // Prevent clipping
    tone(BUZZER, 262, 100);        // Play sound when ball hits the paddle
    Serial.println("Ball hit paddle");
  }

  // Ball collision with bricks
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++) {
      if (bricks[r][c] == 0) {  // Only check unhit bricks
        int brickX = c * (brickWidth + 5);
        int brickY = r * (brickHeight + 5);
        if (ballX + ballRadius > brickX && ballX - ballRadius < brickX + brickWidth && ballY + ballRadius > brickY && ballY - ballRadius < brickY + brickHeight) {
          
          // Check if ball is moving upwards before reversing vertical speed
          if (ballSpeedY < 0) {
            ballSpeedY *= -1;
          }

          bricks[r][c] = 1;                                                  // Mark brick as hit
          tft.fillRect(brickX, brickY, brickWidth, brickHeight, TFT_BLACK);  // Remove the brick
          tone(BUZZER, 262, 100);                                            // Play sound
          score += 10;                                                       // Update score
          updateScore();                                                     // Redraw score
          Serial.print("Brick hit at (");
          Serial.print(brickX);
          Serial.print(", ");
          Serial.print(brickY);
          Serial.println(")");

          return;  // Exit after first hit
        }
      }
    }
  }

  // Game over if the ball goes off the screen
  if (ballY > screenHeight) {
    endLevel(false);  // Pass false for loss
  }

  // Clear the previous ball position
  tft.fillCircle(lastBallX, lastBallY, ballRadius + 3, TFT_BLACK);
  // Clear the previous paddle position
  clearPaddle(lastPaddleX, paddleY, paddleWidth);
  // Draw the new paddle position
  tft.fillRect(paddleX, paddleY, powerUpActive ? paddleWidth * 2 : paddleWidth, paddleHeight, TFT_WHITE);
  // Draw the new ball position
  tft.fillCircle(ballX, ballY, ballRadius, TFT_RED);

  lastPaddleX = paddleX;  // Update last paddle position
}





void runJoystick() {
  // Read joystick input and map to screen width
  int joyX = analogRead(JOY_VRX);
  int joyY = analogRead(JOY_VRY);
  // Map joystick values (0-4095) to the screen width (0-240)
  paddleX = map(joyX, 0, 4095, 0, screenWidth - paddleWidth);
  // Clamp paddle position
  paddleX = constrain(paddleX, 0, screenWidth - paddleWidth);
  Serial.print("Joystick X: ");
  Serial.print(joyX);
  Serial.print(" Joystick Y: ");
  Serial.println(joyY);
}

void clearPaddle(int x, int y, int width) {
  tft.fillRect(x, y, width, paddleHeight, TFT_BLACK);
}

void endLevel(bool won) {
  gameOver = true;
  gameOverStatus = won;

  // Display message for the player
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(50, screenHeight / 2 - 10);
  tft.setTextSize(2);
  tft.print(won ? "You Won!" : "Game Over");
  tft.setTextSize(1);
  tft.setCursor(50, screenHeight / 2 + 10);
  tft.print("Score: ");
  tft.print(score);
  tft.print(" Level: ");
  tft.print(level);

  Serial.print("Game Over. ");
  Serial.print(won ? "Player won. " : "Player lost. ");
  Serial.print("Score: ");
  Serial.print(score);
  Serial.print(" Level: ");
  Serial.println(level);

  // Wait for joystick button press to reset the game
  while (digitalRead(JOY_SW) == HIGH) {
    // Busy wait until joystick button is pressed
    delay(10);
  }

  // Call resetGame with the gameOverStatus (win/loss) passed
  resetGame(gameOverStatus);
}

void resetGame(bool winStatus) {
  gameOver = false;

  // Clear the screen to remove any leftover text from the previous game
  tft.fillScreen(TFT_BLACK);

  Serial.print("Resetting game. Level: ");
  Serial.println(level);
  score = 0;
  // Reset paddle and ball positions
  paddleX = screenWidth / 2 - paddleWidth / 2;
  paddleY = screenHeight - 20;
  int randomX = random(-60, 60);
  int randomY = random(-15, 15);
  Serial.print("Random X: ");
  Serial.print(randomX);
  Serial.print(", Random Y: ");
  Serial.println(randomY);
  ballX = screenWidth / 2 + randomX;
  ballY = screenHeight / 2 + randomY;


  // Randomise ball speed and increase level on win
  ballSpeedX = 2;
  ballSpeedY = 2;
  if (winStatus) {
    if (ballSpeedX > 0.75) { ballSpeedX *= getRandomFloat(0.95, 1.15); }
    if (ballSpeedY > 0.75) { ballSpeedY *= getRandomFloat(0.95, 1.15); }
    level++;  // Increment level only if the player won
  }

  // Reset bricks and redraw
  resetBricks();
  drawBricks();

  // Update the score and level display
  updateScore();

  Serial.println("Game reset complete.");
}


void updateScore() {
  tft.fillRect(0, paddleY -15, screenWidth, 20, TFT_BLACK);  // Clear previous score display
  tft.setCursor(0, paddleY - 15);                  // Set cursor to 10 pixels above the paddle
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.print("Score: ");
  tft.print(score);
  tft.setCursor(screenWidth - 60, paddleY - 15);  // Adjust level text position
  tft.print("Level: ");
  tft.print(level);
}


void resetBricks() {
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++) {
      bricks[r][c] = 0;  // Initialize bricks as not hit
    }
  }
  Serial.println("Bricks reset.");
}

void drawBricks() {
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++) {
      if (bricks[r][c] == 0) {
        int brickX = c * (brickWidth + 5);
        int brickY = r * (brickHeight + 5);
        tft.fillRect(brickX, brickY, brickWidth, brickHeight, TFT_BLUE);  // Draw bricks
      }
    }
  }
  Serial.println("Bricks drawn.");
}



void activatePowerUp() {
  powerUpActive = true;
  powerUpStartTime = millis();
  Serial.println("Power-up activated!");
}

void deactivatePowerUp() {
  powerUpActive = false;
  Serial.println("Power-up deactivated.");
}
