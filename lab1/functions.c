#include "bigint.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


Bigint* first_function(unsigned int n, Bigint* (*mult)(Bigint*, Bigint*)) {
    /* Recieve natural number and calculate sum from 1 to n of (-1)^n * n! */
    if (n == 0) return NULL;
    Bigint* factorial = init();
    if (n % 2 == 0) return factorial;
    assign_value(factorial, "1");
    for (unsigned int i = 2; i <= n; i++) {
        Bigint* current_number = uint_to_bigint(i);
        Bigint* temp_result = mult(factorial, current_number);
        if (!temp_result) { destroy(factorial); destroy(current_number); return NULL; }
        factorial = temp_result;
        destroy(current_number);
    }

    return factorial;
}

Bigint* second_function(unsigned int n, Bigint* (*mult)(Bigint*, Bigint*)) {
    if (n == 0) return NULL;

    Bigint* result = uint_to_bigint(1u);
    Bigint* base = uint_to_bigint(115249u);
    unsigned int power = 4183;

    while (power != 0) {
        if (power & 1) {
            Bigint* temp = mult(result, base);
            if (!temp) { destroy(result); destroy(base); return NULL; }
            destroy(result);
            result = temp;
            
            if (mask_bigint(result, n)) { destroy(result); destroy(base); return NULL; }
        }

        if (power == 1) break;
        
        Bigint* temp = mult(base, base);
        if (!temp) { destroy(result); destroy(base); return NULL; }
        destroy(base);
        base = temp;
        
        if (mask_bigint(base, n)) { destroy(result); destroy(base); return NULL; }
        power >>= 1;
    }
    
    destroy(base);
    return result;
}

double compare_time(Bigint* (*function)(unsigned int n, Bigint* (*mult)(Bigint*, Bigint*))) {
    struct timespec start1, end1, start2, end2;

    clock_gettime(CLOCK_MONOTONIC, &start1);
    Bigint* res1 = function(100000, mult_external);
    clock_gettime(CLOCK_MONOTONIC, &end1);
    
    clock_gettime(CLOCK_MONOTONIC, &start2);
    Bigint* res2 = function(100000, Karatsuba_external);
    clock_gettime(CLOCK_MONOTONIC, &end2);
    
    double time_taken1 = (end1.tv_sec - start1.tv_sec) + (end1.tv_nsec - start1.tv_nsec) / 1e9;
    double time_taken2 = (end2.tv_sec - start2.tv_sec) + (end2.tv_nsec - start2.tv_nsec) / 1e9;
    double difference = time_taken1 - time_taken2;

    destroy(res1); destroy(res2);
    return difference;
}

void test_functions(void) {
    printf("\nMATH FUNCTIONS TESTS\n");

    printf("\nFUNCTION 1 TESTS\n");
    printf("\nTest 1\n"); {
        Bigint* r = first_function(0, Karatsuba_external);
        printf("NULL\n");
        print_number(r);
    }

    printf("\nTest 2\n"); {
        Bigint* r = first_function(6, Karatsuba_external);
        printf("0\n");
        print_number(r);
    }

    printf("\nTest 3\n"); {
        Bigint* r = first_function(7, Karatsuba_external);
        printf("5040\n");
        print_number(r);
    }

    printf("\nTest 4\n"); {
        Bigint* r = first_function(27, Karatsuba_external);
        printf("10888869450418352160768000000\n");
        print_number(r);
    }

    printf("\nFUNCTION 2 TESTS\n");
    printf("\nTest 5\n"); {
        Bigint* r = second_function(0, Karatsuba_external);
        printf("NULL\n");
        print_number(r);
    }

    printf("\nTest 6\n"); {
        Bigint* r = second_function(1, Karatsuba_external);
        printf("1\n");
        print_number(r);
    }

    printf("\nTest 7\n"); {
        Bigint* r = second_function(16, Karatsuba_external);
        printf("54097\n");
        print_number(r);
    }

    printf("\nTest 8\n"); {
        Bigint* r = second_function(160, Karatsuba_external);
        printf("1395949026112318731162903731117745318796059792209\n");
        print_number(r);
    }

    printf("\nMATH FUNCTIONS TESTS DONE\n");
}

int main(void) {
    test_functions();

    double diff1 = compare_time(first_function);
    printf("First function time comaprison: %.12lf\n", diff1);

    double diff2 = compare_time(second_function);
    printf("Second function time comaprison: %.12lf\n", diff2);


    return 0;
}