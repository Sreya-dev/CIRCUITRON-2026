# 🔐 Arduino Button Password Lock System

**Platform:** Tinkercad Simulation · **Microcontroller:** Arduino Uno

🔗 **Tinkercad Simulation:** [View Project](https://www.tinkercad.com/things/4DgVgO8YYR7-buttonpassword-lock-system/editel?sharecode=xhDpbzL1JIbb_HxtRfiyxc20ovMYoZXo3xJBn7er_w)

---

## 📌 Aim

To design and implement a hardware-based security lock system using an Arduino Uno that authenticates users via a specific 3-button sequence. The system provides visual feedback through LEDs and audio feedback through a piezo buzzer.

---

## 📖 Project Overview

The system acts as a digital combination lock that continuously monitors three tactile pushbuttons and records the sequence of inputs.

- **Authentication Logic** — The system tracks user presses using an input array. Once exactly three inputs are captured, it validates the entries against a hardcoded sequence (Button 2 → Button 1 → Button 3).
- **Debouncing Mechanism** — Software-based debouncing tracks state histories using array indexes mapped to hardware pin numbers, eliminating electrical noise and double-triggering.
- **Feedback Mechanism**
  - **Success:** Triggers the green LED, outputs a high-frequency confirmation tone (1200 Hz), and prints `ACCESS GRANTED` to the console.
  - **Failure:** Triggers the red LED, outputs a low-frequency alert tone (300 Hz), and prints `ACCESS DENIED` to the console.
- **System Reset** — Instantly resets the index tracker and prints status lines to clear memory for subsequent attempts.

---

## 🧩 Component Pin Mapping

| Component     | Arduino Pin | Configuration Type         |
| ------------- | ----------- | -------------------------- |
| Push Button 1 | Pin 2       | Input (External pull-down) |
| Push Button 2 | Pin 3       | Input (External pull-down) |
| Push Button 3 | Pin 4       | Input (External pull-down) |
| Piezo Buzzer  | Pin 8       | Output                     |
| Green LED     | Pin 9       | Output                     |
| Red LED       | Pin 10      | Output                     |

---

## 💻 Source Code

```cpp
// Pin Definitions
const int btn1Pin = 2;
const int btn2Pin = 3;
const int btn3Pin = 4;
const int buzzerPin = 8;
const int greenLedPin = 9;
const int redLedPin = 10;

// Password Configuration (Button 2 -> Button 1 -> Button 3)
const int password[3] = {2, 1, 3};
int inputSequence[3];
int currentIndex = 0;

// Debounce Tracking Arrays
const unsigned long debounceDelay = 50;
int buttonState[11] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastButtonState[11] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);

  pinMode(btn1Pin, INPUT);
  pinMode(btn2Pin, INPUT);
  pinMode(btn3Pin, INPUT);

  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  noTone(buzzerPin);

  Serial.println("System Ready. Enter Password...");
}

void loop() {
  checkButton(btn1Pin, 1);
  checkButton(btn2Pin, 2);
  checkButton(btn3Pin, 3);
}

void checkButton(int pin, int buttonNumber) {
  int reading = digitalRead(pin);

  if (reading != lastButtonState[pin]) {
    lastDebounceTime[pin] = millis();
  }

  if ((millis() - lastDebounceTime[pin]) > debounceDelay) {
    if (reading != buttonState[pin]) {
      buttonState[pin] = reading;

      if (buttonState[pin] == HIGH) {
        inputSequence[currentIndex] = buttonNumber;
        currentIndex++;

        Serial.print("Button ");
        Serial.print(buttonNumber);
        Serial.println(" registered.");

        tone(buzzerPin, 800, 50); // Short click sound for feedback

        if (currentIndex == 3) {
          verifyPassword();
        }
      }
    }
  }
  lastButtonState[pin] = reading;
}

void verifyPassword() {
  bool correct = true;
  for (int i = 0; i < 3; i++) {
    if (inputSequence[i] != password[i]) {
      correct = false;
      break;
    }
  }

  if (correct) {
    Serial.println("ACCESS GRANTED");
    digitalWrite(greenLedPin, HIGH);
    tone(buzzerPin, 1200, 500);
    delay(1500);
    digitalWrite(greenLedPin, LOW);
  } else {
    Serial.println("ACCESS DENIED");
    digitalWrite(redLedPin, HIGH);
    tone(buzzerPin, 300, 500);
    delay(1500);
    digitalWrite(redLedPin, LOW);
  }
  resetSystem();
}

void resetSystem() {
  currentIndex = 0;
  Serial.println("-------------------------");
  Serial.println("System Ready. Enter Password...");
}
```

---

## 🖥️ System Output (Serial Monitor Log)

```
System Ready. Enter Password...
Button 2 registered.
Button 1 registered.
Button 3 registered.
ACCESS GRANTED
-------------------------
System Ready. Enter Password...
Button 1 registered.
Button 2 registered.
Button 3 registered.
ACCESS DENIED
-------------------------
System Ready. Enter Password...
```

---

## 🏁 Conclusion

The project successfully demonstrates the implementation of a secure, state-driven digital lock using an Arduino Uno. By implementing dynamic software debouncing routines, the system completely filters out mechanical contact noise, ensuring that button presses are logged accurately and reliably. The integration of instant visual and audio feedback via LEDs and the piezo buzzer creates an intuitive user interface. This structural logic serves as a robust baseline for developing more complex access control systems, such as electronic safes, smart home entry points, or multi-tiered hardware security keypads.

---

**Author:** Sreya Vinod
