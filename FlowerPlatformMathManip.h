/*
 *  Author: Anca Barbu
 */

#ifndef FlowerPlatformMathManip_h
#define FlowerPlatformMathManip_h

int getNumberOfDigitsBeforeDecimal(double num){
	if (abs(num) < 1) {
		return 1;
	}
	return floor (log10 (abs (int(num)))) + 1;
}

// doesn't work properly, it must be enhanced
int getNumberOfDigitsAfterDecimal(double num){
	int numberOfDigitsAfterDecimal = 0;
	num = abs(num);
	num = num - int(num);
	while (num > 0.001) {
		Serial.println(num);
		num = num * 10;
		numberOfDigitsAfterDecimal = numberOfDigitsAfterDecimal + 1;
		num = num - int(num);
	}
	return numberOfDigitsAfterDecimal;
}

void dtoa(char* numberAsStr, double number){
	Serial.println(number);
	int numberOfDigitsBeforeDecimal = getNumberOfDigitsBeforeDecimal(number);
	int numberOfDigitsAfterDecimal = 2; //TODO it should be calculated, see getgetNumberOfDigitsAfterDecimal() function
	dtostrf(number, numberOfDigitsBeforeDecimal + 1 + numberOfDigitsAfterDecimal, numberOfDigitsAfterDecimal, numberAsStr);
	Serial.println(numberAsStr);
}

int min_array(int n, ...) {
  int i = 0,val, minim;
  va_list vl;
  va_start(vl, n);
  minim = va_arg(vl, int);
  for (i = 1; i < n; i++) {
    val = va_arg(vl, int);
    minim = (val < minim) ? val : minim;
  }
  va_end(vl);
  return minim;
}

int max_array(int n, ...) {
  int i = 0,val, max;
  va_list vl;
  va_start(vl, n);
  max = va_arg(vl, int);
  for (i = 1; i < n; i++) {
    val = va_arg(vl, int);
    max = (max > val)?max : val;
  }
  va_end(vl);
  return max;
}


double setPrecision(double number, int prec) {
  char numberAsStr[24];
  int numberOfDigits = getNumberOfDigitsBeforeDecimal(number);
  dtostrf(number, numberOfDigits+prec, prec, numberAsStr);
  return strtod(numberAsStr, NULL);
}

String hex(int number) {
  return String(number, HEX);
}

String bin(int number) {
  return String(number, BIN);
}

int hexToBase10(String number) {
  int ul = strtoul (number.c_str(), NULL, 16);
  return ul;
}

int binaryToBase10(String number) {
  int ul = strtoul (number.c_str(), NULL, 2);
  return ul;
}

int powint(int x, int y) {
	int val = x;
	for (int z = 0; z <= y; z++) {
		if (z == 0) {
			val = 1;
		}
		else {
		val = val * x;
		}
	}
 return val;
}

#endif
