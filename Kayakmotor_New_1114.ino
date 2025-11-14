// Pins
const uint8_t enA = 9;          // PWM enable for motor A
const uint8_t in1 = 10;         // Direction
const uint8_t in2 = 11;         // Direction
const uint8_t potPin = A0;      // Potentiometer
const uint8_t buttonDir = 7;    // Toggle direction (active LOW, pullup)
const uint8_t buttonEStop = 2;  // Emergency stop (active LOW, pullup)
const uint8_t ledFwd = 3;       // Forward indicator LED
const uint8_t ledRev = 4;       // Reverse indicator LED

// State
bool motorDirection = true;     // true=forward, false=reverse
bool lastDirBtn = HIGH;
unsigned long lastDebounceMs = 0;
const unsigned long debounceDelayMs = 50;

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(buttonDir, INPUT_PULLUP);
  pinMode(buttonEStop, INPUT_PULLUP);

  pinMode(ledFwd, OUTPUT);
  pinMode(ledRev, OUTPUT);

  Serial.begin(9600);

  // Motor off initially
  analogWrite(enA, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(ledFwd, LOW);
  digitalWrite(ledRev, LOW);
}

void loop() {
  // --- Emergency stop ---
  if (digitalRead(buttonEStop) == LOW) {
    analogWrite(enA, 0);
    // coast (both LOW) or brake (both HIGH). Choose one:
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);

    Serial.println("EMERGENCY STOP: Motor halted. Power-cycle to reset.");
    // Blink both LEDs forever
    while (true) {
      digitalWrite(ledFwd, HIGH);
      digitalWrite(ledRev, HIGH);
      delay(400);
      digitalWrite(ledFwd, LOW);
      digitalWrite(ledRev, LOW);
      delay(400);
    }
  }

  // --- Read controls ---
  int raw = analogRead(potPin);                 // 0..1023
  int pwm = map(raw, 0, 1023, 0, 255);         // 0..255
  pwm = constrain(pwm, 0, 255);

  // Optional deadband/minimum to overcome stall friction
  const int minPWM = 35;                        // tweak for your motor/driver
  if (pwm > 0 && pwm < minPWM) pwm = minPWM;

  // --- Direction toggle with debounce on state change ---
  int dirBtn = digitalRead(buttonDir);
  if (dirBtn != lastDirBtn) {
    lastDebounceMs = millis();
    lastDirBtn = dirBtn;
  }
  if ((millis() - lastDebounceMs) > debounceDelayMs) {
    // rising->falling (button press)
    if (dirBtn == LOW) {
      // Smooth/instant stop before reversing
      analogWrite(enA, 0);
      // Choose brake or coast during pause:
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      delay(300); // short pause to let rotor stop

      motorDirection = !motorDirection;
      Serial.println(motorDirection ? "Direction: FORWARD" : "Direction: REVERSE");
    }
  }

  // --- Set direction & LEDs ---
  if (motorDirection) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(ledFwd, HIGH);
    digitalWrite(ledRev, LOW);
    if (pwm == 0) Serial.println("Coasting forward (pwm=0)");
    else { Serial.print("Forward pwm: "); Serial.println(pwm); }
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(ledFwd, LOW);
    digitalWrite(ledRev, HIGH);
    if (pwm == 0) Serial.println("Coasting reverse (pwm=0)");
    else { Serial.print("Reverse pwm: "); Serial.println(pwm); }
  }

  // --- Apply speed ---
  analogWrite(enA, pwm);
}
