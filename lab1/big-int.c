#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define SUCCESS 0
#define MEMORY_ERROR 1
#define INCORRECT_INPUT 2
#define BASE (sizeof(int) * 8)

#define BASE_BITS   (8u * (unsigned)sizeof(unsigned int))
#define BASE_ULL    (1ULL << BASE_BITS)
#define DIGIT_MASK  ((unsigned long long)UINT_MAX)
#define SIGN_MASK_U ((unsigned int)(1u << (BASE_BITS - 1u)))
#define ABS_MASK_U  ((unsigned int)(SIGN_MASK_U - 1u))

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
}

void printBits_long(long long unsigned x) {
    int bits = sizeof(x) * CHAR_BIT;
    
    for (int i = bits - 1; i >= 0; i--) {
        unsigned int bit = (x >> i) & 1;
        printf("%u", bit);
        
        if (i % 8 == 0 && i != 0) {
            printf(" ");
        }
    }
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
                unsigned int* temp_ptr = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
                if (temp_ptr == NULL) return;
                number->digits = temp_ptr;
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
                unsigned int* temp_ptr = (unsigned int*)realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
                if (temp_ptr == NULL) return;
                number->digits = temp_ptr;
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

unsigned int get_word_print(Bigint* number, unsigned int i) {
    if (i < number->digits[0])  return number->digits[i + 1u];
    if (i == number->digits[0]) return (unsigned int)number->high_digit & ABS_MASK_U;
    return 0u;
}

void print_number(Bigint* number) {
        unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1)); 
        if (((unsigned int)number->high_digit & ABS_MASK_U) == 0 && number->digits[0] == 0) {
                puts("0"); return;
        }
        if (number->high_digit & sign_mask) putchar('-');

        unsigned int n = number->digits[0] + 1u;
        unsigned int *buffer = (unsigned int *)malloc(n * sizeof(unsigned int));
        if (!buffer) { printf("Memory error!\n"); return; }
        for (unsigned int i = 0; i < n; i++) buffer[i] = get_word_print(number, i);

        char digits[8192];
        int  position = 0;

        while (1) {
                unsigned int topnz = n;
                while (topnz > 0 && buffer[topnz - 1u] == 0) topnz--;
                if (topnz == 0) break;

                unsigned long long remainer = 0;
                for (int i = (int)topnz - 1; i >= 0; i--) {
                    unsigned long long current = remainer * BASE_ULL + buffer[i];
                    buffer[i] = (unsigned int)(current / 10u);
                    remainer = current % 10u;
                }
                digits[position++] = (char)('0' + (int)remainer);
                if (position >= (int)sizeof(digits) - 1) break;
        }
        free(buffer);

        for (int i = position - 1; i >= 0; i--) putchar(digits[i]);
        putchar('\n');
}

void number_debug(Bigint* number) {
        if (number == NULL) {
                printf("Number is not initilized!\n");
                return;
        } else {
                if (number->high_digit >> (sizeof(int) * 8 - 1)) {
                        printf("-%9.d(",number->high_digit - (1 << (sizeof(int) * 8 - 1)));
                        printBits(number->high_digit);
                        printf(")h ");   
                } else {
                        printf("%9.d(",number->high_digit);
                        printBits(number->high_digit);
                        printf(")h ");   
                }
                if (!(number->digits)) {
                        printf("None");
                } else {
                        for (int i = number->digits[0]; i > 0; i--) {
                                printf("%10.u(", number->digits[i]);
                                printBits(number->digits[i]);
                                printf(") ");
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

void sum_interior(Bigint* number1, Bigint* number2) {
        if (number1 == NULL || number2 == NULL) return;
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
                                unsigned int* temp_ptr = (unsigned int*)realloc(number1->digits, 
                                (number1->digits[0] + 2) * sizeof(unsigned int));
                                if (temp_ptr == NULL) return;
                                number1->digits = temp_ptr;
                                number1->digits[0]++;
                                number1->digits[number1->digits[0]] = new_high;
                        } else {
                                number1->high_digit = new_high;
                        }
                        if (sign) number1->high_digit |= sign_mask;
                        return;
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
                        unsigned int* temp_ptr = (unsigned int*)realloc(number1->digits, (number1->digits[0] +
                         (number2->digits[0] - min_length) + 1) * sizeof(unsigned int));
                        if (temp_ptr == NULL) return;
                        number1->digits = temp_ptr;
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
                                unsigned int* temp_ptr = (unsigned int*)realloc(number1->digits, 
                                (number1->digits[0] + 2) * sizeof(unsigned int));
                                if (temp_ptr == NULL) return;
                                number1->digits = temp_ptr;
                                number1->digits[0]++;
                                number1->digits[number1->digits[0]] = new_high + carry;
                                number1->high_digit &= sign_mask;
                        } else {
                                number1->high_digit = (int)new_high;
                        }
                        if (sign) number1->high_digit |= sign_mask;
                }
                return;
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
                        return;
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
                return;
        }
}

Bigint* sum_external(Bigint* number1, Bigint* number2) {
        if (number1 == NULL || number2 == NULL) return NULL;

        Bigint* result = init();
        if (result == NULL) return NULL;

        result->high_digit = number1->high_digit;

        if (number1->digits) {
                unsigned int size = number1->digits[0] + 1;
                result->digits = (unsigned int*)malloc(size * sizeof(unsigned int));
                if (result->digits == NULL) {
                        free(result);
                        return NULL;
                }
                for (unsigned int i = 0; i < size; i++) {
                        result->digits[i] = number1->digits[i];
                }
        } else {
                result->digits = NULL;
        }

        sum_interior(result, number2);
        return result;
}

void sub_interior(Bigint* number1, Bigint* number2) {
        number2->high_digit = (number2->high_digit & SIGN_MASK_U) ? 
                number2->high_digit & ABS_MASK_U : number2->high_digit | SIGN_MASK_U;
        sum_interior(number1, number2);
}

Bigint* sub_external(Bigint* number1, Bigint* number2) {
        number2->high_digit = (number2->high_digit & SIGN_MASK_U) ? 
                number2->high_digit & ABS_MASK_U : number2->high_digit | SIGN_MASK_U;
        sum_external(number1, number2);
}

unsigned int loword(unsigned int value) {
    return value & ((1 << (sizeof(unsigned int) << 2)) - 1);
}

unsigned int hiword(unsigned int value) {
    return value >> (sizeof(unsigned int) << 2);
}

unsigned int get_word(Bigint* number, unsigned int index) {
        if (number->digits) {
            unsigned int count = number->digits[0];
            if (index < count) return number->digits[index + 1u];
            if (index == count) return (unsigned int)number->high_digit & ABS_MASK_U;
            return 0u;
        } else {
            if (index == 0) return (unsigned int)number->high_digit & ABS_MASK_U;
            return 0u;
        }
}

Bigint* mult_external(Bigint* number1, Bigint* number2) {
        if (number1 == NULL || number2 == NULL) return NULL;

        // Calculating lengths
        unsigned int length1 = number1->digits ? (number1->digits[0] + 1u) : 1u;
        unsigned int length2 = number2->digits ? (number2->digits[0] + 1u) : 1u;

        Bigint* result = init();
        if (!result) return NULL;

        // Case of one number being equal to zero
        if ((!number1->digits && !(number1->high_digit & ABS_MASK_U)) ||
                (!number2->digits && !(number2->high_digit & ABS_MASK_U))) {
                result->high_digit = 0;
                free(result->digits);
                result->digits = NULL;
                return result;
        }

        // Sign determination
        unsigned int sign = ((number1->high_digit & SIGN_MASK_U) ^ (number2->high_digit & SIGN_MASK_U)) ? 1u : 0u;

        unsigned int res_words = length1 + length2;
        unsigned int *res = (unsigned int*)calloc(res_words, sizeof(unsigned int));
        if (!res) { free(result); return NULL; }

        unsigned int HALF_BITS = (unsigned int)(sizeof(unsigned int) << 2);
        unsigned int WORD_BITS = (unsigned int)(sizeof(unsigned int) << 3);

        // Long multiplication
        for (unsigned int i = 0; i < length1; ++i) {
                for (unsigned int j = 0; j < length2; ++j) {
                        unsigned int a = get_word(number1, i);
                        unsigned int b = get_word(number2, j);

                        // Calculating products
                        uint64_t product1 = (uint64_t)loword(a) * (uint64_t)loword(b); // low * low
                        uint64_t product2 = (uint64_t)loword(a) * (uint64_t)hiword(b); // low * hi
                        uint64_t product3 = (uint64_t)hiword(a) * (uint64_t)loword(b); // hi  * low
                        uint64_t product4 = (uint64_t)hiword(a) * (uint64_t)hiword(b); // hi  * hi

                        // Calculating split
                        uint64_t middle = product2 + product3;
                        uint64_t buffer = (product4 << (HALF_BITS * 2)) + (middle << HALF_BITS) + product1;

                        // Result distribution between current digit and buffer for next digit
                        unsigned int k = i + j;
                        while (buffer && k < res_words) {
                                uint64_t sum = (uint64_t)res[k] + (buffer & (uint64_t)~0u);
                                res[k] = (unsigned int)sum;
                                buffer = (buffer >> WORD_BITS) + (sum >> WORD_BITS);
                                k++;
                        }
                }
        }

        // Trim leading zeros
        int top = (int)res_words - 1;
        while (top > 0 && res[top] == 0u) top--;

        // Single digit result
        if (top == 0 && (res[0] <= (unsigned int)ABS_MASK_U)) {
                result->high_digit = (int)res[0];
                free(result->digits);
                result->digits = NULL;
        // Multiple digits result
        } else {
                unsigned int lower_count = (unsigned int)top;
                unsigned int *temp = (unsigned int*)realloc(result->digits, (lower_count + 1u) * sizeof(unsigned int));
                if (!temp) { free(res); free(result); return NULL; }
                result->digits = temp;
                result->digits[0] = lower_count;
                for (unsigned int p = 0; p < lower_count; ++p) result->digits[p + 1u] = res[p];
                result->high_digit = (int)res[lower_count];
        }           
        free(res);

        if (sign) result->high_digit |= SIGN_MASK_U;
        return result;
}

void mult_internal(Bigint* number1, Bigint* number2) {
        if (number1 == NULL || number2 == NULL) return;

        Bigint* result = mult_external(number1, number2);
        number1->high_digit = result->high_digit;
        number1->digits = malloc((result->high_digit + 1) * sizeof(unsigned int));
        memcpy(number1->digits, result->digits, (result->high_digit + 1) * sizeof(unsigned int));
}

int main(void) {
        // Tests segment

        // 1 - interior sum: 0 + 0
        printf("Test 1\n");
        char* a1 = "0";
        Bigint* na1 = init();
        assign_value(na1, a1);
        char* b1 = "0";
        Bigint* nb1 = init();
        assign_value(nb1, b1);
        sum_interior(na1, nb1);
        printf("0\n");
        print_number(na1);

        // 2 - external sum: 0 + 0
        printf("\nTest 2\n");
        char* a2 = "0";
        Bigint* na2 = init();
        assign_value(na2, a2);
        char* b2 = "0";
        Bigint* nb2 = init();
        assign_value(nb2, b2);
        Bigint* res_sum00 = sum_external(na2, nb2);
        printf("0\n");
        print_number(res_sum00);

        // 3 - small numbers that fit into high_digit: 5 + 7 = 12 (interior)
        printf("\nTest 3\n");
        Bigint* s1 = init(); assign_value(s1, "5");
        Bigint* s2 = init(); assign_value(s2, "7");
        sum_interior(s1, s2);
        printf("12\n");
        print_number(s1);

        // 4 - overflow high_digit -> create multi-word: 2147483647 + 1 (external)
        printf("\nTest 4\n");
        Bigint* biga = init(); assign_value(biga, "2147483647");
        Bigint* bigb = init(); assign_value(bigb, "1");
        Bigint* res_big = sum_external(biga, bigb);
        printf("2147483648\n");
        print_number(res_big);

        // 5 - negatives and sign handling (external): -5 + 3 = -2
        printf("\nTest 5\n");
        Bigint* n5 = init(); assign_value(n5, "-5");
        Bigint* p3 = init(); assign_value(p3, "3");
        Bigint* r53 = sum_external(n5, p3);
        printf("-2\n");
        print_number(r53);

        // 6 - both negative (interior): -10 + -20 = -30
        printf("\nTest 6\n");
        Bigint* n10 = init(); assign_value(n10, "-10");
        Bigint* n20 = init(); assign_value(n20, "-20");
        sum_interior(n10, n20);
        printf("-30\n");
        print_number(n10);

        // Subtraction tests using sub_interior and external-sub via negation + sum_external

        // 7 - sub_interior: 7 - 5 = 2
        printf("\nTest 7\n");
        Bigint* suba = init(); assign_value(suba, "7");
        Bigint* subb = init(); assign_value(subb, "5");
        sub_interior(suba, subb);   // suba := suba - subb
        printf("2\n");
        print_number(suba);

        // 8 - external subtraction (use negation + sum_external): 10000000000 - 1 = 9999999999
        printf("\nTest 8\n");
        Bigint* exa = init(); assign_value(exa, "10000000000");
        Bigint* exb = init(); assign_value(exb, "1");
        // negate exb for subtraction and use sum_external to get new Bigint*
        exb->high_digit |= SIGN_MASK_U;
        Bigint* res_sub_ext = sum_external(exa, exb);
        printf("9999999999\n");
        print_number(res_sub_ext);

        // Multiplication tests

        // 9 - mult_internal small: 6 * 7 = 42
        printf("\nTest 9\n");
        Bigint* m1 = init(); assign_value(m1, "6");
        Bigint* m2 = init(); assign_value(m2, "7");
        mult_internal(m1, m2);
        printf("42\n");
        print_number(m1);

        // 10 - mult_external with zero: 123456 * 0 = 0
        printf("\nTest 10\n");
        Bigint* mm1 = init(); assign_value(mm1, "123456");
        Bigint* mm0 = init(); assign_value(mm0, "0");
        Bigint* res_m0 = mult_external(mm1, mm0);
        printf("0\n");
        print_number(res_m0);

        // 11 - larger multiplication (internal) - original big test
        printf("\nTest 11\n");
        char* an = "744309584305832935";
        Bigint* nan = init();
        assign_value(nan, an);
        char* bn = "9512075892352375938395204";
        Bigint* nbn = init();
        assign_value(nbn, bn);
        mult_internal(nan, nbn);
        printf("7079929253322331804219415104297920429243740\n");
        print_number(nan);

        return SUCCESS;
}