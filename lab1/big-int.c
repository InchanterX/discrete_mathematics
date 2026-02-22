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
        unsigned int i;

        for (i = 1; i <= number->digits[0]; i++) {
                unsigned long long temp = number->digits[i] * value + carry;                
                number->digits[i] = (unsigned int)(temp % BASE);
                carry = temp / BASE;
        }

        if (carry > 0) {
                number->digits = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                // TODO Add reallocation memory check
                number->digits[0]++;
                number->digits[i] = (unsigned int)carry;
        }
}

void small_add(Bigint* number, int value) {
        unsigned long long carry = value;
        unsigned int i;

        for (i = 1; i <= number->digits[0]; i++) {
                unsigned long long temp = number->digits[i] + carry;
                number->digits[i] = (unsigned int)(temp % BASE);
                carry = temp / BASE;
        }
        if (carry > 0) {
                number->digits = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));;
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
                number->digits = (unsigned int*)realloc(number->digits, number->digits[0] * sizeof(unsigned int));
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
                        printf("-%d ",number->high_digit - (1 << (sizeof(int) * 8 - 1))); 
                } else {
                        printf("%d ",number->high_digit);
                }
                for (int i = number->digits[0]; i > 0; i--) {
                        printf("%u ", number->digits[i]);
                }
        }
        printf("\n");
        // printf("\b\n");
        // printBits(4294967295 >> (sizeof(int) * 8 - 1));
        printf("%u\n", number->digits[0]);
}

int main(void) {
        char* a = "4294967299";
        Bigint* number1 = init();
        assign_value(number1, a);
        print_number(number1);

        char* b = "-4294967299";
        Bigint* number2 = init();
        assign_value(number2, b);
        print_number(number2);
        return SUCCESS;
}
