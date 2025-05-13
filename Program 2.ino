// Arduino Calculator - Arduino Side
// Tested on Arduino Nano
// Receives math operations over Serial, calculates and returns result

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
}

// Helper function: skips leading spaces
char* skipSpaces(char* str) {
  while (*str == ' ') str++;
  return str;
}

// Main loop: waits for input, processes it
void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Read until newline character
    input.trim();                                // Remove any leading/trailing whitespace

    int firstNumber = 0;
    int secondNumber = 0;
    char operatorChar = 0;
    bool valid = parseInput(input, firstNumber, operatorChar, secondNumber);

    if (valid) {
      float result = calculate(firstNumber, operatorChar, secondNumber);
      Serial.println(result); // Send the result back
    } else {
      Serial.println("ERROR: invalid input"); // Send error message
    }
  }
}

// Parses the input string and extracts two numbers and an operator
bool parseInput(String input, int &firstNumber, char &operatorChar, int &secondNumber) {
  char buffer[50];
  input.toCharArray(buffer, sizeof(buffer)); // Copy String into a C-style array

  char *ptr = buffer;
  ptr = skipSpaces(ptr);                     // Skip spaces before first number

  // Read first number
  char *endPtr;
  firstNumber = strtol(ptr, &endPtr, 10);     // Convert first number
  if (ptr == endPtr) return false;            // No number found

  ptr = skipSpaces(endPtr);                   // Skip spaces after first number

  // Read operator
  operatorChar = *ptr;
  if (operatorChar != '+' && operatorChar != '-' && operatorChar != '*' && operatorChar != '/') {
    return false;                             // Invalid operator
  }

  ptr++;
  ptr = skipSpaces(ptr);                      // Skip spaces after operator

  // Read second number
  secondNumber = strtol(ptr, &endPtr, 10);
  if (ptr == endPtr) return false;             // No second number found

  // Check that nothing extra follows
  ptr = skipSpaces(endPtr);
  if (*ptr != '\0') return false;              // Extra junk detected after second number

  return true; // Parsing was successful
}

// Calculates the result based on two numbers and an operator
float calculate(int num1, char op, int num2) {
  switch (op) {
    case '+': return num1 + num2;
    case '-': return num1 - num2;
    case '*': return num1 * num2;
    case '/': 
      if (num2 == 0) return 0; // Division by zero protection
      return (float)num1 / num2;
    default: 
      return 0; // Should never happen because parsing already checked
  }
}
