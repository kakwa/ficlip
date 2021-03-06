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
#define CUB_BEZIER_POINT(P0, P1, P2, P3, c, t)                                 \
    P0.c *pow((1 - t), 3) + 3 * P1.c *t *pow((1 - t), 2) +                     \
        3 * P2.c *pow(t, 2) * (1 - t) + P3.c *pow(t, 3)

#define QUA_BEZIER_POINT(P0, P1, P2, c, t)                                     \
    P0.c *pow((1 - t), 2) + 2 * t *(1 - t) * P1.c + pow(t, 2) * P2.c

/* Resolution of the bezier curve transformation (number of segments used to
 * approximate the bezier curve)
 */
#define BEZIER_RES 100
#define ARC_RES 100

#define M_PI 3.14159265358979323846

// Degree to radian conversion ratio
#define D2R M_PI / 180.0
// Radian to degree conversion ratio
#define R2D 180.0 / M_PI

void fi_point_draw_d(FI_POINT_D pt, FILE *out) {
    fprintf(out, "%.4f,%.4f ", pt.x, pt.y);
}

int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode) {
    return 0;
}

void fi_free_path(FI_PATH *path) {
    FI_PATH *tmp = NULL;
    FI_META *meta = NULL;
    if (path != NULL) {
        meta = path->meta;
    }
    while (path != NULL) {
        tmp = path;
        path = path->next;
        if (tmp->section.points != NULL)
            free(tmp->section.points);
        free(tmp);
    }
    if (meta != NULL)
        free(meta);
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

double fi_angle_vect(FI_POINT_D a, FI_POINT_D b) {
    double ret;
    double sign = 1;
    ret = acos((a.x * b.x + a.y * b.y) / (sqrt(pow(a.x, 2) + pow(a.y, 2)) *
                                          sqrt(pow(b.x, 2) + pow(b.y, 2))));
    if ((a.x * b.y - a.y * b.x) < 0)
        sign = -1;
    return sign * ret;
}

// converts endpoints definition of arc to center defition for elliptic arc
FI_PARAM_ARC fi_arc_endpoint_to_center(FI_POINT_D s, FI_POINT_D e, FI_POINT_D r,
                                       double phi, FI_SEG_FLAG flag) {
    // https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
    int fa = 0;
    phi *= D2R;
    if (flag & FI_LARGE_ARC)
        fa = 1;
    int fs = 0;
    if (flag & FI_SWEEP)
        fs = 1;

    FI_PARAM_ARC ret = {0};

    FI_POINT_D p1;
    p1.x = cos(phi) * (s.x - e.x) / 2 + sin(phi) * (s.y - e.y) / 2;
    p1.y = -1.0 * sin(phi) * (s.x - e.x) / 2 + cos(phi) * (s.y - e.y) / 2;
    double delta = pow(p1.x, 2) / pow(r.x, 2) + pow(p1.y, 2) / pow(r.y, 2);
    if (delta > 1) {
        r.x = sqrt(delta) * r.x;
        r.y = sqrt(delta) * r.y;
    }

    double coef =
        sqrt((pow(r.x, 2) * pow(r.y, 2) - pow(r.x, 2) * pow(p1.y, 2) -
              pow(r.y, 2) * pow(p1.x, 2)) /
             (pow(r.x, 2) * pow(p1.y, 2) + pow(r.y, 2) * pow(p1.x, 2)));
    FI_POINT_D cp;
    double sign = 1;
    if (fa == fs)
        sign = -1;
    cp.x = sign * coef * r.x * p1.y / r.y;
    cp.y = sign * coef * r.y * p1.x / r.x * -1;

    ret.center.x = cos(phi) * cp.x - sin(phi) * cp.y + (s.x + e.x) / 2;
    ret.center.y = sin(phi) * cp.x + cos(phi) * cp.y + (s.y + e.y) / 2;

    FI_POINT_D t1;
    FI_POINT_D t2;

    t1.x = 1;
    t1.y = 0;
    t2.x = (p1.x - cp.x) / r.x;
    t2.y = (p1.y - cp.y) / r.y;
    ret.angle_s = fi_angle_vect(t1, t2);

    t1.x = (p1.x - cp.x) / r.x;
    t1.y = (p1.y - cp.y) / r.y;
    t2.x = (-p1.x - cp.x) / r.x;
    t2.y = (-p1.y - cp.y) / r.y;
    ret.angle_d = fi_angle_vect(t1, t2);
    if (fs == 0) {
        if (ret.angle_d > 0) {
            ret.angle_d -= 2 * M_PI;
        }
    } else {
        if (ret.angle_d < 0) {
            ret.angle_d += 2 * M_PI;
        }
    }

    ret.angle_s *= R2D;
    ret.angle_d *= R2D;
    ret.phi = phi * R2D;
    ret.radius.x = r.x;
    ret.radius.y = r.y;
    return ret;
}

// converts one arc to segments
void fi_arc_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_SEG_FLAG flag,
                     FI_PATH **out) {
    FI_POINT_D s = ref;
    FI_POINT_D r = in[0];
    FI_POINT_D e = in[2];
    double phi = in[1].x;
    if (r.x == 0 || r.y == 0) {
        fi_append_new_seg(out, FI_SEG_LINE);
        (*out)->section.points[0].x = e.x;
        (*out)->section.points[0].y = e.y;
        return;
    }
    FI_PARAM_ARC param = fi_arc_endpoint_to_center(s, e, r, phi, flag);
    double angle = 0;
    double cos_phi = cos(param.phi * D2R);
    double sin_phi = sin(param.phi * D2R);
    for (int i = 0; i <= ARC_RES; i++) {
        fi_append_new_seg(out, FI_SEG_LINE);
        angle = param.angle_s * D2R +
                (double)i / (double)ARC_RES * param.angle_d * D2R;
        FI_POINT_D t1;
        t1.x = cos(angle) * param.radius.x;
        t1.y = sin(angle) * param.radius.y;
        (*out)->meta->last->section.points[0].x =
            t1.x * cos_phi - t1.y * sin_phi + param.center.x;
        (*out)->meta->last->section.points[0].y =
            t1.x * sin_phi + t1.y * cos_phi + param.center.y;
    }
    return;
}

// converts one cubic bezier curve to segments
void fi_cub_bezier_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out) {
    FI_POINT_D P0 = ref;
    FI_POINT_D P1 = in[0];
    FI_POINT_D P2 = in[1];
    FI_POINT_D P3 = in[2];

    for (int i = 0; i <= BEZIER_RES; i++) {
        fi_append_new_seg(out, FI_SEG_LINE);
        double t = (double)i / BEZIER_RES;
        (*out)->meta->last->section.points[0].x =
            CUB_BEZIER_POINT(P0, P1, P2, P3, x, t);
        (*out)->meta->last->section.points[0].y =
            CUB_BEZIER_POINT(P0, P1, P2, P3, y, t);
    }
    // fi_draw_path(*out, stdout);
    return;
}

void fi_qua_bezier_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out) {
    FI_POINT_D P0 = ref;
    FI_POINT_D P1 = in[0];
    FI_POINT_D P2 = in[1];

    for (int i = 0; i <= BEZIER_RES; i++) {
        fi_append_new_seg(out, FI_SEG_LINE);
        double t = (double)i / BEZIER_RES;
        (*out)->meta->last->section.points[0].x =
            QUA_BEZIER_POINT(P0, P1, P2, x, t);
        (*out)->meta->last->section.points[0].y =
            QUA_BEZIER_POINT(P0, P1, P2, y, t);
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
        FI_SEG_FLAG flag = tmp->section.flag;
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
            fi_arc_to_lines(last_ref_point, pt, flag, &new_seg);
            last_ref_point.x = pt[1].x;
            last_ref_point.y = pt[1].y;
            fi_replace_path(&tmp, new_seg);
            break;
        case FI_SEG_QUA_BEZIER:
            fi_qua_bezier_to_lines(last_ref_point, pt, &new_seg);
            last_ref_point.x = pt[1].x;
            last_ref_point.y = pt[1].y;
            fi_replace_path(&tmp, new_seg);
            break;
        case FI_SEG_CUB_BEZIER:
            fi_cub_bezier_to_lines(last_ref_point, pt, &new_seg);
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
    FI_META *tmp_meta = new->meta;
    FI_PATH *old_tmp = *old;

    // old is the first point
    if (old_tmp->prev == NULL) {
        old_tmp->meta->first = new;
        *old = new;
    } else {
        old_tmp->prev->next = new;
        new->prev = old_tmp->prev;
    }
    // old is the last point
    if (old_tmp->next == NULL) {
        old_tmp->meta->last = new->meta->last;
    } else {
        old_tmp->next->prev = new->meta->last;
        new->meta->last->next = old_tmp->next;
    }

    // replace the metaaries of the inserted segment
    // by those of the host segment
    FI_PATH *tmp = new;
    while (tmp != tmp_meta->last) {
        tmp->meta = old_tmp->meta;
        tmp = tmp->next;
    }
    tmp->meta = old_tmp->meta;

    // isolate the old point
    old_tmp->next = NULL;
    old_tmp->meta = NULL;

    // free the old point
    fi_free_path(old_tmp);

    new->meta->n_total += tmp_meta->n_total - 1;
    new->meta->n_end += tmp_meta->n_end - 1;
    new->meta->n_move += tmp_meta->n_move - 1;
    new->meta->n_line += tmp_meta->n_line - 1;
    new->meta->n_arc += tmp_meta->n_arc - 1;
    new->meta->n_qbez += tmp_meta->n_qbez - 1;
    new->meta->n_cbez += tmp_meta->n_cbez - 1;
    // free the new path meta (it's using the old one now)
    free(tmp_meta);

    return;
}

void fi_copy_path(FI_PATH *in, FI_PATH **out) {
    FI_PATH *tmp = in;
    FI_PATH *out_current = NULL;
    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        FI_SEG_FLAG flag = tmp->section.flag;
        FI_POINT_D *pt = tmp->section.points;
        fi_append_new_seg(&out_current, type);
        switch (type) {
        case FI_SEG_END:
            break;
        case FI_SEG_MOVE:
            out_current->meta->last->section.points[0] = pt[0];
            break;
        case FI_SEG_LINE:
            out_current->meta->last->section.points[0] = pt[0];
            break;
        case FI_SEG_ARC:
            out_current->meta->last->section.points[0] = pt[0];
            out_current->meta->last->section.points[1] = pt[1];
            out_current->meta->last->section.points[2] = pt[2];
            out_current->meta->last->section.flag = flag;
            break;
        case FI_SEG_QUA_BEZIER:
            out_current->meta->last->section.points[0] = pt[0];
            out_current->meta->last->section.points[1] = pt[1];
            break;
        case FI_SEG_CUB_BEZIER:
            out_current->meta->last->section.points[0] = pt[0];
            out_current->meta->last->section.points[1] = pt[1];
            out_current->meta->last->section.points[2] = pt[2];
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
            tmp->section.points[2].x += pt.x;
            tmp->section.points[2].y += pt.y;
            break;
        case FI_SEG_QUA_BEZIER:
            tmp->section.points[0].x += pt.x;
            tmp->section.points[0].y += pt.y;
            tmp->section.points[1].x += pt.x;
            tmp->section.points[1].y += pt.y;
            break;
        case FI_SEG_CUB_BEZIER:
            tmp->section.points[0].x += pt.x;
            tmp->section.points[0].y += pt.y;
            tmp->section.points[1].x += pt.x;
            tmp->section.points[1].y += pt.y;
            tmp->section.points[2].x += pt.x;
            tmp->section.points[2].y += pt.y;
            break;
        }
        tmp = tmp->next;
    }
}

int fi_append_new_seg(FI_PATH **path, FI_SEG_TYPE type) {
    FI_PATH *new_path = calloc(1, sizeof(FI_PATH));
    FI_POINT_D *new_seg;
    int n_point = 0;
    if (*path == NULL || (*path)->meta == NULL) {
        *path = new_path;
        (*path)->meta = calloc(sizeof(FI_META), 1);
        new_path->meta->last = new_path;
        new_path->meta->first = new_path;
        new_path->meta->n_max = DEFAULT_MAX_PATH_LENGTH;
    } else {
        if ((*path)->meta->n_total >= (*path)->meta->n_max) {
            free(new_path);
            return ERR_PATH_TOO_LONG;
        }
        (*path)->meta->last->next = new_path;
        new_path->prev = (*path)->meta->last;
        (*path)->meta->last = new_path;
        new_path->meta = (*path)->meta;
    }
    new_path->meta->n_total += 1;
    switch (type) {
    case FI_SEG_END:
        new_seg = NULL;
        new_path->meta->n_end += 1;
        n_point = 0;
        break;
    case FI_SEG_MOVE:
        new_seg = calloc(1, sizeof(FI_POINT_D));
        new_path->meta->n_move += 1;
        n_point = 1;
        break;
    case FI_SEG_LINE:
        new_seg = calloc(1, sizeof(FI_POINT_D));
        new_path->meta->n_line += 1;
        n_point = 1;
        break;
    case FI_SEG_ARC:
        new_seg = calloc(3, sizeof(FI_POINT_D));
        new_path->meta->n_arc += 1;
        n_point = 3;
        break;
    case FI_SEG_QUA_BEZIER:
        new_seg = calloc(2, sizeof(FI_POINT_D));
        new_path->meta->n_qbez += 1;
        n_point = 2;
        break;
    case FI_SEG_CUB_BEZIER:
        new_seg = calloc(3, sizeof(FI_POINT_D));
        new_path->meta->n_cbez += 1;
        n_point = 3;
        break;
    default:
        new_seg = NULL;
        break;
    }
    new_path->section.points = new_seg;
    new_path->section.type = type;
    new_path->section.n_point = n_point;
    return 0;
}

int fi_compare_point(FI_POINT_D p1, FI_POINT_D p2) {
    int ret = 0;
    if (p1.x < p2.x)
        ret = -1;
    if (p1.x == p2.x) {
        if (p1.y < p2.y)
            ret = -1;
        if (p1.y == p2.y)
            ret = 0;
        if (p1.y > p2.y)
            ret = 1;
    }
    if (p1.x > p2.x)
        ret = 1;
    return ret;
}

int fi_compare_x(const void *in_1, const void *in_2) {
    FI_PATH **seg_1 = (FI_PATH **)in_1;
    FI_PATH **seg_2 = (FI_PATH **)in_2;
    int n_point_1 = (*seg_1)->section.n_point;
    int n_point_2 = (*seg_2)->section.n_point;

    // if it's a segment with no points (like Z)
    if (n_point_1 == 0)
        return 1;
    if (n_point_2 == 0)
        return -1;

    // last point index
    int l1 = n_point_1 - 1;
    int l2 = n_point_2 - 1;
    FI_POINT_D p1 = (*seg_1)->section.points[l1];
    FI_POINT_D p2 = (*seg_2)->section.points[l2];
    return fi_compare_point(p1, p2);
}

void fi_sort_path(FI_PATH **table_path_in, size_t table_len, FI_PATH ***out,
                  size_t *len_out) {
    int len_seg = 0;
    for (int i = 0; i < table_len; i++) {
        len_seg += table_path_in[i]->meta->n_total;
    }
    FI_PATH **tmp_out = calloc(len_seg, sizeof(FI_PATH *));
    size_t len_tmp_out = 0;
    for (int i = 0; i < table_len; i++) {
        FI_PATH *tmp = table_path_in[i];
        while (tmp != NULL) {
            tmp_out[len_tmp_out] = tmp;
            tmp = tmp->next;
            len_tmp_out++;
        }
    }
    qsort(tmp_out, len_tmp_out, sizeof(FI_PATH *), fi_compare_x);
    *out = tmp_out;
    *len_out = len_tmp_out;
}

bool fi_is_left_event(FI_SWEEPEVENT *event) {
    if (event == NULL || event->other == NULL)
        return false;
    int cmp = fi_compare_point(event->point, event->other->point);
    return (cmp < 0);
}

int fi_validate_path(FI_PATH *path) {
    bool in_path = false;
    int counter = 0;
    FI_PATH *tmp = path;
    while (tmp != NULL) {
        FI_SEG_TYPE type = tmp->section.type;
        switch (type) {
        case FI_SEG_END:
            if (!in_path)
                return ERR_PATH_NO_MZ;
            if (counter < 2)
                return ERR_PATH_SECTION_TOO_SHORT;
            in_path = false;
            break;
        case FI_SEG_MOVE:
            if (in_path)
                return ERR_PATH_NO_MZ;
            in_path = true;
            break;
        case FI_SEG_LINE:
        case FI_SEG_ARC:
        case FI_SEG_QUA_BEZIER:
        case FI_SEG_CUB_BEZIER:
            if (!in_path)
                return ERR_PATH_NO_MZ;
            counter++;
        }
        tmp = tmp->next;
    }
    if (in_path)
        return ERR_PATH_NO_MZ;
    else
        return 0;
}

void fi_insert_events(FI_PATH *path, FI_SWEEPEVENT **event_queue,
                      FI_POLYGON_TYPE type) {
    FI_PATH *tmp = path;
    FI_SWEEPEVENT *segment_start;
    FI_SWEEPEVENT *last_segment = NULL;
    FI_SWEEPEVENT *ret = NULL;

    while (tmp != NULL) {
        FI_POINT_D *pt = tmp->section.points;
        int pt_index = tmp->section.n_point - 1;
        FI_SEG_TYPE type = tmp->section.type;
        /*
         * for each point, we create 2 events:
         * - one for the segment prev_point <-> current_point
         * - one for the segment current_point <-> next_point
         */
        FI_SWEEPEVENT *new_event_prev = calloc(1, sizeof(FI_SWEEPEVENT));
        FI_SWEEPEVENT *new_event_next = calloc(1, sizeof(FI_SWEEPEVENT));
        if (ret == NULL)
            ret = new_event_prev;
        switch (type) {
        case FI_SEG_END:
            if (last_segment != NULL) {
                segment_start->other = last_segment;
                segment_start->is_left_event = fi_is_left_event(segment_start);
                last_segment->other = segment_start;
                last_segment->is_left_event = fi_is_left_event(last_segment);
            }
            free(new_event_prev);
            free(new_event_next);
            break;
        case FI_SEG_LINE:
        case FI_SEG_ARC:
        case FI_SEG_QUA_BEZIER:
        case FI_SEG_CUB_BEZIER:
        case FI_SEG_MOVE:
            if (last_segment != NULL) {
                last_segment->other = new_event_prev;
                last_segment->is_left_event = fi_is_left_event(last_segment);
                last_segment->next = new_event_prev;
            }
            new_event_prev->other = last_segment;
            new_event_prev->is_left_event = fi_is_left_event(new_event_prev);
            new_event_prev->point = pt[pt_index];
            new_event_prev->polygon_type = type;
            new_event_prev->next = new_event_next;
            new_event_next->prev = new_event_prev;
            new_event_next->point = pt[pt_index];
            new_event_next->polygon_type = type;
            last_segment = new_event_next;
            break;
        }
        if (type == FI_SEG_MOVE)
            segment_start = new_event_prev;
        tmp = tmp->next;
    }
    if (*event_queue != NULL) {
        last_segment->next = *event_queue;
        (*event_queue)->prev = last_segment;
    }
    *event_queue = ret;
}

void fi_sort_events(FI_SWEEPEVENT **event_queue) {
    // TODO
    return;
}

void fi_create_sweepevent_queue(FI_PATH *path_subject, FI_PATH *path_clip,
                                FI_SWEEPEVENT **event_queue) {
    *event_queue = NULL;
    fi_insert_events(path_subject, event_queue, FI_SUBJECT);
    fi_insert_events(path_clip, event_queue, FI_CLIPPED);
    fi_sort_events(event_queue);
}
