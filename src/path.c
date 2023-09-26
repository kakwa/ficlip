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
#include <math.h>
#include "ficlip.h"
#include "ficlip-private.h"

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
