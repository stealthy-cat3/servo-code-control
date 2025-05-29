#include <ESP32Servo.h>

Servo elbow;  // GPIO 12
Servo wrist;  // GPIO 13
Servo base;   // GPIO 14
Servo claw;   // GPIO 27

String inputString = "";

// Converts strength to angle using 96 as neutral
int strengthToAngle(int strength) {
  strength = constrain(strength, -100, 100);
  if (strength == 0) return 96;
  return (strength > 0) ? map(strength, 0, 100, 96, 0) : map(strength, 0, -100, 96, 180);
}

// Move using differential equation
void moveServoDiffEq(Servo& servo, int targetAngle, int durationMs, float gainK = 0.01) {
  float angle = servo.read();       // Current angle θ₀
  float u = targetAngle;            // Target angle
  float dt = 10.0;                  // ms time step
  int steps = durationMs / dt;

  for (int i = 0; i < steps; i++) {
    float dTheta = gainK * (u - angle);  // dθ/dt = k(u - θ)
    angle += dTheta * dt;                // Euler integration
    servo.write(round(angle));
    delay(dt);
  }

  servo.write(targetAngle); // Final correction
}

// Reset to neutral
void stopAll() {
  elbow.write(96);
  wrist.write(96);
  base.write(96);
  claw.write(96);
  delay(1000);
}

// Pickup sequence using diff eq based motion
void pickupSequence() {
  Serial.println("Starting pickUp sequence");

  moveServoDiffEq(claw, strengthToAngle(-30), 300);   // Close claw slightly
  stopAll();

  moveServoDiffEq(wrist, strengthToAngle(10), 100);   // Tilt wrist
  stopAll();

  moveServoDiffEq(elbow, strengthToAngle(-20), 300);  // Lower elbow
  stopAll();

  moveServoDiffEq(claw, strengthToAngle(30), 400);    // Open claw
  stopAll();

  initialPos(); // Return to initial position
  stopAll();

  moveServoDiffEq(base, strengthToAngle(20), 600);    // Rotate base
  stopAll();

  moveServoDiffEq(claw, strengthToAngle(-30), 300);   // Close claw again
  stopAll();

  moveServoDiffEq(base, strengthToAngle(-20), 700);   // Rotate base back
  stopAll();

  Serial.println("PickUp sequence complete");
}

// Custom initial position sequence
void initialPos() {
  Serial.println("Going to initial position");

  moveServoDiffEq(elbow, strengthToAngle(-20), 1500);
  moveServoDiffEq(wrist, strengthToAngle(-30), 500);
  stopAll();

  moveServoDiffEq(elbow, strengthToAngle(50), 700);
  stopAll();

  moveServoDiffEq(wrist, strengthToAngle(50), 500);
  stopAll();

  Serial.println("Now in initial position");
}

void setup() {
  Serial.begin(9600);
  elbow.attach(12);
  wrist.attach(13);
  base.attach(14);
  claw.attach(27);
  delay(500);
  Serial.println("Ready for commands");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputString.trim();
      if (inputString == "pickUp") {
        pickupSequence();
      } else if (inputString == "initialPos") {
        initialPos();
      } else {
        Serial.println("Unknown command: " + inputString);
      }
      inputString = "";
    } else {
      inputString += c;
    }
  }
}
