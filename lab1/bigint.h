#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>

#define SUCCESS 0
#define BASE (unsigned int)(sizeof(unsigned int) * 8)
#define BASE_BITS (8u * (unsigned)sizeof(unsigned int))
#define BASE_ULL (1ULL << BASE_BITS)
#define DIGIT_MASK ((unsigned long long)UINT_MAX)
#define SIGN_MASK_U ((unsigned int)(1u << (BASE_BITS - 1u)))
#define ABS_MASK_U ((unsigned int)(SIGN_MASK_U - 1u))

typedef struct Bigint {
    int high_digit;
    unsigned int*
    digits;
} Bigint;

Bigint* init(void);
void destroy(Bigint* number);
void assign_value(Bigint* number, char* value);
void print_number(Bigint* number);
void number_debug(Bigint* number);
void sum_interior(Bigint* number1, Bigint* number2);
Bigint* sum_external(Bigint* number1, Bigint* number2);
void sub_interior(Bigint* number1, Bigint* number2);
Bigint* sub_external(Bigint* number1, Bigint* number2);
Bigint* mult_external(Bigint* number1, Bigint* number2);
void mult_internal(Bigint* number1, Bigint* number2);
void shift_words(Bigint* number, unsigned int n);
Bigint* Karatsuba_external(Bigint* number1, Bigint* number2);
void Karatsuba_interior(Bigint* number1, Bigint* number2);
Bigint* uint_to_bigint(unsigned int number);
int mask_bigint(Bigint* number, unsigned int n);

#endif