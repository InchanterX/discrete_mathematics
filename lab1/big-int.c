#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define SUCCESS 0
#define MEMORY_ERROR 1
#define INCORRECT_INPUT 2
#define BASE (sizeof(int) * 8)

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
        if (!number || !number->digits) return;

        if (value == 0) {
                number->high_digit = 0;
                number->digits[0] = 0;
                return;
        }

        unsigned int WORD_BITS = sizeof(unsigned int) * CHAR_BIT;
        unsigned int sign_mask = 1U << (WORD_BITS - 1);
        unsigned int abs_mask  = sign_mask - 1;
        unsigned int sign = number->high_digit & sign_mask;

        // in case of empty array
        if (number->digits[0] == 0) {
                unsigned int abs_high = number->high_digit & abs_mask;
                unsigned long long tmp = (unsigned long long)abs_high * value;
                if ((tmp >> WORD_BITS) == 0) {
                    number->high_digit = (unsigned int)tmp | sign;
                    return;
                }

                unsigned int* tmp_ptr = realloc(number->digits, 2 * sizeof(unsigned int));
                if (!tmp_ptr) return;
                number->digits = tmp_ptr;

                number->digits[0] = 1;
                number->digits[1] = (unsigned int)tmp;
                number->high_digit = (unsigned int)(tmp >> WORD_BITS) | sign;
                return;
            }

        // in case of not empty array
        unsigned long long carry = 0;

        for (unsigned int i = 1; i <= number->digits[0]; i++) {
                unsigned long long temp = (unsigned long long)number->digits[i] * value + carry;                
                number->digits[i] = (unsigned int)temp;
                carry = temp >> (8 * sizeof(unsigned int));
        }

        if (carry) {
                number->digits = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                if (number->digits == NULL) return;
                number->digits[0]++;
                number->digits[number->digits[0]] = (unsigned int)carry;
        }
}

void small_add(Bigint* number, int value) {
        if (number == NULL) return;

        unsigned int WORD_BITS = sizeof(unsigned int) * 8;
        unsigned int sign_mask = 1U << (WORD_BITS - 1);
        unsigned int abs_mask  = sign_mask - 1;
        unsigned long long carry = value;

        // in case of empty array
        if (number->digits[0] == 0) {
                unsigned int abs_high = number->high_digit & abs_mask;
                unsigned int sign = number->high_digit & sign_mask;

                unsigned long long tmp = (unsigned long long)abs_high + carry;
                if (tmp <= abs_mask) {
                    number->high_digit = (unsigned int)tmp | sign;
                    return;
                }

                unsigned int* tmp_ptr = realloc(number->digits, 2 * sizeof(unsigned int));
                if (!tmp_ptr) return;
                number->digits = tmp_ptr;

                number->digits[0] = 1;
                number->digits[1] = (unsigned int)tmp;
                number->high_digit = (unsigned int)(tmp >> WORD_BITS) | sign;
                return;
        }

        // in case of not empty array
        unsigned int i = 1;

        while (carry && i <= number->digits[0]) {
                unsigned long long temp = (unsigned long long)number->digits[i] + carry;
                number->digits[i] = (unsigned int)temp;
                carry = temp >> (sizeof(unsigned int) * 8);
                i++;
        } 
        if (carry) {
                // TODO do this way everywhere to protect the program
                unsigned int* tmp_ptr = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                if (!tmp_ptr) return;
                number->digits = tmp_ptr;
                number->digits[0]++;
                number->digits[number->digits[0]] = (unsigned int)carry;
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
        if (number == NULL) return;
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
                number->digits = (unsigned int*)realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
                if (number->digits == NULL) return;
        }
        if (negative) {
                number->high_digit |= (1U << (sizeof(unsigned int) * 8 - 1));
        }
}

void normolize(Bigint* number) {
        unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));
        unsigned int max_number_i = ~sign_mask;
        if ((number->high_digit & (sign_mask - 1)) != 0) return;
        int i = number->digits[0];
        int cnt = 0;
        while (i > 0 && number->digits[i] == 0) {
                cnt++;
                i--;
        }

        if ((i != 0) && (number->digits[i] <= max_number_i)) {
                int sign = number->high_digit & sign_mask;
                number->high_digit = number->digits[i];
                if (sign) {
                        number->high_digit |= sign_mask;
                }
                cnt++;
        }

        number->digits[0] -= cnt;
        unsigned int* temp_ptr = (unsigned int*)realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;
        return;
}

void print_number(Bigint* number) {
}

void number_debug(Bigint* number) {
        if (number == NULL) {
                printf("Number is not initilized!\n");
                return;
        } else {
                if (number->high_digit >> (sizeof(int) * 8 - 1)) {
                        printf("-%9.dh ",number->high_digit - (1 << (sizeof(int) * 8 - 1)));
                } else {
                        printf("%9.dh ",number->high_digit);
                }
                if (!(number->digits)) {
                        printf("None");
                } else {
                        for (int i = number->digits[0]; i > 0; i--) {
                                printf("%10.u ", number->digits[i]);
                        }
                }
        }
        if (!(number->digits)) {
                printf("(Digits: 1) (Memory: %lu) Content: ", sizeof(int));
        } else {
                printf("(Digits: %u) (Memory: %lu) Content: ", number->digits[0] + 1, (number->digits[0] + 1) * sizeof(unsigned int) + sizeof(int));
        }
        print_number(number);
        printf("\n");
}

Bigint* sum(Bigint* number1, Bigint* number2) {
        if (number1 == NULL || number2 == NULL) return NULL;
        // unsigned int max_number_ui = (0U - 1);
        unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));
        unsigned int max_number_i = ~sign_mask;
        unsigned int min_length = (number1->digits[0] < number2->digits[0]) ? number1->digits[0] : number2->digits[0];
        unsigned int carry = 0;
        long long unsigned summary = 0;

        // in case of the same sign just sum up values
        if ((number1->high_digit & sign_mask) == (number2->high_digit & sign_mask)) {

                // in case of empty array
                if (number1->digits[0] == 0 && number2->digits[0] == 0) {
                        int sign = number1->high_digit & sign_mask;
                        unsigned int abs_high1 = number1->high_digit & max_number_i;
                        unsigned int abs_high2 = number2->high_digit & max_number_i;
                        summary = (unsigned long long)abs_high1 + abs_high2 + carry;
                        unsigned int new_high = (unsigned int)summary;
                        carry = summary >> (sizeof(unsigned int) * 8);
                        if (carry != 0) {
                                number1->digits = (unsigned int*)realloc(number1->digits, 
                                (number1->digits[0] + 2) * sizeof(unsigned int));
                                if (number1->digits == NULL) return NULL;
                                number1->digits[0]++;
                                number1->digits[number1->digits[0]] = new_high;
                        } else {
                                number1->high_digit = new_high;
                        }
                        if (sign) number1->high_digit |= sign_mask;
                        return number1;
                }

                for (int i = 1; i <= min_length; i++) {
                        summary = (long long unsigned)number1->digits[i] +
                         (long long unsigned)number2->digits[i] + carry;
                        number1->digits[i] = (unsigned int)summary;
                        carry = summary >> (sizeof(unsigned int) * 8);
                }
                
                if (number1->digits[0] > number2->digits[0]) {
                        int i = min_length + 1;
                        summary = (unsigned int)(number2->high_digit & max_number_i) + number1->digits[i] + carry;
                        number1->digits[i] = (unsigned int)summary;
                        carry = summary >> (sizeof(unsigned int) * 8);
                        i++;
                        while ((carry != 0) && (i <= number1->digits[0])) {
                                summary = (long long unsigned)number1->digits[i] + carry;
                                number1->digits[i] = (unsigned int)summary;
                                carry = summary >> (sizeof(unsigned int) * 8);
                                i++;
                        }
                } else if (number1->digits[0] < number2->digits[0]) {
                        int i = min_length + 1;
                        number1->digits = (unsigned int*)realloc(number1->digits, (number1->digits[0] +
                         (number2->digits[0] - min_length) + 1) * sizeof(unsigned int));
                        if (number1->digits == NULL) return NULL;
                        number1->digits[0] += (number2->digits[0] - min_length);

                        summary = (unsigned int)(number1->high_digit & max_number_i) + number2->digits[i] + carry;
                        number1->digits[i] = (unsigned int)summary;
                        carry = summary >> (sizeof(unsigned int) * 8);
                        i++;

                        while (i <= number2->digits[0]) {
                                summary = (long long unsigned)number2->digits[i] + carry;
                                number1->digits[i] = (unsigned int)summary;
                                carry = summary >> (sizeof(unsigned int) * 8);
                                i++;
                        }

                        number1->high_digit = number2->high_digit;
                } else {
                        int sign = number1->high_digit & sign_mask;
                        unsigned int abs_high1 = number1->high_digit & max_number_i;
                        unsigned int abs_high2 = number2->high_digit & max_number_i;
                        summary = (unsigned long long)abs_high1 + abs_high2 + carry;
                        unsigned int new_high = (unsigned int)summary;
                        carry = summary >> (sizeof(unsigned int) * 8);
                        if (carry != 0 || (new_high > max_number_i)) {
                                number1->digits = (unsigned int*)realloc(number1->digits, 
                                (number1->digits[0] + 2) * sizeof(unsigned int));
                                if (number1->digits == NULL) return NULL;
                                number1->digits[0]++;
                                number1->digits[number1->digits[0]] = new_high + carry;
                                number1->high_digit &= sign_mask;
                        } else {
                                number1->high_digit = (int)new_high;
                        }
                        if (sign) number1->high_digit |= sign_mask;
                }
                return number1;
        } else {
                // determine greater number out of two
                Bigint* great_number = NULL; 
                Bigint* less_number = NULL; 
                unsigned int abs_high1 = number1->high_digit & max_number_i; 
                unsigned int abs_high2 = number2->high_digit & max_number_i; 
                if (number1->digits[0] > number2->digits[0]) { 
                        great_number = number1; 
                        less_number = number2; 
                } else if (number1->digits[0] < number2->digits[0]) { 
                        great_number = number2; 
                        less_number = number1; 
                } else if (abs_high1 > abs_high2) { 
                        great_number = number1; 
                        less_number = number2; 
                } else if (abs_high1 < abs_high2) { 
                        great_number = number2; 
                        less_number = number1; 
                } else { 
                        int i = number1->digits[0]; 
                        while ((number1->digits[i] == number2->digits[i]) && i > 0) i--; 
                        if (number1->digits[i] > number2->digits[i]) { 
                                great_number = number1; 
                                less_number = number2; 
                        } else if (number1->digits[i] <= number2->digits[i]) { 
                                great_number = number2; 
                                less_number = number1; 
                        } 
                } 
                unsigned int abs_great = great_number->high_digit & max_number_i;
                unsigned int abs_less = less_number->high_digit & max_number_i;

                // in case of empty arrays
                if (number1->digits[0] == 0 && number2->digits[0] == 0) {
                        int high_sign = great_number->high_digit & sign_mask;
                        unsigned int result = abs_great - abs_less - carry;
                        great_number->high_digit = result;
                        if (high_sign) great_number->high_digit |= sign_mask;
                        return number1;
                }

                for (int i = 1; i <= min_length; i++) {
                        if (great_number->digits[i] < less_number->digits[i] + carry) {
                                summary = (1ULL << BASE) + (long long unsigned)great_number->digits[i] -
                                (long long unsigned)less_number->digits[i] - carry;
                                // printf("%llu + %u - %u - %u = %llu\n", 1ULL << BASE, great_number->digits[i], less_number->digits[i], carry, summary);
                                carry = 1;
                        } else {
                                summary = (long long unsigned)great_number->digits[i] -
                                 (long long unsigned)less_number->digits[i] - carry;
                                // printf("%u - %u - %u = %llu\n", great_number->digits[i], less_number->digits[i], carry, summary);
                                carry = 0;
                        }
                        great_number->digits[i] = (unsigned int)summary;
                }

                if (great_number->digits[0] > less_number->digits[0]) {
                        int i = min_length + 1;
                        if (great_number->digits[i] < abs_less + carry) {
                                summary = (long long unsigned)(1ULL << BASE) + (long long unsigned)great_number->digits[i] -
                                (long long unsigned)abs_less - carry;
                                carry = 1;
                        } else {
                                summary = (long long unsigned)great_number->digits[i] -
                                 (long long unsigned)abs_less - carry;
                                carry = 0;
                        }
                        great_number->digits[i] = (unsigned int)summary;
                        i++;

                        while (carry && (i <= great_number->digits[0])) {
                                if (great_number->digits[i] < carry) {
                                        summary = (long long unsigned)(1ULL << BASE) + 
                                        (long long unsigned)great_number->digits[i] - carry;
                                        carry = 1;
                                } else {
                                        summary = (long long unsigned)great_number->digits[i] - carry;
                                        carry = 0;
                                }
                                great_number->digits[i] = (unsigned int)summary;
                        }

                        great_number->high_digit -= carry;
                } else {
                        int high_sign = great_number->high_digit & sign_mask;
                        unsigned int result = abs_great - abs_less - carry;
                        great_number->high_digit = result;
                        if (high_sign) great_number->high_digit |= sign_mask;      
                }

                normolize(great_number);
                return great_number;
        }
}

unsigned int loword(int value) {
    return value & ((1U << (sizeof(value) << 2)) - 1);
}

unsigned int hiword(int value) {
    return value & (~((1U << (sizeof(value) << 2)) - 1));
}

Bigint* mult(Bigint* number1, Bigint* number2) {
        // Input validation
        if (number1 == NULL || number2 == NULL) return NULL;

        // Auxilary variables
        unsigned int sign_mask = (1 << ((sizeof(unsigned int) << 3) - 1));
        unsigned int value_mask = ~sign_mask;

        // Multiplication variables
        long long unsigned carry = 0;
        unsigned int loword1, loword2, hiword1, hiword2;
        unsigned int interim_value1, interim_value2;
        unsigned int product_high, product_cross1, product_cross2, product_low;
        long long unsigned sum1;
        Bigint* result = init();

        // Case of one number being equal to zero
        if ((!(number1->digits) && (number1->high_digit & sign_mask)) || 
                (!(number2->digits) && (number2->high_digit & sign_mask))) {
                result->high_digit = 0;
                return result;
        }

        // Product sign determination
        unsigned int sign = 0;
        if (((number1->high_digit & sign_mask) > 0) ^ ((number2->high_digit & sign_mask) > 0)) sign = 1;

        // Calculating maximum length of the product + technical space, lengths separetely
        unsigned int result_length = 3;
        unsigned int first_length = 1, second_length = 1;
        if (number1->digits) {
                result_length += (number1->digits[0] + 1);
                first_length += (number1->digits[0] + 1);
        }
        if (number2->digits) {
                result_length += number2->digits[0];
                second_length += number2->digits[0];
        }

        // !!! There is a memory leak here. It can be fixed by modifing init function !!! 
        // Memory allocation for the product 
        unsigned int* temp_ptr = (unsigned int*)calloc(result_length, sizeof(unsigned int));
        if (!temp_ptr) return NULL;
        result->digits = temp_ptr;
        result->digits[0] = result_length - 1;

        printf("4");

        // Long multiplication algorithm
        for (int i = 1; i < first_length; i++) {
                carry = 0;

                // Split digit of the first number to loword and hiword
                if (i == first_length) {
                        loword1 = loword(number1->high_digit);
                        hiword1 = (hiword(number1->high_digit & value_mask)) >> (sizeof(unsigned int) << 1);
                } else {
                        loword1 = loword(number1->digits[i]);
                        hiword1 = (hiword(number1->digits[i])) >> (sizeof(unsigned int) << 1);
                }

                for (int j = 1; j < second_length; j++) {

                        // Split digit of the second number to loword and hiword
                        if (j == second_length) {
                                loword2 = loword(number2->high_digit);
                                hiword2 = (hiword(number2->high_digit & value_mask)) >> (sizeof(unsigned int) << 1);
                        } else {
                                loword2 = loword(number2->digits[j]);
                                hiword2 = (hiword(number2->digits[j])) >> (sizeof(unsigned int) << 1);
                        }

                        // Multiplication of the digits' elements
                        product_low = loword1 * loword2;
                        product_cross1 = loword1 * hiword2;
                        product_cross2 = loword2 * hiword1;
                        product_high = hiword1 * hiword2;

                        // Cross-elements summation
                        interim_value1 = (loword(product_cross1) >> (sizeof(unsigned int) << 1)) +
                        (loword(product_cross2) >> (sizeof(unsigned int) << 1));
                        interim_value2 = hiword(product_cross1) + hiword(product_cross2);

                        // Overflow value determination and current vakue assignment
                        sum1 = product_low + interim_value1 + carry;
                        carry = ((long long unsigned)sum1 + (long long unsigned)result->digits[i + j - 1]) >> 
                        (sizeof(unsigned int) << 3);
                        carry += (product_high + interim_value2);
                        result->digits[i + j - 1] += sum1;
                }

                // add remainder to the next digit
                if (carry) result->digits[i + second_length - 1] += carry;
        }

        // Result normalization
        normolize(result);
        if (result->digits[0] && result->digits[0] > 0 && result->digits[result->digits[0]] <= value_mask) {
                result->high_digit = result->digits[result->digits[0]];

                unsigned int new_length = result->digits[0] - 1;

                if (new_length == 0) {
                        free(result->digits);
                        result->digits = NULL;
                } else {
                        unsigned int* temp_ptr = 
                        (unsigned int*)realloc(result->digits, (new_length + 1) * sizeof(unsigned int));
                        if (!temp_ptr) return NULL;
                        result->digits = temp_ptr;
                }

                result->digits[0] = new_length;
        }

        // Sign assignment
        if (sign) result->high_digit |= sign_mask;

        return result;
}

int main(void) {
        // char* a = "-21294579897845434784556789046789522267299";
        // // char* a = "-1423564989346289667299";
        // // char* a = "-42";
        // // char* a = "47345897893450224385723751231272112";
        // // char* a = "-97345897893450224385723751231272112000";
        // // char* a = "97345897893450224385723751231272112000";
        // Bigint* number1 = init();
        // assign_value(number1, a);
        // number_debug(number1);

        // printf("+\n");

        // char* b = "1423564989346289667299";
        // // char* b = "21294579897845434784556789046789522267299";
        // // char* b = "4";
        // // char* b = "-47345897893450224385723751231356913";
        // // char* b = "-97345897893450224385723751231272112000";
        // // char* b = "-97345897893450224385723751231272112000";
        // Bigint* number2 = init();
        // assign_value(number2, b);
        // number_debug(number2);

        // printf("=\n");

        // Bigint* number3 = sum(number1, number2);
        // number_debug(number3);

        char* a = "1423564989346289667299";
        Bigint* number1 = init();
        assign_value(number1, a);
        number_debug(number1);

        char* b = "263390250395733";
        Bigint* number2 = init();
        assign_value(number2, b);
        number_debug(number2);

        Bigint* result = mult(number1, number2);
        number_debug(result);

        return SUCCESS;
}
