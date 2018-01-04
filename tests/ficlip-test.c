#define _POSIX_C_SOURCE 200809L

#include "ficlip.h"
#include <stdio.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

void test_parse() {
    FI_PATH *path;
    int ret = parse_path("M 0.0,1.1 L 10.0,23.5432 L 0.5,42.987 A 0.42,50 "
                         "49,10.2 C 50.2,0.567 40,10 5,5.69 Z",
                         &path);

    FILE *stream;
    char *out;
    size_t len;
    stream = open_memstream(&out, &len);
    if (stream == NULL) {
        printf("Failed to allocate output stream\n");
        return;
    }

    fi_draw_path(path, stream);
    // fi_draw_path(path, stdout);

    fflush(stream);
    fclose(stream);

    const char *expected = "M 0.0000,1.1000 L 10.0000,23.5432 L 0.5000,42.9870 "
                           "A 0.4200,50.0000 49.0000,10.2000 C 50.2000,0.5670 "
                           "40.0000,10.0000 5.0000,5.6900 Z ";
    CU_ASSERT_STRING_EQUAL(out, expected);
    CU_ASSERT(ret == 0);
    free(out);
}

void test_parse_fail() {
    FI_PATH *path;
    int ret = parse_path("MCRAP 0.0,1.1 L 10.0,23.5432 L 0.5,42.987 A 0.42,50 "
                         "49,10.2 C 50.2,0.567 40,10 5,5.69 Z",
                         &path);
    CU_ASSERT(ret == 1);
}

void test_empty() {
    return;
}

int main() {
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    // pSuite = CU_add_suite("FICLIP", init_suite1, clean_suite1);
    pSuite = CU_add_suite("FICLIP", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test of fi_draw_path()", test_parse)) ||
        (NULL == CU_add_test(pSuite, "test of fi_draw_path() (error)",
                             test_parse_fail)) ||
        (NULL == CU_add_test(pSuite, "place holder 2", test_empty))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    // CU_automated_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
