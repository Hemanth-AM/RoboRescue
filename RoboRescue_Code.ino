#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial BTSerial(0, 1);
Servo myServo;  

#define enA 6  
#define in1 9  
#define in2 8   
#define in3 7  
#define in4 10  
#define enB 5   

#define WATER_PUMP A3  

#define ir_R A0  
#define ir_F A1  
#define ir_L A2  

int Speed = 100;  
unsigned long lastCommandTime = 0; 
const int timeout = 300; 
bool autoMode = true;  
bool pumpState = false; 
bool btConnected = false; 

void setup() {
    Serial.begin(9600);  
    BTSerial.begin(9600);  

    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(WATER_PUMP, OUTPUT);

    pinMode(ir_R, INPUT);
    pinMode(ir_F, INPUT);
    pinMode(ir_L, INPUT);

    digitalWrite(WATER_PUMP, HIGH);  
    pumpState = false;

    analogWrite(enA, Speed); 
    analogWrite(enB, Speed); 

    myServo.attach(11);
    myServo.write(90);

    Serial.println("🔥 Automatic Mode ON (Default)");
}

void loop() {
    if (BTSerial.available() && !btConnected) {
        btConnected = true;
        Serial.println("Hemanth Robot 🤖 is activated");
    }

    if (BTSerial.available()) {
        char command = BTSerial.read();
        lastCommandTime = millis();

        if (command == 'A') {
            autoMode = true;
            Serial.println("Automatic Mode ON");
        } else if (command == 'M') {
            autoMode = false;
            Serial.println("Manual Mode ON");
            Stop();
        } else if (!autoMode) {
            if (command == 'F') forward();
            else if (command == 'B') backward();
            else if (command == 'L') turnLeft();
            else if (command == 'R') turnRight();
            else if (command == 'S') Stop();
            else if (command == 'X') turnOnPump();
            else if (command == 'T') turnOffPump();
            else if (command == 'C') rotateServo();
        }
    }

    if (autoMode) {
        automaticFireDetection();
    }

    if (!autoMode && millis() - lastCommandTime > timeout) {
        Stop();
    }
}

// **🔥 Automatic Mode**
void automaticFireDetection() {
    int rightFire = digitalRead(ir_R);
    int frontFire = digitalRead(ir_F);
    int leftFire = digitalRead(ir_L);

    int fireCount = (!rightFire) + (!frontFire) + (!leftFire);

    if (fireCount >= 2) {
        if (!pumpState) { 
            Serial.println("🔥🔥 Multiple Fires Detected! Stopping & Activating Pump!");
            pumpState = true;
        }
        Stop();
        adjustServo(leftFire, frontFire, rightFire);
        turnOnPump();  
    } else {
        if (pumpState) { 
            pumpState = false;
            turnOffPump();
        }

        if (frontFire == LOW) {
            Serial.println("🔥 Fire Detected in Front");
            myServo.write(90);
            forward();
        } else if (leftFire == LOW) {
            Serial.println("🔥 Fire Detected on Left");
            myServo.write(180);
            turnLeft();
        } else if (rightFire == LOW) {
            Serial.println("🔥 Fire Detected on Right");
            myServo.write(0);
            turnRight();
        } else {
            Stop();
        }
    }
}

// **📍 Adjust Servo & Rotate Continuously on 3 Fires**
void adjustServo(int leftFire, int frontFire, int rightFire) {
    if (leftFire == LOW && frontFire == LOW && rightFire == LOW) {
        Serial.println("🔥🔥🔥 Fire Detected on All Sensors - Rotating Servo Continuously!");

        while (digitalRead(ir_R) == LOW && digitalRead(ir_F) == LOW && digitalRead(ir_L) == LOW) {
            sweepServo();
        }

        myServo.write(90);
    } else if (leftFire == LOW && frontFire == LOW) {
        Serial.println("🔥 Fire Detected on Left & Front - Rotating Servo to 135°");
        myServo.write(135);
    } else if (rightFire == LOW && frontFire == LOW) {
        Serial.println("🔥 Fire Detected on Right & Front - Rotating Servo to 45°");
        myServo.write(45);
    } else if (leftFire == LOW && rightFire == LOW) {
        Serial.println("🔥 Fire Detected on Left & Right - Rotating Servo Between 0° & 180°");
        sweepServo();
    } else {
        myServo.write(90);
    }
}

// **🔄 Sweep Servo from 0° to 180° Continuously**
void sweepServo() {
    for (int pos = 0; pos <= 180; pos += 5) {  
        myServo.write(pos);
        delay(30);
    }
    for (int pos = 180; pos >= 0; pos -= 5) {  
        myServo.write(pos);
        delay(30);
    }
}

// **🔁 Move forward**
void forward() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// **🔁 Move backward**
void backward() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

// **🔁 Turn right**
void turnRight() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// **🔁 Turn left**
void turnLeft() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

// **🛑 Stop motors**
void Stop() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

// **💦 Turn ON the water pump**
void turnOnPump() {
    digitalWrite(WATER_PUMP, LOW);
}

// **💦 Turn OFF the water pump**
void turnOffPump() {
    digitalWrite(WATER_PUMP, HIGH);
}

// **🔄 Rotate Servo 3 Times in Manual Mode**
void rotateServo() {
    for (int i = 0; i < 3; i++) {
        myServo.write(140);
        delay(500);
        myServo.write(0);
        delay(500);
    }
}
