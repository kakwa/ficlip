/* This file is part of the ficlip clipping library
 *
 * ficlip is licensed under MIT.
 *
 * Copyright 2017, Pierre-Francois Carpentier
 */

#include <stdio.h>

/* Type of segments
 */
typedef enum {
    FI_SEG_END = 0x00,
    FI_SEG_MOVE = 0x01,
    FI_SEG_LINE = 0x02,
    FI_SEG_ARC = 0x03,
    FI_SEG_QUA_BEZIER = 0x04,
    FI_SEG_CUB_BEZIER = 0x05,
} FI_SEG_TYPE;

/* List of operations possible
 */
typedef enum {
    FI_AND = 0x01,
    FI_OR = 0x02,
    FI_XOR = 0x03,
    FI_DIFF = 0x04,
} FI_OPS;

/* List segment flags
 */
typedef enum {
    FI_LARGE_ARC = 0x01,
    FI_SWEEP = 0x02,
} FI_SEG_FLAG;

typedef enum {
    FI_CONVALL = 0x01,
} FI_MODE;

typedef struct {
    double x;
    double y;
} FI_POINT_D;

typedef struct {
    FI_SEG_TYPE type;
    FI_POINT_D *points;
    FI_SEG_FLAG flag;
} FI_PATH_SECTION;

typedef struct _FI_BOUND {
    struct _FI_PATH *last;
    struct _FI_PATH *first;
} FI_BOUND;

typedef struct _FI_PATH {
    FI_PATH_SECTION section;
    struct _FI_BOUND *bound;
    struct _FI_PATH *next;
    struct _FI_PATH *prev;
} FI_PATH;

typedef struct _FI_PARAM_ARC {
    FI_POINT_D center;
    FI_POINT_D radius;
    double phi;
    double angle_s;
    double angle_d;
} FI_PARAM_ARC;

/* Build the clipping path from paths "p1" and "p2" with operation "ops"
 * Result in FIPATH **out, return integer error code
 */
int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode);

/* Add a new segment of a given type to a FI_PATH
 */
void fi_append_new_seg(FI_PATH **path, FI_SEG_TYPE type);

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
int parse_path(const char *in, FI_PATH **out);

/* Draw a point (x,y format)
 */
void fi_point_draw_d(FI_POINT_D pt, FILE *out);

/* Draw a FI_PATH in an SVG manner (M x1,y1 L x2,y2 C x3,y3 x4,y4 x5,y5 A x6,y6
 * x7,y7 Z)
 */
void fi_draw_path(FI_PATH *in, FILE *out);

/* start an svg document
 *
 */
void fi_start_svg_doc(FILE *out, double width, double height);

/* end an svg document
 *
 */
void fi_end_svg_doc(FILE *out);

/* start an svg path
 *
 */
void fi_start_svg_path(FILE *out);

/* end an svg path
 *
 */
void fi_end_svg_path(FILE *out, double stroke_width, char *stroke_color,
                     char *fill_color, char *fill_opacity);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
