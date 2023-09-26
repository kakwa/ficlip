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

void fi_point_draw_d(FI_POINT_D pt, FILE *out) {
    fprintf(out, "%.4f,%.4f ", pt.x, pt.y);
}

/* really bad parser for path declaration, but will make life far easier for
 * testing
 */
int fi_parse_path(const char *in, int s_in, FI_PATH **out) {
    *out = NULL;
    int i;
    char *n_start = NULL;
    size_t n_len = 0;
    int pc = 0;
    int ret = 0;

    FI_PATH *out_current = NULL;
    FI_SEG_TYPE type = FI_SEG_END;

    for (i = 0; i < s_in; i++) {
        switch (in[i]) {
        case '\n':
        case ',':
        case ' ':
            if (n_start != NULL && n_len != 0) {
                char *tmp = calloc(1, n_len + 1);
                strncpy(tmp, n_start, n_len);
                double coord = atof(tmp);
                free(tmp);
                n_start = NULL;
                n_len = 0;
                switch (type) {
                case FI_SEG_END:
                    break;
                case FI_SEG_MOVE:
                    switch (pc) {
                    case 0:
                        out_current->meta->last->section.points[0].x = coord;
                        pc++;
                        break;
                    case 1:
                        out_current->meta->last->section.points[0].y = coord;
                        pc = 0;
                        break;
                    default:
                        ret = ERR_PARSING_FAIL;
                        break;
                    }
                    break;
                case FI_SEG_LINE:
                    switch (pc) {
                    case 0:
                        out_current->meta->last->section.points[0].x = coord;
                        pc++;
                        break;
                    case 1:
                        out_current->meta->last->section.points[0].y = coord;
                        pc = 0;
                        break;
                    default:
                        ret = ERR_PARSING_FAIL;
                        break;
                    }
                    break;
                case FI_SEG_ARC:
                    switch (pc) {
                    case 0:
                        out_current->meta->last->section.points[0].x = coord;
                        pc++;
                        break;
                    case 1:
                        out_current->meta->last->section.points[0].y = coord;
                        pc++;
                        break;
                    case 2:
                        out_current->meta->last->section.points[1].x = coord;
                        pc++;
                        break;
                    case 3:
                        if (coord)
                            out_current->meta->last->section.flag |=
                                FI_LARGE_ARC;
                        pc++;
                        break;
                    case 4:
                        if (coord)
                            out_current->meta->last->section.flag |= FI_SWEEP;
                        pc++;
                        break;
                    case 5:
                        out_current->meta->last->section.points[2].x = coord;
                        pc++;
                        break;
                    case 6:
                        out_current->meta->last->section.points[2].y = coord;
                        pc = 0;
                        break;
                    default:
                        ret = ERR_PARSING_FAIL;
                        break;
                    }
                    break;
                case FI_SEG_QUA_BEZIER:
                    switch (pc) {
                    case 0:
                        out_current->meta->last->section.points[0].x = coord;
                        pc++;
                        break;
                    case 1:
                        out_current->meta->last->section.points[0].y = coord;
                        pc++;
                        break;
                    case 2:
                        out_current->meta->last->section.points[1].x = coord;
                        pc++;
                        break;
                    case 3:
                        out_current->meta->last->section.points[1].y = coord;
                        pc = 0;
                        break;
                    default:
                        ret = ERR_PARSING_FAIL;
                        break;
                    }
                    break;
                case FI_SEG_CUB_BEZIER:
                    switch (pc) {
                    case 0:
                        out_current->meta->last->section.points[0].x = coord;
                        pc++;
                        break;
                    case 1:
                        out_current->meta->last->section.points[0].y = coord;
                        pc++;
                        break;
                    case 2:
                        out_current->meta->last->section.points[1].x = coord;
                        pc++;
                        break;
                    case 3:
                        out_current->meta->last->section.points[1].y = coord;
                        pc++;
                        break;
                    case 4:
                        out_current->meta->last->section.points[2].x = coord;
                        pc++;
                        break;
                    case 5:
                        out_current->meta->last->section.points[2].y = coord;
                        pc = 0;
                        break;
                    default:
                        ret = ERR_PARSING_FAIL;
                        break;
                    }
                    break;
                }
            } else {
                n_start = NULL;
                n_len = 0;
            }
            break;
        case 'M':
            ret = fi_append_new_seg(&out_current, FI_SEG_MOVE);
            type = FI_SEG_MOVE;
            pc = 0;
            break;
        case 'L':
            ret = fi_append_new_seg(&out_current, FI_SEG_LINE);
            type = FI_SEG_LINE;
            pc = 0;
            break;
        case 'A':
            ret = fi_append_new_seg(&out_current, FI_SEG_ARC);
            type = FI_SEG_ARC;
            pc = 0;
            break;
        case 'Q':
            ret = fi_append_new_seg(&out_current, FI_SEG_QUA_BEZIER);
            type = FI_SEG_QUA_BEZIER;
            pc = 0;
            break;
        case 'C':
            ret = fi_append_new_seg(&out_current, FI_SEG_CUB_BEZIER);
            type = FI_SEG_CUB_BEZIER;
            pc = 0;
            break;
        case 'Z':
            ret = fi_append_new_seg(&out_current, FI_SEG_END);
            type = FI_SEG_END;
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
                n_start = (char *)&in[i];
            }
            n_len++;
            break;
        default:
            ret = ERR_PARSING_FAIL;
            break;
        }
        if (ret) {
            fi_free_path(out_current);
            return ret;
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
        FI_SEG_TYPE flag = tmp->section.flag;
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
            fprintf(out, "%.4f ", pt[0].x);
            fprintf(out, "%.4f ", pt[0].y);
            fprintf(out, "%.4f ", pt[1].x);
            if (flag & FI_LARGE_ARC)
                fprintf(out, "1 ");
            else
                fprintf(out, "0 ");
            if (flag & FI_SWEEP)
                fprintf(out, "1 ");
            else
                fprintf(out, "0 ");
            fi_point_draw_d(pt[2], out);
            break;
        case FI_SEG_QUA_BEZIER:
            fprintf(out, "Q ");
            fi_point_draw_d(pt[0], out);
            fi_point_draw_d(pt[1], out);
            break;

        case FI_SEG_CUB_BEZIER:
            fprintf(out, "C ");
            fi_point_draw_d(pt[0], out);
            fi_point_draw_d(pt[1], out);
            fi_point_draw_d(pt[2], out);
            break;
        }
        tmp = tmp->next;
    }
}
