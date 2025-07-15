#include "types.h"

u64 bits_to_bytes(u64 bits)
{
    return (bits+7)/8;
}


#if defined(UNIT_TESTS)
#include <stdio.h>
#include "../tests/minunit.h"
int tests_run = 0;

static char * test_bits_to_bytes()
{
    mu_assert("error, bits_to_bytes(0) != 0", bits_to_bytes(0) == 0);
    mu_assert("error, bits_to_bytes(1) != 1", bits_to_bytes(1) == 1);
    mu_assert("error, bits_to_bytes(2) != 1", bits_to_bytes(2) == 1);
    mu_assert("error, bits_to_bytes(3) != 1", bits_to_bytes(3) == 1);
    mu_assert("error, bits_to_bytes(4) != 1", bits_to_bytes(4) == 1);
    mu_assert("error, bits_to_bytes(5) != 1", bits_to_bytes(5) == 1);
    mu_assert("error, bits_to_bytes(6) != 1", bits_to_bytes(6) == 1);
    mu_assert("error, bits_to_bytes(7) != 1", bits_to_bytes(7) == 1);
    mu_assert("error, bits_to_bytes(8) != 1", bits_to_bytes(8) == 1);
    mu_assert("error, bits_to_bytes(9) != 2", bits_to_bytes(9) == 2);
    return 0;
}

static char * all_tests()
{
    mu_run_test(test_bits_to_bytes);
    return 0;
}

int main(int argc, char **argv)
{
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

#endif
