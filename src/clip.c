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

int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode) {
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
