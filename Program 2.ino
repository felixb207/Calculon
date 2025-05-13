// Arduino Calculator in pure C (no C++ features)

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
}

// Helper: skip leading spaces
char* skip_spaces(char* str) {
  while (*str == ' ') str++;
  return str;
}

// Parses a math operation string like "10 + 5"
int parse_input(char* input, int* num1, char* op, int* num2) {
  char* ptr = skip_spaces(input);
  char* endptr;

  // Parse first number
  *num1 = strtol(ptr, &endptr, 10);
  if (ptr == endptr) return 0; // No number found

  ptr = skip_spaces(endptr);

  // Parse operator
  *op = *ptr;
  if (*op != '+' && *op != '-' && *op != '*' && *op != '/') return 0;

  ptr++;
  ptr = skip_spaces(ptr);

  // Parse second number
  *num2 = strtol(ptr, &endptr, 10);
  if (ptr == endptr) return 0; // No second number found

  ptr = skip_spaces(endptr);
  if (*ptr != '\0') return 0; // Unexpected characters

  return 1; // Valid input
}

// Perform calculation
float calculate(int a, char op, int b) {
  switch (op) {
    case '+': return (float)(a + b);
    case '-': return (float)(a - b);
    case '*': return (float)(a * b);
    case '/': return b != 0 ? (float)a / b : 0.0;
    default:  return 0.0;
  }
}

void loop() {
  static char buffer[64];
  static int index = 0;

  // Read incoming bytes
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      buffer[index] = '\0'; // Null-terminate string

      // Reset index for next input
      index = 0;

      int num1, num2;
      char op;
      if (parse_input(buffer, &num1, &op, &num2)) {
        float result = calculate(num1, op, num2);
        char resultBuffer[32];
        dtostrf(result, 0, 2, resultBuffer); // Convert float to string with 2 decimal digits
        Serial.println(resultBuffer);
      } else {
        Serial.println("ERROR: invalid input");
      }
    } else {
      // Store character in buffer if there's space
      if (index < sizeof(buffer) - 1) {
        buffer[index++] = c;
      }
    }
  }
}
