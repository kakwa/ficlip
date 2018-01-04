/* This file is part of the ficlip clipping library
 *
 * ficlip is licensed under MIT.
 *
 * Copyright 2017, Pierre-Francois Carpentier
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ficlip.h"

void fi_point_draw_d(FI_POINT_D pt, FILE *out) {
    fprintf(out, "%.4f,%.4f ", pt.x, pt.y);
}

int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode) {
    return 0;
}

void fi_free_path(FI_PATH *path) {
    if (path == NULL) {
        return;
    }
    FI_PATH *tmp1 = path;
    FI_PATH *tmp2 = path;
    while (tmp1 != NULL) {
        tmp1 = tmp1->next;
        if(tmp2->section.points != NULL)
            free(tmp2->section.points);
        free(tmp2);
        tmp2 = tmp1;
    }
}

/* really bad parser for path declaration, but will make life far easier for
 * testing
 */
int parse_path(char *in, FI_PATH **out) {
    *out = NULL;
    int i;
    char *n_start = NULL;
    size_t n_len = 0;
    bool is_x = 1;
    int pc = 0;

    FI_PATH *out_current = NULL;

    for (i = 0; i < strlen(in); i++) {
        switch (in[i]) {
        case ',':
        case ' ':
            if (pc < 3 && n_start != NULL && n_len != 0) {
                char *tmp = calloc(1, n_len + 1);
                strncpy(tmp, n_start, n_len);
                double coord = atof(tmp);
                free(tmp);
                n_start = NULL;
                n_len = 0;
                if (is_x) {
                    out_current->last->section.points[pc].x = coord;
                    is_x = 0;
                } else {
                    out_current->last->section.points[pc].y = coord;
                    is_x = 1;
                    pc++;
                }
            } else {
                n_start = 0;
            }
            break;
        case 'M':
            fi_add_new_seg(&out_current, FI_SEG_MOVE);
            pc = 0;
            break;
        case 'L':
            fi_add_new_seg(&out_current, FI_SEG_LINE);
            pc = 0;
            break;
        case 'A':
            fi_add_new_seg(&out_current, FI_SEG_ARC);
            pc = 0;
            break;
        case 'C':
            fi_add_new_seg(&out_current, FI_SEG_BEZIER);
            pc = 0;
            break;
        case 'Z':
            fi_add_new_seg(&out_current, FI_SEG_END);
            pc = 0;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            if (n_start == NULL) {
                n_start = &in[i];
            }
            n_len++;
            break;
        default:
            return 1;
            break;
        }
    }
    (*out) = out_current;
    return 0;
}

void fi_draw_path(FI_PATH *in, FILE *out) {
    FI_PATH *tmp = in;
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
        tmp = tmp->next;
    }
}

void fi_arc_to_lines(FI_PATH *in, FI_PATH **out) {
    return;
}

void fi_bezier_to_lines(FI_PATH *in, FI_PATH **out) {
    return;
}

void fi_copy_path(FI_PATH *in, FI_PATH **out) {
    FI_PATH *tmp = in;
    FI_PATH *out_current = NULL;
    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        FI_POINT_D *pt = tmp->section.points;
        fi_add_new_seg(&out_current, type);
        switch (type) {
        case FI_SEG_END:
            break;
        case FI_SEG_MOVE:
            out_current->last->section.points[0] = pt[0];
            break;
        case FI_SEG_LINE:
            out_current->last->section.points[0] = pt[0];
            break;
        case FI_SEG_ARC:
            out_current->last->section.points[0] = pt[0];
            out_current->last->section.points[1] = pt[1];
            break;
        case FI_SEG_BEZIER:
            out_current->last->section.points[0] = pt[0];
            out_current->last->section.points[1] = pt[1];
            out_current->last->section.points[2] = pt[2];
            break;
        }
        tmp = tmp->next;
    }
    (*out) = out_current;
}

void fi_offset_path(FI_PATH *in, FI_POINT_D pt) {
    FI_PATH *tmp = in;
    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        switch (type) {
        case FI_SEG_END:
            break;
        case FI_SEG_MOVE:
            tmp->section.points[0].x += pt.x;
            tmp->section.points[0].y += pt.y;
            break;
        case FI_SEG_LINE:
            tmp->section.points[0].x += pt.x;
            tmp->section.points[0].y += pt.y;
            break;
        case FI_SEG_ARC:
            tmp->section.points[1].x += pt.x;
            tmp->section.points[1].y += pt.y;
            break;
        case FI_SEG_BEZIER:
            tmp->section.points[2].x += pt.x;
            tmp->section.points[2].y += pt.y;
            break;
        }
        tmp = tmp->next;
    }
}

void fi_add_new_seg(FI_PATH **path, FI_SEG_TYPE type) {
    FI_PATH *new_path = calloc(1, sizeof(FI_PATH));
    FI_POINT_D *new_seg;
    switch (type) {
    case FI_SEG_END:
        new_seg = NULL;
        break;
    case FI_SEG_MOVE:
        new_seg = calloc(1, sizeof(FI_POINT_D));
        break;
    case FI_SEG_LINE:
        new_seg = calloc(1, sizeof(FI_POINT_D));
        break;
    case FI_SEG_ARC:
        new_seg = calloc(2, sizeof(FI_POINT_D));
        break;
    case FI_SEG_BEZIER:
        new_seg = calloc(3, sizeof(FI_POINT_D));
        break;
    default:
        new_seg = NULL;
        break;
    }
    new_path->section.points = new_seg;
    new_path->section.type = type;
    if (*path == NULL || (*path)->last == NULL) {
        *path = new_path;
        new_path->last = new_path;
    } else {
        (*path)->last->next = new_path;
        (*path)->last = new_path;
    }
}