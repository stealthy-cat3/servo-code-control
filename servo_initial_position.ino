#include <ESP32Servo.h>

Servo elbow;  // GPIO 12
Servo wrist;  // GPIO 13
Servo base;   // GPIO 14
Servo claw;   // GPIO 27

String inputString = "";

struct Movement {
  String target;
  int strength;
  int waitTime;
};

void setup() {
  Serial.begin(9600);
  elbow.attach(12);
  wrist.attach(13);
  base.attach(14);
  claw.attach(27);
}

void initialPosition() {
  Serial.println("Going to initial position");

  elbow.write(110);
  wrist.write(90);
  base.write(90);
  claw.write(90);
  delay(3000);

  wrist.write(96);
  elbow.write(96);
  base.write(96);
  claw.write(96);
  delay(500);

  Serial.println("Now in initial position");
}

void initialPos() {
  Serial.println("Starting initialPos sequence");

  elbow.write(strengthToAngle(-20));
  wrist.write(strengthToAngle(-30));
  delay(1500);

  elbow.write(96);
  wrist.write(96);
  delay(1000);

  elbow.write(strengthToAngle(50));
    delay(780);
  elbow.write(96);
  delay(1000);

  wrist.write(strengthToAngle(50));
  delay(560);
  wrist.write(96);
  delay(500);

  Serial.println("Finished initialPos sequence");
}

void pickAndDrop() {
  Serial.println("Starting pickAndDrop sequence");

  claw.write(strengthToAngle(-30));
  delay(300);
  claw.write(96);
  delay(500);

  wrist.write(strengthToAngle(10));
  delay(100);
  wrist.write(96);
  delay(500);

  elbow.write(strengthToAngle(-20));
  delay(300);
  elbow.write(96);
  delay(500);

  claw.write(strengthToAngle(30));
  delay(400);
  claw.write(96);
  delay(500);

  initialPos();  // Call the initialPos sequence

  base.write(strengthToAngle(20));
  delay(600);
  base.write(96);
  delay(500);

  claw.write(strengthToAngle(-30));
  delay(300);
  claw.write(96);
  delay(500);

  base.write(strengthToAngle(-20));
  delay(600);
  base.write(96);
  delay(500);

claw.write(strengthToAngle(30));
  delay(400);
  claw.write(96);
  delay(500);
  
  Serial.println("Finished pickAndDrop sequence");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputString.trim();
      processInput(inputString);
      inputString = "";
    } else {
      inputString += c;
    }
  }
}

int strengthToAngle(int strength) {
  strength = constrain(strength, -100, 100);
  if (strength == 0) return 96;
  return (strength > 0) ? map(strength, 0, 100, 96, 0) : map(strength, 0, -100, 96, 180);
}

void processInput(String cmd) {
  if (cmd == "1") {
    initialPosition();
    return;
  } else if (cmd == "initialPos") {
    initialPos();
    return;
  } else if (cmd == "pickAndDrop") {
    pickAndDrop();
    return;
  }

  const int MAX_MOVES = 10;
  Movement moves[MAX_MOVES];
  int moveCount = 0;

  int start = 0;
  while (start < cmd.length() && moveCount < MAX_MOVES) {
    int end = cmd.indexOf(',', start);
    if (end == -1) end = cmd.length();
    String segment = cmd.substring(start, end);

    int firstColon = segment.indexOf(':');
    int secondColon = segment.indexOf(':', firstColon + 1);
    if (firstColon != -1 && secondColon != -1) {
      moves[moveCount].target = segment.substring(0, firstColon);
      moves[moveCount].strength = segment.substring(firstColon + 1, secondColon).toInt();
      moves[moveCount].waitTime = segment.substring(secondColon + 1).toInt();
      moveCount++;
    }

    start = end + 1;
  }

  int maxWait = 0;
  for (int i = 0; i < moveCount; i++) {
    int angle = strengthToAngle(moves[i].strength);
    Servo* servo = nullptr;

    if (moves[i].target == "elbow") servo = &elbow;
    else if (moves[i].target == "wrist") servo = &wrist;
    else if (moves[i].target == "base") servo = &base;
    else if (moves[i].target == "claw") servo = &claw;

    if (servo != nullptr) {
      Serial.println("Moving " + moves[i].target + " → strength: " + String(moves[i].strength) + ", angle: " + String(angle));
      servo->write(angle);
      if (moves[i].waitTime > maxWait) maxWait = moves[i].waitTime;
    } else {
      Serial.println("Unknown target: " + moves[i].target);
    }
  }

  delay(maxWait);

  for (int i = 0; i < moveCount; i++) {
    Servo* servo = nullptr;
    if (moves[i].target == "elbow") servo = &elbow;
    else if (moves[i].target == "wrist") servo = &wrist;
    else if (moves[i].target == "base") servo = &base;
    else if (moves[i].target == "claw") servo = &claw;

    if (servo != nullptr) {
      servo->write(96);
    }
  }

  delay(500);
}
