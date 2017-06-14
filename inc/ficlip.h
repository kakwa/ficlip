/* This file is part of the ficlip clipping library
 *
 * ficlip is licensed under MIT.
 *
 * Copyright 2017, Pierre-Francois Carpentier
 */

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
int ficlip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out);

/* Add a new segment of a given type to a FI_PATH
 */
void fi_add_new_seg(FI_PATH **path, FI_SEG_TYPE type);

/* Free a FI_PATH structure
 */
void fi_free_path(FI_PATH **path);

/* Draw a FI_PATH in an SVG manner (M x1,y1 L x2,y2 C x3,y3 x4,y4 x5,y5 A x6,y6 x7,y7 Z)
 */
void fi_draw_path(FI_PATH *in, FILE *out);

/* Copy a FI_PATH
 */
void fi_copy_path(FI_PATH *in, FI_PATH **out);

/* Offset a FI_PATH
 */
void fi_offset_path(FI_PATH *in, FI_POINT_D pt);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
