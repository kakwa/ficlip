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

void test_reverse() {
    FI_PATH *path;
    int ret = parse_path("M 1,1 L 2,2 L 3,3 A 4,4 "
                         "5,5 C 6,6 7,7 8,8 Z",
                         &path);

    FI_PATH *tmp = path->bound->last;

    FILE *out;
    char *sout;
    size_t len;
    out = open_memstream(&sout, &len);
    if (out == NULL) {
        printf("Failed to allocate output stream\n");
        return;
    }

    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        FI_POINT_D *pt = tmp->section.points;
        switch (type) {
        case FI_SEG_END:
            fprintf(out, "Z ");
            break;
        case FI_SEG_MOVE:
            fprintf(out, "M ");
            fi_point_draw_d(pt[0], out);
            break;
        case FI_SEG_LINE:
            fprintf(out, "L ");
            fi_point_draw_d(pt[0], out);
            break;
        case FI_SEG_ARC:
            fprintf(out, "A ");
            fi_point_draw_d(pt[0], out);
            fi_point_draw_d(pt[1], out);
            break;
        case FI_SEG_BEZIER:
            fprintf(out, "C ");
            fi_point_draw_d(pt[0], out);
            fi_point_draw_d(pt[1], out);
            fi_point_draw_d(pt[2], out);
            break;
        }
        tmp = tmp->prev;
    }
    fflush(out);
    fclose(out);

    const char *expected = "Z C 6.0000,6.0000 7.0000,7.0000 8.0000,8.0000 A "
                           "4.0000,4.0000 5.0000,5.0000 L 3.0000,3.0000 L "
                           "2.0000,2.0000 M 1.0000,1.0000 ";
    CU_ASSERT(ret == 0);
    CU_ASSERT_STRING_EQUAL(sout, expected);
    free(sout);
    fi_free_path(path);
}

void test_bound() {
    FI_PATH *path;
    int ret = parse_path(
        "C 147.41071,143.54167 96.761905,262.98214 "
        "63.499999,170 C 30.238094,77.017855 -6.047619,95.160713 "
        "30.238094,77.017855 C 66.523808,58.875001 97.517856,108.76785 "
        "60.47619,82.309522 C 23.434524,55.851189 79.374999,35.440477 "
        "79.374999,35.440477 ",
        &path);

    FI_PATH *first;
    FI_PATH *last;
    FI_PATH *tmp;
    FI_BOUND *bound;

    // get the last point by iteration
    tmp = path;
    while (tmp != NULL) {
        last = tmp; 
        tmp = tmp->next;
    }
    // remember the first point
    first = path;

    tmp = path;
    bound = path->bound;
    while (tmp != NULL) {
        // check that bound is always the same and points to the correct points
        CU_ASSERT_PTR_EQUAL(tmp->bound, bound);
        CU_ASSERT_PTR_EQUAL(tmp->bound->first, first);
        CU_ASSERT_PTR_EQUAL(tmp->bound->last, last);
        tmp = tmp->next;
    }

    fi_linearize(&path);

    // get the last point by iteration
    tmp = path;
    while (tmp != NULL) {
        last = tmp; 
        tmp = tmp->next;
    }
    // remember the first point
    first = path;

    tmp = path;
    bound = path->bound;
    while (tmp != NULL) {
        // check that bound is always the same and points to the correct points
        CU_ASSERT_PTR_EQUAL(tmp->bound, bound);
        CU_ASSERT_PTR_EQUAL(tmp->bound->first, first);
        CU_ASSERT_PTR_EQUAL(tmp->bound->last, last);
        tmp = tmp->next;
    }

    CU_ASSERT(ret == 0);
    fi_free_path(path);
}

void test_bezier2seg() {
    FI_PATH *path;
    int ret = parse_path(
        "M 86.934523,78.529761 C 147.41071,143.54167 96.761905,262.98214 "
        "63.499999,170 C 30.238094,77.017855 -6.047619,95.160713 "
        "30.238094,77.017855 C 66.523808,58.875001 97.517856,108.76785 "
        "60.47619,82.309522 C 23.434524,55.851189 79.374999,35.440477 "
        "79.374999,35.440477 Z",
        &path);

    FILE *stream;
    char *out;
    size_t len;
    stream = open_memstream(&out, &len);
    if (stream == NULL) {
        printf("Failed to allocate output stream\n");
        return;
    }

    fi_start_svg_doc(stream, 300, 300);
    fi_start_svg_path(stream);

    fi_draw_path(path, stream);

    fi_end_svg_path(stream, 2, "red", "none", NULL);

    fi_linearize(&path);

    fi_start_svg_path(stream);
    fi_draw_path(path, stream);
    fi_end_svg_path(stream, 1, "black", "none", NULL);

    fi_end_svg_doc(stream);
    // fi_draw_path(path, stdout);

    fflush(stream);
    fclose(stream);

    FILE *out_file = fopen("svg-test-bezier2seg.svg", "w+");
    fprintf(out_file, "%s", out);
    fclose(out_file);
    free(out);

    CU_ASSERT(ret == 0);
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
    fflush(stream);
    fclose(stream);

    stream = open_memstream(&sin, &len);
    fi_draw_path(out, stream);
    fflush(stream);
    fclose(stream);

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
        (NULL == CU_add_test(pSuite, "test reverse list", test_reverse)) ||

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
    if ((NULL == CU_add_test(pSuite, "test of bezier curve to segment",
                             test_bezier2seg)) ||
        (NULL == CU_add_test(pSuite, "test bound after convert", test_bound))) {
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
