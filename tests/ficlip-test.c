#define _POSIX_C_SOURCE 200809L

#include "ficlip.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <CUnit/Automated.h>
#include <CUnit/CUCurses.h>

static char args_doc[] = "[-b] [-a] [-i] [-c]";

static char doc[] = "\nUnit Tests";

static struct argp_option options[] = {
    {"basic", 'b', 0, 0, "Run in basic mode (default)"},
    {"automated", 'a', 0, 0, "Run in automated mode (with xml)"},
    {"interactive", 'i', 0, 0, "Run in interactive mode (console)"},
    {"curses", 'c', 0, 0, "Run in interactive mode (ncurses)"},
    {0}};

struct arguments {
    char *args[2]; /* arg1 & arg2 */
    bool basic;
    bool automated;
    bool interactive;
    bool curses;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key) {
    case 'b':
        arguments->basic = 1;
        break;
    case 'a':
        arguments->automated = 1;
        break;
    case 'i':
        arguments->interactive = 1;
        break;
    case 'c':
        arguments->curses = 1;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

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
    fi_free_path(path);
}

void test_copy() {
    FI_PATH *in;
    int ret = parse_path("M 0.0,1.1 L 10.0,23.5432 L 0.5,42.987 A 0.42,50 "
                         "49,10.2 C 50.2,0.567 40,10 5,5.69 Z",
                         &in);
    CU_ASSERT(ret == 0);
    FI_PATH *out;

    fi_copy_path(in, &out);

    FILE *stream;
    char *sout;
    char *sin;
    size_t len;

    stream = open_memstream(&sout, &len);
    fi_draw_path(in, stream);
    fflush(stream);fclose(stream);

    stream = open_memstream(&sin, &len);
    fi_draw_path(out, stream);
    fflush(stream); fclose(stream);

    CU_ASSERT_STRING_EQUAL(out, in);

    free(sout);
    fi_free_path(out);
    free(sin);
    fi_free_path(in);
}

void test_parse_fail() {
    FI_PATH *path;
    int ret = parse_path("MCRAP 0.0,1.1 L 10.0,23.5432 L 0.5,42.987 A 0.42,50 "
                         "49,10.2 C 50.2,0.567 40,10 5,5.69 Z",
                         &path);
    CU_ASSERT(ret == 1);
    fi_free_path(path);
}

void test_empty() {
    return;
}

int main(int argc, char **argv) {
    struct arguments args = {0};
    argp_parse(&argp, argc, argv, 0, 0, &args);

    if (!(args.basic || args.automated || args.interactive || args.curses))
        args.basic = 1;

    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    // pSuite = CU_add_suite("FICLIP", init_suite1, clean_suite1);
    pSuite = CU_add_suite("path utils", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test of fi_draw_path()", test_parse)) ||
        (NULL == CU_add_test(pSuite, "test of fi_draw_path() (error)",
                             test_parse_fail)) ||
        (NULL == CU_add_test(pSuite, "fi_copy_path", test_copy))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("convert to segments", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "place holder 2", test_empty)) ||
        (NULL == CU_add_test(pSuite, "place holder 3", test_empty))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("clipping", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "place holder 4", test_empty)) ||
        (NULL == CU_add_test(pSuite, "place holder 5", test_empty))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    if (args.basic)
        CU_basic_run_tests();
    if (args.automated)
        CU_automated_run_tests();
    if (args.interactive)
        CU_console_run_tests();
    if (args.curses)
        CU_curses_run_tests();

    // int ret = CU_get_error();
    int ret = CU_get_number_of_failures();
    CU_cleanup_registry();
    return ret;
}
