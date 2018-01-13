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
#include <math.h>

/* A little macro magic to compute a bezier curve point
 * This is the parametric form of the bezier curve
 *
 * P0 -> start point of the bezier curve
 * P1 -> first control point
 * P2 -> second control point
 * P3 -> end point of the bezier curve
 * c  -> coordinate (x or y)
 * t  -> parameter (must be in [0,1]
 */
#define BEZIER_POINT(P0, P1, P2, P3, c, t)                                     \
    P0.c *pow((1 - t), 3) + 3 * P1.c *t *pow((1 - t), 2) +                     \
        3 * P2.c *pow(t, 2) * (1 - t) + P3.c *pow(t, 3)

/* Resolution of the bezier curve transformation (number of segments used to
 * approximate the bezier curve)
 */
#define BEZIER_RES 100

void fi_point_draw_d(FI_POINT_D pt, FILE *out) {
    fprintf(out, "%.4f,%.4f ", pt.x, pt.y);
}

int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode) {
    return 0;
}

void fi_free_path(FI_PATH *path) {
    FI_PATH *tmp = NULL;
    FI_BOUND *bound = NULL;
    if (path != NULL) {
        bound = path->bound;
    }
    while (path != NULL) {
        tmp = path;
        path = path->next;
        if (tmp->section.points != NULL)
            free(tmp->section.points);
        free(tmp);
    }
    if (bound != NULL)
        free(bound);
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
                    out_current->bound->last->section.points[pc].x = coord;
                    is_x = 0;
                } else {
                    out_current->bound->last->section.points[pc].y = coord;
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
        case '-':
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
            fi_free_path(out_current);
            return 1;
            break;
        }
    }
    (*out) = out_current;
    return 0;
}

void fi_start_svg_doc(FILE *out, double width, double height) {
    fprintf(out,
            "<?xml version=\"1.0\"  encoding=\"UTF-8\" standalone=\"no\"?>\n");
    fprintf(out, "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
                 "xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"%.4f\" "
                 "height=\"%.4f\">\n",
            width, height);
    return;
}

void fi_end_svg_doc(FILE *out) {
    fprintf(out, "</svg>\n");
    return;
}

void fi_start_svg_path(FILE *out) {
    fprintf(out, "<path d=\"");
    return;
}

void fi_end_svg_path(FILE *out, double stroke_width, char *stroke_color,
                     char *fill_color, char *fill_opacity) {
    fprintf(out, "\" stroke-width=\"%.4fpx\" ", stroke_width);
    if (stroke_color)
        fprintf(out, "stroke=\"%s\" ", stroke_color);
    if (fill_color)
        fprintf(out, "fill=\"%s\" ", fill_color);
    if (fill_opacity)
        fprintf(out, "fill-opacity=\"%s\" ", fill_opacity);
    fprintf(out, " />\n");
    return;
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

// converts one arc to segments
void fi_arc_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out) {
    fi_add_new_seg(out, FI_SEG_LINE);
    (*out)->section.points[0].x = in[1].x;
    (*out)->section.points[0].y = in[1].y;
    return;
}

// converts one cubic bezier curve to segments
void fi_bezier_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out) {
    FI_POINT_D P0 = ref;
    FI_POINT_D P1 = in[0];
    FI_POINT_D P2 = in[1];
    FI_POINT_D P3 = in[2];

    for (int i = 0; i < BEZIER_RES; i++) {
        fi_add_new_seg(out, FI_SEG_LINE);
        double t = (double)i / BEZIER_RES;
        (*out)->bound->last->section.points[0].x =
            BEZIER_POINT(P0, P1, P2, P3, x, t);
        (*out)->bound->last->section.points[0].y =
            BEZIER_POINT(P0, P1, P2, P3, y, t);
    }
    // fi_draw_path(*out, stdout);
    return;
}

void fi_linearize(FI_PATH **in) {
    FI_PATH *tmp = *in;
    FI_POINT_D last_ref_point;
    last_ref_point.x = 0;
    last_ref_point.y = 0;

    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        FI_POINT_D *pt = tmp->section.points;
        FI_PATH *new_seg = NULL;
        FI_PATH *next = tmp->next;
        bool is_first = false;
        if (tmp->prev == NULL)
            is_first = true;
        switch (type) {
        case FI_SEG_END:
            last_ref_point.x = 0;
            last_ref_point.y = 0;
            break;
        case FI_SEG_MOVE:
            last_ref_point.x = pt[0].x;
            last_ref_point.y = pt[0].y;
            break;
        case FI_SEG_LINE:
            last_ref_point.x = pt[0].x;
            last_ref_point.y = pt[0].y;
            break;
        case FI_SEG_ARC:
            // FIXME
            // It's probably false but it doesn't crash too much
            fi_arc_to_lines(last_ref_point, pt, &new_seg);
            last_ref_point.x = pt[1].x;
            last_ref_point.y = pt[1].y;
            fi_replace_path(&tmp, new_seg);
            break;
        case FI_SEG_BEZIER:
            fi_bezier_to_lines(last_ref_point, pt, &new_seg);
            last_ref_point.x = pt[2].x;
            last_ref_point.y = pt[2].y;
            fi_replace_path(&tmp, new_seg);
            break;
        }
        if (is_first)
            *in = tmp;
        tmp = next;
    }
}

void fi_replace_path(FI_PATH **old, FI_PATH *new) {
    FI_BOUND *tmp_bound = new->bound;
    FI_PATH *old_tmp = *old;

    // old is the first point
    if (old_tmp->prev == NULL) {
        old_tmp->bound->first = new;
        *old = new;
    } else {
        old_tmp->prev->next = new;
        new->prev = old_tmp->prev;
    }
    // old is the last point
    if (old_tmp->next == NULL) {
        old_tmp->bound->last = new->bound->last;
    } else {
        old_tmp->next->prev = new->bound->last;
        new->bound->last->next = old_tmp->next;
    }

    // replace the boundaries of the inserted segment
    // by those of the host segment
    FI_PATH *tmp = new;
    while (tmp != tmp_bound->last) {
        tmp->bound = old_tmp->bound;
        tmp = tmp->next;
    }
    tmp->bound = old_tmp->bound;

    // isolate the old point
    old_tmp->next = NULL;
    old_tmp->bound = NULL;

    // free the old point
    fi_free_path(old_tmp);

    // free the new path bound (it's using the old one now)
    free(tmp_bound);
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
            out_current->bound->last->section.points[0] = pt[0];
            break;
        case FI_SEG_LINE:
            out_current->bound->last->section.points[0] = pt[0];
            break;
        case FI_SEG_ARC:
            out_current->bound->last->section.points[0] = pt[0];
            out_current->bound->last->section.points[1] = pt[1];
            break;
        case FI_SEG_BEZIER:
            out_current->bound->last->section.points[0] = pt[0];
            out_current->bound->last->section.points[1] = pt[1];
            out_current->bound->last->section.points[2] = pt[2];
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
    if (*path == NULL || (*path)->bound == NULL) {
        *path = new_path;
        (*path)->bound = calloc(sizeof(FI_BOUND), 1);
        new_path->bound->last = new_path;
        new_path->bound->first = new_path;
    } else {
        (*path)->bound->last->next = new_path;
        new_path->prev = (*path)->bound->last;
        (*path)->bound->last = new_path;
        new_path->bound = (*path)->bound;
    }
}
