/* This file is part of the ficlip clipping library
 *
 * ficlip is licensed under MIT.
 *
 * Copyright 2017, Pierre-Francois Carpentier
 */

/* Default maximum length of a path
 */
#define DEFAULT_MAX_PATH_LENGTH 1000

/* Error codes
 */
#define ERR_PARSING_FAIL 0x01
#define ERR_PATH_TOO_LONG 0x02
#define ERR_PATH_NO_MZ 0x03
#define ERR_PATH_SECTION_TOO_SHORT 0x04

#include <stdio.h>
#include <stdbool.h>

/* Type of segments
 */
typedef enum {
    FI_SEG_END = 0x00,        // End and close the path
    FI_SEG_MOVE = 0x01,       // Move cursor without tracing
    FI_SEG_LINE = 0x02,       // Line to target point
    FI_SEG_ARC = 0x03,        // Elliptic arc to point
    FI_SEG_QUA_BEZIER = 0x04, // Quadratic Bezier curve to point
    FI_SEG_CUB_BEZIER = 0x05, // Cubic bezier curve to point
} FI_SEG_TYPE;

/* List of operations possible
 */
typedef enum {
    FI_AND = 0x01,  // and operation on shapes
    FI_OR = 0x02,   // or operation on shapes
    FI_XOR = 0x03,  // xor operation on shapes
    FI_DIFF = 0x04, // diff operation on shapes
} FI_OPS;

/* List segment flags
 */
typedef enum {
    FI_LARGE_ARC = 0x01,
    FI_SWEEP = 0x02,
} FI_SEG_FLAG;

/* flag list for conversion mode
 */
typedef enum {
    FI_CONVALL = 0x01, // convert all curves to segments
} FI_MODE;

/* definition of a point (x, y)
 */
typedef struct {
    double x;
    double y;
} FI_POINT_D;

/* Definition of a bezier curve/elliptic arc/segment portion of a curve
 */
typedef struct {
    FI_SEG_TYPE type;
    FI_POINT_D *points;
    FI_SEG_FLAG flag;
    int n_point;
} FI_PATH_SECTION;

/* Wrapper structure for a path
 */
typedef struct _FI_META {
    struct _FI_PATH *last;
    struct _FI_PATH *first;
    int n_total;
    int n_max;
    int n_end;
    int n_move;
    int n_line;
    int n_arc;
    int n_qbez;
    int n_cbez;
} FI_META;

/* actual path definition
 */
typedef struct _FI_PATH {
    FI_PATH_SECTION section;
    struct _FI_META *meta;
    struct _FI_PATH *next;
    struct _FI_PATH *prev;
} FI_PATH;

/* parameterize structure for defining an elliptic arc
 */
typedef struct _FI_PARAM_ARC {
    FI_POINT_D center;
    FI_POINT_D radius;
    double phi;
    double angle_s;
    double angle_d;
} FI_PARAM_ARC;

typedef enum {
    FI_SUBJECT,
    FI_CLIPPED,
} FI_POLYGON_TYPE;

typedef enum {
    FI_NON_CONTRIBUTIN,
    FI_SAME_TRANSITION,
    FI_DIFFERENT_TRANSITION,
} FI_EDGETYPE;

typedef struct _FI_SWEEPEVENT {
    FI_POINT_D point;
    FI_POLYGON_TYPE polygon_type;
    FI_EDGETYPE type;
    bool in_out;
    bool inside;
    bool is_left_event;
    struct _FI_SWEEPEVENT *other;
    struct _FI_SWEEPEVENT *next;
    struct _FI_SWEEPEVENT *prev;
} FI_SWEEPEVENT;

/* Build the clipping path from paths "p1" and "p2" with operation "ops"
 * Result in FIPATH **out, return integer error code
 */
int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode);

/* Add a new segment of a given type to a FI_PATH
 */
int fi_append_new_seg(FI_PATH **path, FI_SEG_TYPE type);

/* Free a FI_PATH structure
 */
void fi_free_path(FI_PATH *path);

/* Copy a FI_PATH
 */
void fi_copy_path(FI_PATH *in, FI_PATH **out);

/* Offset a FI_PATH
 */
void fi_offset_path(FI_PATH *in, FI_POINT_D pt);

/* Replace the first point of old with the new segment
 */
void fi_replace_path(FI_PATH **old, FI_PATH *new);

/* Convert elliptic arc from the endpoints to center parameterization
 */
FI_PARAM_ARC fi_arc_endpoint_to_center(FI_POINT_D s, FI_POINT_D e, FI_POINT_D r,
                                       double phi, FI_SEG_FLAG flag);

/* Convert elliptic arc to a series of segments
 */
void fi_arc_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_SEG_FLAG flag,
                     FI_PATH **out);

/* Convert a cubic bezier curve to a series of segments
 */
void fi_cub_bezier_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out);

/* Convert a quadratic bezier curve to a series of segments
 */
void fi_qua_bezier_to_lines(FI_POINT_D ref, FI_POINT_D *in, FI_PATH **out);

/* Convert a "complex" path (arc and bezier) to a series of segments
 */
void fi_linearize(FI_PATH **in);

/* Use at your own risks the following function (used for testing) */

/* Rough function to parse an SVG manner path string to create a FI_PATH
 */
int fi_parse_path(const char *in, int s_in, FI_PATH **out);

/* Draw a point (x,y format)
 */
void fi_point_draw_d(FI_POINT_D pt, FILE *out);

/* Draw a FI_PATH in an SVG manner (M x1,y1 L x2,y2 C x3,y3 x4,y4 x5,y5 A x6,y6
 * x7,y7 Z)
 */
void fi_draw_path(FI_PATH *in, FILE *out);

/* start an svg document
 */
void fi_start_svg_doc(FILE *out, double width, double height);

/* end an svg document
 */
void fi_end_svg_doc(FILE *out);

/* start an svg path
 */
void fi_start_svg_path(FILE *out);

/* end an svg path
 */
void fi_end_svg_path(FILE *out, double stroke_width, char *stroke_color,
                     char *fill_color, char *fill_opacity);

/* sort an array of PATH into an array of segments
 */
void fi_sort_path(FI_PATH **table_path_in, size_t table_len, FI_PATH ***out,
                  size_t *len_out);

/* Validate that the path is correctly defined (M <stuff> Z section and at least
 * 2 intermediate points (to make at least a triangle
 */
int fi_validate_path(FI_PATH *path);

/* insert a path in the event queue
 */
void fi_insert_events(FI_PATH *path, FI_SWEEPEVENT **event_queue,
                      FI_POLYGON_TYPE type);

/* Create the event queue that will be consumed by the clipping algorithm
 */
void fi_create_sweepevent_queue(FI_PATH *path_subject, FI_PATH *path_clip,
                                FI_SWEEPEVENT **event_queue);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
