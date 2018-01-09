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
    FI_SEG_BEZIER = 0x04,
} FI_SEG_TYPE;

/* List of operations possible
 */
typedef enum {
    FI_AND = 0x01,
    FI_OR = 0x02,
    FI_XOR = 0x03,
    FI_DIFF = 0x04,
} FI_OPS;

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
} FI_PATH_SECTION;

typedef struct _FI_PATH {
    FI_PATH_SECTION section;
    struct _FI_PATH *last;
    struct _FI_PATH *next;
} FI_PATH;

/* Build the clipping path from pathes "p1" and "p2" with opetration "ops"
 * Result in FIPATH **out, return integer error code
 */
int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out, FI_MODE mode);

/* Add a new segment of a given type to a FI_PATH
 */
void fi_add_new_seg(FI_PATH **path, FI_SEG_TYPE type);

/* Free a FI_PATH structure
 */
void fi_free_path(FI_PATH *path);

/* Copy a FI_PATH
 */
void fi_copy_path(FI_PATH *in, FI_PATH **out);

/* Offset a FI_PATH
 */
void fi_offset_path(FI_PATH *in, FI_POINT_D pt);

/* Convert elliptic arc to a series of segments
 */
void fi_arc_to_lines(FI_PATH *in, FI_PATH **out);

/* Convert a quadratic bezier curve to a series of segments
 */
void fi_arc_to_lines(FI_PATH *in, FI_PATH **out);

/* Use at your own risks the following function (used for testing) */

/* Rough function to parse an SVG manner path string to create a FI_PATH
 */
int parse_path(char *in, FI_PATH **out);

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
                     char *fill_color);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
