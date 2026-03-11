#include "bigint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define SUCCESS 0
#define BASE (unsigned int)(sizeof(unsigned int) * 8)
#define BASE_BITS (8u * (unsigned)sizeof(unsigned int))
#define BASE_ULL (1ULL << BASE_BITS)
#define DIGIT_MASK ((unsigned long long)UINT_MAX)
#define SIGN_MASK_U ((unsigned int)(1u << (BASE_BITS - 1u)))
#define ABS_MASK_U ((unsigned int)(SIGN_MASK_U - 1u))

static void printBits(unsigned int number) {
    /* Display bits of a given number */
    for (int i = sizeof(number)*CHAR_BIT - 1; i >= 0; i--) {
        printf("%u", (number >> i) & 1);
        if (i % 8 == 0 && i != 0) printf(" ");
    }
}

static void small_multiply(Bigint* number, int value) {
    if (!number) return;

    // Base case of zero value
    if (value == 0) {
        number->high_digit = 0;
        free(number->digits);
        number->digits = NULL;
        return;
    }

    // Auxiliary variables
    unsigned int WORD_BITS = sizeof(unsigned int) * CHAR_BIT;
    unsigned int sign_mask = 1U << (WORD_BITS - 1);
    unsigned int abs_mask = sign_mask - 1;
    unsigned int sign = number->high_digit & sign_mask;

    // In case of empty array
    if (!number->digits) {
        unsigned long long tmp = (unsigned long long)(number->high_digit & abs_mask) * value;
        if (tmp < sign_mask) {
            number->high_digit = (unsigned int)tmp | sign;
            return;
        }

        unsigned int* temp_ptr = malloc(2 * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;

        number->digits[0] = 1;
        number->digits[1] = (unsigned int)tmp;
        number->high_digit = (unsigned int)(tmp >> WORD_BITS) | sign;
        return;
    }

    // In case of not empty array
    unsigned long long carry = 0;
    for (unsigned int i = 1; i <= number->digits[0]; i++) {
        unsigned long long temp = (unsigned long long)number->digits[i] * value + carry;
        number->digits[i] = (unsigned int)temp;
        carry = temp >> WORD_BITS;
    }

    unsigned long long temp = (unsigned long long)(number->high_digit & abs_mask) * value + carry;
    unsigned int new_lo = (unsigned int)temp;
    unsigned int new_carry = (unsigned int)(temp >> WORD_BITS);

    if (new_carry) {
        unsigned int nc = number->digits[0] + 1;
        unsigned int* temp_ptr = realloc(number->digits, (nc + 1) * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;
        number->digits[nc] = new_lo;
        number->digits[0] = nc;
        number->high_digit = new_carry | sign;

    } else if (new_lo >= sign_mask) {
        unsigned int nc = number->digits[0] + 1;
        unsigned int* temp_ptr = realloc(number->digits, (nc + 1) * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;
        number->digits[nc] = new_lo;
        number->digits[0] = nc;
        number->high_digit = sign;
    } else {
        number->high_digit = new_lo | sign;
    }
}

static void small_add(Bigint* number, int value) {
    if (!number) return;

    // Auxiliary variables
    unsigned int WORD_BITS=sizeof(unsigned int)*8;
    unsigned int sign_mask=1U << (WORD_BITS - 1);
    unsigned int abs_mask=sign_mask - 1;
    unsigned long long carry=value;

    // In case of empty array
    if (!number->digits) {
        unsigned int abs_high = number->high_digit & abs_mask;
        unsigned int sign = number->high_digit & sign_mask;

        unsigned long long tmp = (unsigned long long)abs_high + carry;
        if (tmp < sign_mask) {
                number->high_digit = (unsigned int)tmp | sign;
                return;
        }

        unsigned int* temp_ptr = malloc(2 * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;
        number->digits[0] = 1;
        number->digits[1] = (unsigned int)tmp;
        number->high_digit = (unsigned int)(tmp >> WORD_BITS) | sign;
    }

    unsigned int i = 1;
    while (carry && i <= number->digits[0]) {
        unsigned long long temp = (unsigned long long)number->digits[i] + carry;
        number->digits[i] = (unsigned int)temp;
        carry = temp >> (sizeof(unsigned int) * 8);
        i++;
    }

    if (carry) {
        unsigned int* temp_ptr = realloc(number->digits, (number->digits[0] + 2) * sizeof(unsigned int));
        if (!temp_ptr) return;
        number->digits = temp_ptr;
        number->digits[0]++;
        number->digits[number->digits[0]] = (unsigned int)carry;
    }
}

Bigint* init(void) {
    Bigint* number = (Bigint*)malloc(sizeof(Bigint));
    if (!number) return NULL;

    number->high_digit = 0;
    number->digits = NULL;
    return number;
}

void destroy(Bigint* number) {
    if (!number) return;

    free(number->digits);
    free(number);
}

void assign_value(Bigint* number, char* value) {
    if (!number || !value) return;

    char negative=0;
    int start=0;

    if (value[0]=='-') {
        negative=1;
        start=1;
    }

    for (int i = start; i < (int)strlen(value); i++) {
        small_multiply(number, 10);
        small_add(number, value[i] - '0');
    }

    if (number->digits && number->digits[0] > 0 && (number->high_digit & ABS_MASK_U) == 0) {
        unsigned int top = number->digits[number->digits[0]];
        if (top < SIGN_MASK_U) {
            number->high_digit = (int)top;
            number->digits[0]--;
            if (number->digits[0] == 0) {
                free(number->digits);
                number->digits = NULL;
            } else {
                unsigned int* temp_ptr = realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
                if (temp_ptr) number->digits = temp_ptr;
            }
        }
    }
    if (negative) number->high_digit |= SIGN_MASK_U;
}

static void normalize(Bigint* number) {
    unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));
    unsigned int max_number_i = ~sign_mask;

    // Base cases
    if ((number->high_digit & (sign_mask - 1)) != 0) return;
    if (!number->digits) return;

    // Process array elements
    int i = number->digits[0];
    int cnt = 0;
    while (i > 0 && number->digits[i] == 0) {
        cnt++;
        i--;
    }

    if ((i != 0) && (number->digits[i] <= max_number_i)) {
        int sign = number->high_digit & sign_mask;
        number->high_digit = number->digits[i];
        if (sign) number->high_digit |= sign_mask;
        cnt++;
    }

    number->digits[0] -= cnt;
    if (number->digits[0] == 0) {
        free(number->digits);
        number->digits = NULL;
        return;
    }

    unsigned int* temp_ptr = realloc(number->digits, (number->digits[0] + 1) * sizeof(unsigned int));
    if (!temp_ptr) return;
    number->digits = temp_ptr;
}

static unsigned int get_word_print(Bigint* number, unsigned int i) {
    if (number->digits) {
        if (i < number->digits[0]) return number->digits[i + 1u];
        if (i == number->digits[0]) return (unsigned int)number->high_digit & ABS_MASK_U;
    } else {
        if (i == 0u) return (unsigned int)number->high_digit & ABS_MASK_U;
    }
    return 0u;
}

void print_number(Bigint* number) {
    if (number == NULL) { printf("NULL\n"); return; }

    unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));

    if (!number->digits &&
        ((unsigned int)number->high_digit & ABS_MASK_U) == 0) {
                puts("0");
                return;
    }

    if (number->high_digit & sign_mask) putchar('-');

    unsigned int n = 1u;
    if (number->digits) n += number->digits[0];

    unsigned int *buffer = (unsigned int*)malloc(n * sizeof(unsigned int));
    if (!buffer) {
        printf("Memory error!\n");
        return;
    }

    for (unsigned int i = 0; i < n; i++)  {
        buffer[i] = get_word_print(number, i);
    }

    char digits[8192];
    int position = 0;

    while (1) {
        unsigned int topnz = n;
        while (topnz > 0 && buffer[topnz - 1u] == 0) topnz--;
        if (topnz == 0) break;

        unsigned long long remainder = 0;
        for (int i = (int)topnz - 1; i >= 0; i--) {
            unsigned long long current = remainder * BASE_ULL + buffer[i];
            buffer[i] = (unsigned int)(current / 10u);
            remainder = current % 10u;
        }
        digits[position++] = (char)('0' + (int)remainder);
        if (position >= (int)sizeof(digits) - 1) break;
    }

    free(buffer);
    for (int i = position - 1; i >= 0; i--) putchar(digits[i]);
    putchar('\n');
}

void number_debug(Bigint* number) {
    /* Technical output of number in Bigint type */
    if (!number) {
        printf("Number is not initialized!\n");
        return;
    }

    // High digit display
    if (number->high_digit>>(sizeof(int) * 8 - 1)) {
        printf("-%9.d(",number->high_digit-(1 << (sizeof(int) * 8 - 1)));
        printBits(number->high_digit);
        printf(")h ");
    } else {
        printf("%9.d(",number->high_digit);
        printBits(number->high_digit);
        printf(")h ");
    }

    // Digits array display
    if (!number->digits) printf("None");
    else {
        for(int i=number->digits[0];i>0;i--){
            printf("%10.u(",number->digits[i]);
            printBits(number->digits[i]);
            printf(") ");
        }
    }

    // Memory and content information display
    if (!number->digits) {
        printf("(Digits: 1) (Memory: %lu) Content: ", sizeof(int));
    } else {
        printf("(Digits: %u) (Memory: %lu) Content: ",
        number->digits[0] + 1, (number->digits[0] + 1) * sizeof(unsigned int) + sizeof(int));
    }
    print_number(number);
    printf("\n");
}

void sum_interior(Bigint* number1, Bigint* number2) {
    /* Internal (return nothing, result is placed in number1) sum of two Bigint numbers
    separately process two cases of numbers with different signs and with the same sign
    */
    if (!number1 || !number2) return;

    // Auxiliary variables
    unsigned int sign_mask = (1 << (sizeof(unsigned int) * 8 - 1));
    unsigned int max_number_i = ~sign_mask;
    unsigned int len1 = number1->digits ? number1->digits[0] : 0;
    unsigned int len2 = number2->digits ? number2->digits[0] : 0;
    unsigned int min_length = len1 < len2 ? len1 : len2;
    unsigned int carry = 0; long long unsigned summary = 0;

    // in case of the same sign just sum up values
    if ((number1->high_digit & sign_mask) == (number2->high_digit & sign_mask)) {

        for (unsigned int i = 1; i <= min_length; i++) {
            summary = (long long unsigned)number1->digits[i]
            + (long long unsigned)number2->digits[i] + carry;
            number1->digits[i] = (unsigned int)summary;
            carry = summary >> (sizeof(unsigned int) * 8);
        }

        // If first number is longer add high_digit of second number
        if (len1 > len2) {
            unsigned int i = min_length + 1;
            summary = (unsigned int)(number2->high_digit & max_number_i) + number1->digits[i] + carry;
            number1->digits[i] = (unsigned int)summary;
            carry = summary >> (sizeof(unsigned int) * 8);
            i++;

            while (carry && i <= len1) {
                summary = (long long unsigned)number1->digits[i] + carry;
                number1->digits[i] = (unsigned int)summary;
                carry = summary >> (sizeof(unsigned int) * 8);
                i++;
            }

            if (carry) {
                unsigned int abs_h = number1->high_digit & max_number_i;
                unsigned int sign = number1->high_digit & sign_mask;
                unsigned long long tmp = (unsigned long long)abs_h + carry;
                if (tmp < SIGN_MASK_U) {
                        number1->high_digit = (unsigned int)tmp | sign;
                } else {
                    unsigned int* temp_ptr = realloc(number1->digits, (len1 + 2) * sizeof(unsigned int));
                    if (!temp_ptr) return;
                    number1->digits = temp_ptr;
                    number1->digits[0] = len1 + 1;
                    number1->digits[number1->digits[0]] = (unsigned int)tmp;
                    number1->high_digit = (unsigned int)(tmp >> BASE_BITS) | sign;
                }
            }
        // If second number is longer add high_digit of first number
        } else if (len2 > len1) {
            unsigned int extra = len2 - min_length;
            unsigned int* temp_ptr = realloc(number1->digits, (len1 + extra + 1) * sizeof(unsigned int));
            if (!temp_ptr) return;
            number1->digits = temp_ptr;

            unsigned int i = min_length + 1;
            summary = (unsigned int)(number1->high_digit & max_number_i) + number2->digits[i] + carry;
            number1->digits[0] = len1 + extra;
            number1->digits[i] = (unsigned int)summary; carry = summary >> (sizeof(unsigned int) * 8); i++;

            while (i <= len2) {
                summary = (long long unsigned)number2->digits[i] + carry;
                number1->digits[i] = (unsigned int)summary;
                carry = summary >> (sizeof(unsigned int) * 8);
                i++;
            }

            unsigned int sign = number1->high_digit & sign_mask;
            unsigned int abs_h2 = number2->high_digit & max_number_i;
            unsigned long long tmp = (unsigned long long)abs_h2 + carry;
            if (tmp < SIGN_MASK_U) {
                number1->high_digit = (unsigned int)tmp | sign;
            } else {
                unsigned int new_len = number1->digits[0] + 1;
                unsigned int* temp_ptr = realloc(number1->digits, (new_len + 1) * sizeof(unsigned int));
                if (!temp_ptr) return;
                number1->digits = temp_ptr;
                number1->digits[0] = new_len;
                number1->digits[new_len] = (unsigned int)tmp;
                number1->high_digit = (unsigned int)(tmp >> BASE_BITS) | sign;
            }
        // If lengths are equal sum up digits
        } else {
            unsigned int sign = number1->high_digit & sign_mask;
            unsigned int abs1 = number1->high_digit & max_number_i, abs2 = number2->high_digit & max_number_i;

            summary = (unsigned long long)abs1 + abs2 + carry;
            unsigned int new_high = (unsigned int)summary;
            carry = summary >> (sizeof(unsigned int) * 8);

            if (carry || (new_high >= SIGN_MASK_U)) {
                unsigned int dlen = len1;
                unsigned int* temp_ptr = realloc(number1->digits, (dlen + 2) * sizeof(unsigned int));
                if (!temp_ptr) return;
                number1->digits = temp_ptr;
                number1->digits[0] = dlen + 1;
                number1->digits[dlen + 1] = new_high;
                number1->high_digit = (carry ? 1 : 0) | sign;
            } else {
                number1->high_digit = (int)new_high | sign;
            }
        }
    // In case of different signs
    } else {
        Bigint* great_number = NULL;
        Bigint* less_number = NULL;
        unsigned int abs1 = number1->high_digit & max_number_i;
        unsigned int abs2 = number2->high_digit & max_number_i;

        if (len1 > len2) {
                great_number = number1; less_number = number2;
        } else if (len2 > len1) {
                great_number = number2; less_number = number1;
        } else if (abs1 > abs2) {
                great_number = number1; less_number = number2;
        } else if (abs2 > abs1) {
                great_number = number2; less_number = number1;
        } else if (!number1->digits && !number2->digits) {
                great_number = number1; less_number = number2;
        } else {
            int i = number1->digits[0];
            while (i > 0 && number1->digits[i] == number2->digits[i]) i--;
            if (number1->digits[i] >= number2->digits[i]) {
                great_number = number1;
                less_number = number2;
            } else {
                great_number = number2;
                less_number = number1;
            }
        }

        unsigned int abs_great = great_number->high_digit & max_number_i;
        unsigned int abs_less = less_number->high_digit & max_number_i;
        unsigned int glen = great_number->digits ? great_number->digits[0] : 0;
        unsigned int llen = less_number->digits ? less_number->digits[0] : 0;

        // Subtraction of parts with the same length
        for (unsigned int i = 1; i <= min_length; i++) {
            if ((unsigned long long)great_number->digits[i] < (unsigned long long)less_number->digits[i] + carry) {
                summary = (1ULL << BASE)
                + (long long unsigned)great_number->digits[i]
                - (long long unsigned)less_number->digits[i] - carry;
                carry = 1;
            } else {
                summary = (long long unsigned)great_number->digits[i]
                - (long long unsigned)less_number->digits[i] - carry;
                carry = 0;
            }
            great_number->digits[i] = (unsigned int)summary;
        }

        // If greater number is longer the lesser
        if (glen > llen) {
            unsigned int i = min_length + 1;
            if ((unsigned long long)great_number->digits[i] < (unsigned long long)abs_less + carry) {
                summary = (1ULL << BASE) + (long long unsigned)great_number->digits[i]
                - (long long unsigned)abs_less - carry;
                carry = 1;
            } else {
                summary = (long long unsigned)great_number->digits[i]
                - (long long unsigned)abs_less - carry;
                carry = 0;
            }
            great_number->digits[i] = (unsigned int)summary; i++;
            while (carry && i <= (int)glen) {
                if (great_number->digits[i] < carry) {
                    summary = (1ULL << BASE) + (long long unsigned)great_number->digits[i] - carry;
                    carry = 1;
                } else {
                    summary = (long long unsigned)great_number->digits[i] - carry;
                    carry = 0;
                }
                great_number->digits[i] = (unsigned int)summary; i++;
            }
            if (carry) {
                unsigned int sign = great_number->high_digit & sign_mask;
                great_number->high_digit =
                ((great_number->high_digit & max_number_i) - carry) | sign;
            }

        // If length of greater and lesser numbers is equal
        } else {
            int high_sign = great_number->high_digit & sign_mask;
            unsigned int result = abs_great - abs_less - carry;
            great_number->high_digit = result;
            if (high_sign) great_number->high_digit |= sign_mask;
        }
        normalize(great_number);

        if (great_number == number2)
        {
            if (number1->digits)
            {
                free(number1->digits);
                number1->digits = NULL;
            }

            number1->high_digit = number2->high_digit;

            if (number2->digits)
            {
                unsigned int size = number2->digits[0] + 1;
                number1->digits = (unsigned int *)malloc(size * sizeof(unsigned int));
                if (number1->digits == NULL)
                {
                    number1->high_digit = 0;
                    return;
                }
                memcpy(number1->digits, number2->digits, size * sizeof(unsigned int));
            }
            else
            {
                number1->digits = NULL;
            }
        }
    }
}

Bigint* sum_external(Bigint* number1, Bigint* number2) {
    /* External sum of two Bigint numbers
    Use internal sum function to calculate the sum, copy the value to a new Bigint
    and return it */
    if (!number1 || !number2) return NULL;

    Bigint* result = init();
    if (!result) return NULL;

    result->high_digit = number1->high_digit;
    if (number1->digits) {
        unsigned int size = number1->digits[0] + 1;
        result->digits = malloc(size * sizeof(unsigned int));
        if (!result->digits) {
            free(result);
            return NULL;
        }
        memcpy(result->digits, number1->digits, size * sizeof(unsigned int));
    }
    sum_interior(result, number2);
    return result;
}

void sub_interior(Bigint* number1, Bigint* number2) {
    /* Internal (return nothing, result is placed in number1) subtraction of two Bigint numbers
    Change the sign of the second number to its opposite and call the sum function */
    if (!number1 || !number2) return;

    number2->high_digit = (number2->high_digit & SIGN_MASK_U)
    ? number2->high_digit & ABS_MASK_U : number2->high_digit | SIGN_MASK_U;
    sum_interior(number1, number2);
}

Bigint* sub_external(Bigint* number1, Bigint* number2) {
    /* External subtraction of two Bigint numbers
    Use internal sub function to calculate the difference, copy the value to a new Bigint
    and return it */
    if (!number1 || !number2) return NULL;

    Bigint* copy = init();
    if (!copy) return NULL;

    copy->high_digit = number2->high_digit ^ SIGN_MASK_U;
    if (number2->digits) {
        unsigned int size = number2->digits[0] + 1;
        copy->digits = malloc(size * sizeof(unsigned int));
        if (!copy->digits) {
            free(copy);
            return NULL;
        }
        memcpy(copy->digits, number2->digits, size * sizeof(unsigned int));
    }

    Bigint* result = sum_external(number1, copy);
    free(copy->digits);
    free(copy);
    return result;
}

static unsigned int loword(unsigned int number){
        /* Calculate low word of the number */
        return number & ((1 << (sizeof(unsigned int) << 2)) - 1);
}

static unsigned int hiword(unsigned int number){
        /* Calculate high word of the number */
        return number >> (sizeof(unsigned int) << 2);
}

static unsigned int get_word(Bigint* number, unsigned int index) {
    /* Get the word at the specified index from the Bigint number */
    if (number->digits) {
        if (index < number->digits[0]) return number->digits[index + 1u];
        if (index == number->digits[0]) return (unsigned int)number->high_digit & ABS_MASK_U;
        return 0u;
    }
    return index == 0 ?(unsigned int)number->high_digit & ABS_MASK_U : 0u;
}

Bigint* mult_external(Bigint* number1, Bigint* number2) {
    /* External multiplication of two Bigint numbers */
    if (!number1 || !number2) return NULL;

    // Calculating lengths
    unsigned int length1 = number1->digits ? (number1->digits[0] + 1u) : 1u;
    unsigned int length2 = number2->digits ? (number2->digits[0] + 1u) : 1u;

    Bigint* result = init();
    if (!result) return NULL;

    // Case of one number being equal to zero
    if ((!number1->digits && !(number1->high_digit & ABS_MASK_U)) || (!number2->digits && !(number2->high_digit & ABS_MASK_U))) {
        result->high_digit = 0;
        result->digits = NULL;
        return result;
    }

    unsigned int sign = ((number1->high_digit & SIGN_MASK_U) ^ (number2->high_digit & SIGN_MASK_U)) ? 1u : 0u;

    /* Allocate extra room for carries: length1 + length2 + 2 to be safe during propagation */
    unsigned int res_words = length1 + length2 + 2;
    uint64_t *res = (uint64_t*)calloc(res_words, sizeof(uint64_t));
    if (!res) {
        free(result);
        return NULL;
    }

    unsigned int HALF_BITS = (unsigned int)(sizeof(unsigned int) << 2);
    unsigned int WORD_BITS = (unsigned int)(sizeof(unsigned int) << 3);
    for (unsigned int i = 0; i < length1; ++i) {
        for (unsigned int j = 0; j < length2; ++j) {
            unsigned int a = get_word(number1, i);
            unsigned int b = get_word(number2, j);
            uint64_t product1 = (uint64_t)loword(a) * (uint64_t)loword(b);
            uint64_t product2 = (uint64_t)loword(a) * (uint64_t)hiword(b);
            uint64_t product3 = (uint64_t)hiword(a) * (uint64_t)loword(b);
            uint64_t product4 = (uint64_t)hiword(a) * (uint64_t)hiword(b);
            uint64_t middle = product2 + product3;

            // Calculate low and high parts
            uint64_t loword_product = product1 + ((middle & ((1ULL << HALF_BITS) - 1)) << HALF_BITS);
            uint64_t hiword_product = product4 + (middle >> HALF_BITS) + (loword_product >> WORD_BITS);
            loword_product &= (uint64_t)UINT_MAX;

            unsigned int k = i + j;
            res[k] += loword_product;

            /* safe: res_words was enlarged, so k+1 is within bounds */
            res[k + 1] += (res[k] >> WORD_BITS) + hiword_product;
            res[k] &= (uint64_t)UINT_MAX;

            unsigned int kn = k + 1;
            /* propagate while we can write to kn+1 (kn + 1 < res_words) */
            while (kn + 1 < res_words && res[kn] > (uint64_t)UINT_MAX) {
                res[kn + 1] += res[kn] >> WORD_BITS;
                res[kn] &= (uint64_t)UINT_MAX;
                kn++;
            }
        }
    }

    // Trim leading zeros
    int top = (int)res_words - 1;
    while (top > 0 && res[top] == 0u) top--;

    // Single digit result
    if (top == 0 && (res[0] < (uint64_t)ABS_MASK_U)) {
        result->high_digit = (int)res[0];
        result->digits = NULL;

    // Multiple digits result
    } else {
        unsigned int lower_count = (unsigned int)top;
        if (res[lower_count] > (uint64_t)ABS_MASK_U) {
            unsigned int *temp = realloc(result->digits, (lower_count + 2u) * sizeof(unsigned int));
            if (!temp) { free(res); free(result); return NULL; }
            result->digits = temp;
            result->digits[0] = lower_count + 1u;
            for (unsigned int p = 0; p <= lower_count; ++p) result->digits[p + 1u] = (unsigned int)res[p];
            result->high_digit = 0;
        } else {
            unsigned int *temp = realloc(result->digits, (lower_count + 1u) * sizeof(unsigned int));
            if (!temp) { free(res); free(result); return NULL; }
            result->digits = temp;
            result->digits[0] = lower_count;
            for (unsigned int p = 0; p < lower_count; ++p) result->digits[p + 1u] = (unsigned int)res[p];
            result->high_digit = (int)res[lower_count];
        }
    }
    free(res);

    if (sign) result->high_digit |= SIGN_MASK_U;
    return result;
}

void mult_internal(Bigint* number1, Bigint* number2) {
    /* Internal multiplication of two Bigint numbers
    Use external multiplication function to calculate the result
    and assign it to the first number */
    if (!number1 || !number2) return;

    Bigint* result = mult_external(number1, number2);
    if (!result) return;

    free(number1->digits);
    number1->high_digit = result->high_digit;
    if (result->digits) {
        unsigned int size = result->digits[0] + 1;
        number1->digits = malloc(size * sizeof(unsigned int));
        if (!number1->digits) return;
        memcpy(number1->digits, result->digits, size * sizeof(unsigned int));
    } else {
        number1->digits = NULL;
    }

    free(result->digits);
    free(result);
}

void shift_words(Bigint* number, unsigned int n) {
    /* Shift number for n digits placing 0 on arosed positions */
    if (!number || n == 0) return;

    unsigned int length = number->digits ? number->digits[0] : 0;

    unsigned int* temp = realloc(number->digits, (length + n + 1) * sizeof(unsigned int));
    if (!temp) return;
    number->digits = temp;

    for (int i = length; i >= 1; --i) {
        number->digits[i + n] = number->digits[i];
    }

    for (unsigned int i = 1; i <= n; ++i) {
        number->digits[i] = 0;
    }

    number->digits[0] = length + n;
}

static Bigint* clone_bigint(Bigint* src) {
    /* Clone of bigint */
    if (!src) return NULL;

    Bigint* dst = init();
    if (!dst) return NULL;
    dst->high_digit = src->high_digit;

    if (src->digits) {
        unsigned int size = src->digits[0] + 1;
        dst->digits = malloc(size * sizeof(unsigned int));
        if (!dst->digits) { free(dst); return NULL; }
        memcpy(dst->digits, src->digits, size * sizeof(unsigned int));
    } else {
        dst->digits = NULL;
    }

    return dst;
}

Bigint* Karatsuba_external(Bigint* number1, Bigint* number2) {
    if (number1  == NULL || number2 == NULL) return NULL;

    // Auto-zero if there is zero
    if ((!number1->digits && !(number1->high_digit & ABS_MASK_U)) 
        || (!number2->digits && !(number2->high_digit & ABS_MASK_U))) {
        Bigint* result = init();
        return result;
    }

    // Sign determination
    unsigned int sign = ((number1->high_digit & SIGN_MASK_U) ^ (number2->high_digit & SIGN_MASK_U)) ? 1u : 0u;

    unsigned int length1 = 0, length2 = 0, max_length;
    // First number length
    if (number1->digits) {
        length1 += number1->digits[0]; 
        if (number1->high_digit & ABS_MASK_U) {
            length1 += 1;
        }
    } else {
        if (number1->high_digit & ABS_MASK_U) length1 = 1;
        else length1 = 0;
    }
    // Second number length
    if (number2->digits) {
        length2 += number2->digits[0]; 
        if (number2->high_digit & ABS_MASK_U) {
            length2 += 1;
        }
    } else {
        if (number2->high_digit & ABS_MASK_U) length2 = 1; 
        else length2 = 0;
    }
    // Max length determination
    max_length = (length1 <= length2) ? length2 : length1;

    // Base case
    if (max_length == 1) return mult_external(number1, number2);

    // Make length even for uniform splitting
    if ((max_length % 2) != 0) max_length++;
    unsigned int half = max_length / 2;

    // Halve numbers
    // Number1 procession
    Bigint* number1_l = init();
    Bigint* number1_r = init();
    if (!number1_l || !number1_r) {
        destroy(number1_l); destroy(number1_r);
        return NULL;
    }

    number1_l->digits = (unsigned int*)calloc(half + 1, sizeof(unsigned int));
    number1_r->digits = (unsigned int*)calloc(half + 1, sizeof(unsigned int));
    if (!number1_l->digits || !number1_r->digits) {
        destroy(number1_l); destroy(number1_r);
        return NULL;
    }
    number1_l->digits[0] = half;
    number1_r->digits[0] = half;

    // Transfere elements
    for (unsigned int i = 1; i <= half; i++) {
        number1_r->digits[i] = get_word(number1, i - 1);
        number1_l->digits[i] = get_word(number1, i + half - 1);
    }
    normalize(number1_l);
    normalize(number1_r);

    // Number2 procession
    Bigint* number2_l = init();
    Bigint* number2_r = init();
    if (!number2_l || !number2_r) {
        destroy(number1_l); destroy(number1_r);
        destroy(number2_l); destroy(number2_r);
        return NULL;
    }

    number2_l->digits = (unsigned int*)calloc(half + 1, sizeof(unsigned int));
    number2_r->digits = (unsigned int*)calloc(half + 1, sizeof(unsigned int));
    if (!number2_l->digits || !number2_r->digits) {
        destroy(number1_l); destroy(number1_r);
        destroy(number2_l); destroy(number2_r);
        return NULL;
    }
    number2_l->digits[0] = half;
    number2_r->digits[0] = half;

    // Transfere elements
    for (unsigned int i = 1; i <= half; i++) {
        number2_r->digits[i] = get_word(number2, i - 1);
        number2_l->digits[i] = get_word(number2, i + half - 1);
    }
    normalize(number2_l);
    normalize(number2_r);

    // Recursivly compute products and sums
    Bigint* prob1 = Karatsuba_external(number1_l, number2_l);
    Bigint* prob2 = Karatsuba_external(number1_r, number2_r);

    Bigint* sum1 = sum_external(number1_l, number1_r);
    Bigint* sum2 = sum_external(number2_l, number2_r);

    destroy(number1_l); destroy(number1_r);
    destroy(number2_l); destroy(number2_r);

    Bigint* prob3 = Karatsuba_external(sum1, sum2);
    destroy(sum1); destroy(sum2);

    // Final calculation
    if (!prob1 || !prob2 || !prob3) {
        destroy(prob1); destroy(prob2); destroy(prob3); return NULL;
    }

    Bigint* temp = clone_bigint(prob1);
    if (!temp) { destroy(prob1); destroy(prob2); destroy(prob3); return NULL; }
    sub_interior(prob3, temp);
    destroy(temp);

    temp = clone_bigint(prob2);
    if (!temp) { destroy(prob1); destroy(prob2); destroy(prob3); return NULL; }
    sub_interior(prob3, temp);
    destroy(temp);

    // Bit-shifting multiplication
    if (prob1->digits || (prob1->high_digit & ABS_MASK_U)) {
        shift_words(prob1, max_length);
    }
    if (prob3->digits || (prob3->high_digit & ABS_MASK_U)) {
        shift_words(prob3, half);
    }

    // Summation
    sum_interior(prob1, prob2); destroy(prob2);
    sum_interior(prob1, prob3); destroy(prob3);

    normalize(prob1);
    if (prob1->digits == NULL && ((prob1->high_digit & ABS_MASK_U) == 0u)) {
        prob1->high_digit &= ABS_MASK_U;
    } else if (sign) {
        prob1->high_digit |= SIGN_MASK_U;
    } else {
        prob1->high_digit &= ABS_MASK_U;
    }

    return prob1;
}

void Karatsuba_interior(Bigint* number1, Bigint* number2) {
    /* Interior Karatsuba function that use external version to calculate value */
    if (number1 == NULL || number2 == NULL) return;

    Bigint* result = Karatsuba_external(number1, number2);
    if (!result) return;

    if (number1->digits) free(number1->digits);

    number1->high_digit = result->high_digit;
    number1->digits = result->digits;

    result->digits = NULL; 
    destroy(result);
}

Bigint* uint_to_bigint(unsigned int number) {
    /* Convert unsigned int number to bigint */
    Bigint* result = init();
    if (!result) return NULL;
    
    if (number <= ABS_MASK_U) {
        result->high_digit = number;
        return result;
    }
    
    unsigned int* temp_ptr = (unsigned int*)calloc(2, sizeof(unsigned int));
    if (!temp_ptr) { destroy(result); return NULL; }
    result->digits = temp_ptr;
    result->digits[0] = 1;
    result->digits[1] = number;
    return result;
}

int mask_bigint(Bigint* number, unsigned int n) {
    if (!number) return 1;

    unsigned int digits_shift = n / BASE;
    unsigned int remainder_digit = n % BASE;
    unsigned int high_value = number->high_digit & ABS_MASK_U; 
    unsigned int number_length
    = ((number->digits == NULL) ? 1 : number->digits[0] + (high_value != 0));
    unsigned int remainder_mask = (remainder_digit == 0) ? 0 : ((1u << remainder_digit) - 1);

    if (digits_shift >= number_length) return 0;
    
    if (number->digits == NULL) { 
        number->high_digit = high_value & remainder_mask; 
        return 0; 
    }
    
    if (number->digits[0] == digits_shift) {
        number->high_digit = high_value & remainder_mask;
        return 0;
    }
    
    number->digits[0] = digits_shift;
    number->high_digit = number->digits[digits_shift + 1] & remainder_mask;
    
    unsigned int* temp_ptr = (unsigned int*)realloc(number->digits, sizeof(unsigned int) * (digits_shift + 1));
    if (!temp_ptr) return 1;
    number->digits = temp_ptr;
    
    return 0;
}