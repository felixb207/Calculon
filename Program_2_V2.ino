// Arduino Calculator 
// Program_2 Arduino Side
//Author: Felix Bohdal
//Last Change: 11.07.2025
// Code was improved and Bug-fixed with the Help of Chat-GPT 4o

/*
Changelog 10.07.2025:

1.Changed Datatype for result, so only division will be sent with 2 decimal digits. Other operations can only result in whole numbers. 

2.Changed Buffer Size to 100, same as in PC User Interface. Also added Buffer full Error

3.Removed additional Terminate Character.

4. Added Division by zero Error to Calculate function

*/

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

// Perform calculation and check for Division by Zero
float calculate(int a, char op, int b) {
  
  switch (op) {
    case '+': return (int)(a + b);
    case '-': return (int)(a - b);
    case '*': return (int)(a * b);
    case '/': 
      if (b != 0) {
        return (float)a / b;
      } else {
        Serial.println("ERROR_DIV_0");
      }
      default: return 0.0;
  }
}

void loop() {
  static char buffer[100];
  static int index = 0;
  float result = 0.0;


  // Read incoming bytes
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n') {
      buffer[index] = '\0'; // Null-terminate string

      // Reset index for next input
      index = 0;

      int num1, num2;
      char op;
      if (parse_input(buffer, &num1, &op, &num2)) {
        
        result = calculate(num1, op, num2);
        char resultBuffer[100];
        
        //only show decimals after Division was performed
        if (op == '/' && num2 != 0) {
          dtostrf(result, 0, 2, resultBuffer); // Convert float to string with 2 decimal digits
        } else {
          dtostrf(result, 0, 0, resultBuffer); // Convert float to string with 0 decimal digits
        }
        
        Serial.println(resultBuffer);

      } else {
        Serial.println("ERROR_INVALID_INPUT");
      }
    } else {
      // Store character in buffer if there's space
      if (index < sizeof(buffer) - 1) {
        buffer[index++] = c;
      } else {
        Serial.println("ERROR_BUFFER_FULL");
        break;
      }
    }
  }
}
