#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SUCCESS 0
#define MEMORY_ERROR 1
#define INCORRECT_INPUT 2
#define BASE 256

typedef struct Bigint {
        int high_digit;
        unsigned int* digits;
} Bigint;

void small_multiply(Bigint* number, int value) {
        unsigned long long carry = 0;
        unsigned int i;

        for (i = 1; i <= number->digits[0]; i++) {
                unsigned long long temp = number->digits[i] * value + carry;                
                number->digits[i] = (unsigned int)(temp % BASE);
                carry = temp / BASE;

                if (carry == 0) return;
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

                if (carry == 0) return;
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
        for (int i = 0; i < strlen(value); i++) {
                small_multiply(number, 10);
                small_add(number, value[i] - '0');
        }
}

void print_number(Bigint* number) {
        if (number == NULL) {
                printf("Number is not initilized!");
                return;
        } else {
                printf("%u ",number->high_digit);
                for (int i = number->digits[0]; i > 0; i--) {
                        printf("%u ", number->digits[i]);
                }
        }
        printf("\n");
        // printf("\b\n");
        // printf("%u\n", number->digits[0]);
}

int main(void) {
        char* a = "4294967299";
        Bigint* number = init();
        assign_value(number, a);
        print_number(number);
        return SUCCESS;
}
