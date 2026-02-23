#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define SUCCESS 0
#define MEMORY_ERROR 1
#define INCORRECT_INPUT 2
#define BASE 256

typedef struct Bigint {
        int high_digit;
        unsigned int* digits;
} Bigint;

void printBits(unsigned int x) {
    int bits = sizeof(x) * CHAR_BIT;
    
    for (int i = bits - 1; i >= 0; i--) {
        unsigned int bit = (x >> i) & 1;
        printf("%u", bit);
        
        if (i % 8 == 0 && i != 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void small_multiply(Bigint* number, int value) {
        unsigned long long carry = 0;

        for (unsigned int i = 1; i <= number->digits[0]; i++) {
                unsigned long long temp = (unsigned long long)number->digits[i] * value + carry;                
                number->digits[i] = (unsigned int)temp;
                carry = temp >> (8 * sizeof(unsigned int));
        }

        if (carry) {
                number->digits = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                // TODO Add reallocation memory check
                number->digits[0]++;
                number->digits[number->digits[0]] = (unsigned int)carry;
        }
}

void small_add(Bigint* number, int value) {
        unsigned long long temp = (unsigned long long)number->digits[1] + value;
        number->digits[1] = (unsigned int)temp;
        unsigned long long carry = temp >> (sizeof(unsigned int) * 8);
        unsigned int i = 2;

         while (carry && i <= number->digits[0]) {
                temp = (unsigned long long)number->digits[i] + carry;
                number->digits[i] = (unsigned int)temp;
                carry = temp >> (sizeof(unsigned int) * 8);
                i++;
        } 
        if (carry) {
                number->digits = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                // TODO Add reallocation memory check
                number->digits[0]++;
                number->digits[i] = (unsigned int)carry;
        }
}

Bigint* init(void) {
        Bigint* number = (Bigint*)malloc(sizeof(Bigint));
        if (!number) return NULL;

        number->digits = (unsigned int*)malloc(sizeof(unsigned int) * 2);
        if (!number->digits) {
                free(number);
                return NULL;
        }

        number->high_digit = 0;
        number->digits[0] = 1;
        number->digits[1] = 0;
        return number;
}

void assign_value(Bigint* number, char* value) {
        // TODO check number to be 0 or auto-zero it
        char negative = 0;
        char start = 0;

        if (value[0] == '-') {
                negative = 1;
                start = 1;
        }
        for (int i = start; i < strlen(value); i++) {
                small_multiply(number, 10);
                small_add(number, value[i] - '0');
        }
        if (number->digits[number->digits[0]] < (1U << (8 * sizeof(unsigned int) - 1))) {
                number->high_digit = number->digits[number->digits[0]];
                number->digits[number->digits[0]] = 0;
                number->digits[0]--;
                // number->digits[0] = (number->digits[0] == 0) ? 0 : number->digits[0]--;
                number->digits = (unsigned int*)realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
        }
        if (negative) {
                number->high_digit |= (1U << (sizeof(unsigned int) * 8 - 1));
        }
}

void print_number(Bigint* number) {
        if (number == NULL) {
                printf("Number is not initilized!");
                return;
        } else {
                if (number->high_digit >> (sizeof(int) * 8 - 1)) {
                        printf("-%9.d ",number->high_digit - (1 << (sizeof(int) * 8 - 1))); 
                } else {
                        printf("%9.d ",number->high_digit);
                }
                for (int i = number->digits[0]; i > 0; i--) {
                        printf("%10.u ", number->digits[i]);
                }
        }
        // printf("\n");
        // printf("\b\n");
        // printBits(4294967295 >> (sizeof(int) * 8 - 1));
        printf("(%u)\n", number->digits[0]);
}

// unsigned int sum_bits(unsigned int value1, unsigned int value2) {
//         unsigned int max_number = (1U - 2);
//         unsigned int remainder = max_number - value1;
//         // if remainder is greater than second number it means that sum is suitable for this position
//         if (remainder > value2) {

//         }
// }       

Bigint* sum(Bigint* number1, Bigint* number2) {
        unsigned int max_number_ui = (0U - 1);
        unsigned int max_number_i = ~(1 << (sizeof(int) * 8 - 1));
        unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));

        // determine greater number out of two
        Bigint* great_number = NULL;
        if (number1->digits[0] >= number2->digits[0]) {
                great_number = number1; 
        } else {
                great_number = number2;
        }

        unsigned int min_length = (number1->digits[0] < number2->digits[0]) ? number1->digits[0] : number2->digits[0];
        unsigned int remainder = 0;
        unsigned int carry = 0;

        // in case of the same sign just sum up values
        if ((number1->high_digit & sign_mask) == (number2->high_digit & sign_mask)) {
                for (int i = 1; i <= min_length; i++) {
                        // proccess the carried value
                        if (carry != 0) {
                                remainder = max_number_ui - number1->digits[i];
                                // if remainder fit the bit
                                if (remainder > carry) {
                                        number1->digits[i] += carry;
                                        carry = 0;
                                } else {
                                        remainder = carry - remainder;
                                        number1->digits[i] = remainder;
                                        carry = 1;
                                }
                        }

                        // summary of two numbers
                        remainder = max_number_ui - number1->digits[i];

                        // if remainder is greater than second number it means that sum is suitable for this position
                        if (remainder > number2->digits[i]) {
                                number1->digits[i] += number2->digits[i];
                                continue;
                        }
                        // otherwise it is greater than max number, carry is set to 1 and 
                        remainder = number2->digits[i] - remainder;
                        carry++;

                }
                // proccess the carried value
                char sign = (number1->high_digit & sign_mask) ? 1 : 0;
                if (sign) {
                        number1->high_digit *= -1;
                        number2->high_digit *= -1;
                }
                if (carry != 0) {
                        remainder = max_number_i - number1->high_digit;
                        // if remainder fit the bit
                        if (remainder > carry) {
                                number1->high_digit += carry;
                                carry = 0;
                        } else {
                                remainder = carry - remainder;
                                number1->high_digit = remainder;
                                carry = 1;
                        }
                }

                // summary of two numbers
                remainder = max_number_i - number1->high_digit;

                // if remainder is greater than second number it means that sum is suitable for this position
                if (remainder > number2->high_digit) {
                        number1->high_digit += number2->high_digit;
                } else {
                        // otherwise it is greater than max number, carry is set to 1 and 
                        remainder = number2->high_digit - remainder;
                        carry++;
                }

                // distribute remaining carry value
                if (carry) {
                        number1->digits = realloc(number1->digits, (number1->digits[0] + 2) * sizeof(unsigned int));
                        number1->digits[number1->digits[0]] = (unsigned int)(number1->high_digit) + carry;
                        number1->high_digit = 0;
                }

                // return sign
                if (sign) {
                        number1->high_digit *= -1;
                        number2->high_digit *= -1;
                }
                return number1;
        } else {
                printf("What a hell, I won't work with it, do it by yourself bro.");
        }
}

int main(void) {
        char* a = "2129457989784522267299";
        // char* a = "4267399";
        Bigint* number1 = init();
        assign_value(number1, a);
        print_number(number1);

        printf("+\n");

        char* b = "1423564989346289667299";
        // char* b = "4294946";
        Bigint* number2 = init();
        assign_value(number2, b);
        print_number(number2);

        printf("=\n");

        Bigint* number3 = sum(number1, number2);
        print_number(number3)   ;
        return SUCCESS;
}
