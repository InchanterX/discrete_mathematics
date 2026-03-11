#include "bigint.h"
#include <stdio.h>
#include <stdlib.h>

void test_init_assign(void) {
    printf("\nINIT & ASSIGN_VALUE TESTS\n");

    printf("\nA) init()\n");
    Bigint* t=init();
    number_debug(t);
    free(t);

    printf("\nB) assign_value(\"0\")\n");
    t=init();
    assign_value(t,"0");
    printf("print_number: ");
    print_number(t);
    number_debug(t);
    free(t->digits);
    free(t);

    printf("\nC) assign_value(\"5\")\n");
    t=init();
    assign_value(t,"5");
    printf("print_number: ");
    print_number(t);
    number_debug(t);
    free(t->digits);
    free(t);

    printf("\nD) assign_value(\"-5\")\n");
    t=init();
    assign_value(t,"-5");
    printf("print_number: ");
    print_number(t);
    number_debug(t);
    free(t->digits);
    free(t);

    printf("\nE) assign_value(\"4294967296\")\n");
    t=init();
    assign_value(t,"4294967296");
    printf("print_number: ");
    print_number(t);
    number_debug(t);
    free(t->digits);
    free(t);

    printf("\nF) assign_value long positive and negative\n");
    Bigint* a=init(); assign_value(a,"12345678901234567890");
    Bigint* b=init(); assign_value(b,"-9876543210987654321");
    printf("A: "); print_number(a); number_debug(a);
    printf("B: "); print_number(b); number_debug(b);
    free(a->digits); free(a); free(b->digits);
    free(b);
    printf("\nINIT & ASSIGN_VALUE TESTS DONE\n");
}

void test_arithmetics(void) {
    printf("\nSUMMATION, SUBSTRUCTION & MULTIPLICATION TESTS\n");

    printf("Test 1\n"); {
        Bigint* a=init(); assign_value(a,"0");
        Bigint* b=init(); assign_value(b,"0");
        sum_interior(a,b);
        printf("0\n");
        print_number(a);
    }

    printf("\nTest 2\n"); {
        Bigint* a=init(); assign_value(a,"0");
        Bigint* b=init(); assign_value(b,"0");
        Bigint* r=sum_external(a,b);
        printf("0\n");
        print_number(r);
    }

    printf("\nTest 3\n"); {
        Bigint* a=init(); assign_value(a,"5");
        Bigint* b=init(); assign_value(b,"7");
        sum_interior(a,b);
        printf("12\n");
        print_number(a);
    }

    printf("\nTest 4\n"); {
        Bigint* a=init(); assign_value(a,"2147483647");
        Bigint* b=init(); assign_value(b,"1");
        Bigint* r=sum_external(a,b);
        printf("2147483648\n");
        print_number(r);
    }

    printf("\nTest 5\n"); {
        Bigint* a=init(); assign_value(a,"-5");
        Bigint* b=init(); assign_value(b,"3");
        Bigint* r=sum_external(a,b);
        printf("-2\n");
        print_number(r);
    }

    printf("\nTest 6\n"); {
        Bigint* a=init(); assign_value(a,"-10");
        Bigint* b=init(); assign_value(b,"-20");
        sum_interior(a,b);
        printf("-30\n");
        print_number(a);
    }

    printf("\nTest 7\n"); {
        Bigint* a=init(); assign_value(a,"7");
        Bigint* b=init(); assign_value(b,"5");
        sub_interior(a,b);
        printf("2\n");
        print_number(a);
    }

    printf("\nTest 8\n"); {
        Bigint* a=init(); assign_value(a,"10000000000");
        Bigint* b=init(); assign_value(b,"-1");
        Bigint* r=sum_external(a,b);
        printf("9999999999\n");
        print_number(r);
    }

    printf("\nTest 9\n"); {
        Bigint* a=init(); assign_value(a,"6");
        Bigint* b=init(); assign_value(b,"7");
        mult_internal(a,b);
        printf("42\n");
        print_number(a);
    }

    printf("\nTest 10\n"); {
        Bigint* a=init(); assign_value(a,"123456");
        Bigint* b=init(); assign_value(b,"0");
        Bigint* r=mult_external(a,b);
        printf("0\n");
        print_number(r);
    }

    printf("\nTest 11\n"); {
        Bigint* a=init(); assign_value(a,"744309584305832935");
        Bigint* b=init(); assign_value(b,"9512075892352375938395204");
        mult_internal(a,b);
        printf("7079929253322331804219415104297920429243740\n");
        print_number(a);
    }

    printf("\nTest 12\n"); {
        Bigint* a=init(); assign_value(a,"74430958432567889879235305832935");
        Bigint* b=init(); assign_value(b,"9512075892352789235789235789235375938395204");
        sum_interior(a,b);
        printf("9512075892427220194221803679114611244228139\n");
        print_number(a);
    }

    printf("\nSUMMATION, SUBSTRUCTION & MULTIPLICATION TESTS DONE\n");
}

void test_karatsuba(void) {
    printf("\nKARATSUBA TESTS\n");

    printf("\nTest 1\n"); {
        Bigint* a = init(); assign_value(a, "0");
        Bigint* b = init(); assign_value(b, "0");
        Bigint* r = Karatsuba_external(a, b);
        printf("0\n");
        print_number(r);
    }

    printf("\nTest 2\n"); {
        Bigint* a = init(); assign_value(a, "0");
        Bigint* b = init(); assign_value(b, "9512075892352375938395204");
        Bigint* r = Karatsuba_external(a, b);
        printf("0\n");
        print_number(r);
    }

    printf("\nTest 3\n"); {
        Bigint* a = init(); assign_value(a, "7443");
        Bigint* b = init(); assign_value(b, "95120");
        Bigint* r = Karatsuba_external(a, b);
        printf("707978160\n");
        print_number(r);
    }

    printf("\nTest 4\n"); {
        Bigint* a = init(); assign_value(a, "744353434");
        Bigint* b = init(); assign_value(b, "95120");
        Bigint* r = Karatsuba_external(a, b);
        printf("70802898642080\n");
        print_number(r);
    }

    printf("\nTest 5\n"); {
        Bigint* a = init(); assign_value(a, "744309584305832935");
        Bigint* b = init(); assign_value(b, "9512075892352375938395204");
        Bigint* r = Karatsuba_external(a, b);
        printf("7079929253322331804219415104297920429243740\n");
        print_number(r);
    }

    printf("\nTest 6\n"); {
        Bigint* a = init(); assign_value(a, "-744309584305832935");
        Bigint* b = init(); assign_value(b, "9512075892352375938395204");
        Bigint* r = Karatsuba_external(a, b);
        printf("-7079929253322331804219415104297920429243740\n");
        print_number(r);
    }

    printf("\nTest 7\n"); {
        Bigint* a = init(); assign_value(a, "-744309584305832935");
        Bigint* b = init(); assign_value(b, "-9512075892352375938395204");
        Bigint* r = Karatsuba_external(a, b);
        printf("7079929253322331804219415104297920429243740\n");
        print_number(r);
    }

    printf("\nTest 8\n"); {
        Bigint* a = init(); assign_value(a, "546732983845545");
        Bigint* b = init(); assign_value(b, "0");
        Karatsuba_interior(a, b);
        printf("0\n");
        print_number(a);
    }

    printf("\nTest 9\n"); {
        Bigint* a = init(); assign_value(a, "546732983845545");
        Bigint* b = init(); assign_value(b, "-744309584305832935");
        Karatsuba_interior(a, b);
        printf("-406938599932365272314070614024575\n");
        print_number(a);
    }

    printf("\nKARATSUBA TESTS DONE\n");
}

int main(void) {
    test_init_assign();
    test_arithmetics();
    test_karatsuba();

    return SUCCESS;
}