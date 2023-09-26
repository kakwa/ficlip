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
    P0.c *pow((1 - t), 2) + 2 * t * (1 - t) * P1.c + pow(t, 2) * P2.c

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

/**
 * @brief List of segment flags.
 */
typedef enum {
    FI_LARGE_ARC = 0x01, /**< Large arc flag. */
    FI_SWEEP = 0x02,     /**< Sweep flag. */
} FI_SEG_FLAG;

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

/* Replace the first point of old with the new segment
 */
void fi_replace_path(FI_PATH **old, FI_PATH *new);

/* Draw a point (x,y format)
 */
void fi_point_draw_d(FI_POINT_D pt, FILE *out);

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
