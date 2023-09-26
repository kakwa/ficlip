/**
 * @file ficlip.h
 * @brief This file is part of the ficlip clipping library.
 *
 * @details ficlip is licensed under MIT.
 *
 * @copyright 2023, Pierre-Francois Carpentier
 */

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Default maximum length of a path.
 */
#define DEFAULT_MAX_PATH_LENGTH 1000

/**
 * @brief Error code for parsing failure.
 */
#define ERR_PARSING_FAIL 0x01
/**
 * @brief Error code for path being too long.
 */
#define ERR_PATH_TOO_LONG 0x02
/**
 * @brief Error code for no MZ path.
 */
#define ERR_PATH_NO_MZ 0x03
/**
 * @brief Error code for section being too short.
 */
#define ERR_PATH_SECTION_TOO_SHORT 0x04

/**
 * @brief Type of segments.
 */
typedef enum {
    FI_SEG_END = 0x00,        /**< End and close the path. */
    FI_SEG_MOVE = 0x01,       /**< Move cursor without tracing. */
    FI_SEG_LINE = 0x02,       /**< Line to target point. */
    FI_SEG_ARC = 0x03,        /**< Elliptic arc to point. */
    FI_SEG_QUA_BEZIER = 0x04, /**< Quadratic Bezier curve to point. */
    FI_SEG_CUB_BEZIER = 0x05, /**< Cubic bezier curve to point. */
} FI_SEG_TYPE;

/**
 * @brief List of operations possible.
 */
typedef enum {
    FI_AND = 0x01,  /**< AND operation on shapes. */
    FI_OR = 0x02,   /**< OR operation on shapes. */
    FI_XOR = 0x03,  /**< XOR operation on shapes. */
    FI_DIFF = 0x04, /**< DIFF operation on shapes. */
} FI_OPS;

/**
 * @brief Definition of a point (x, y).
 */
typedef struct {
    double x; /**< X-coordinate of the point. */
    double y; /**< Y-coordinate of the point. */
} FI_POINT_D;

/**
 * @brief List of segment flags.
 */
typedef enum {
    FI_LARGE_ARC = 0x01, /**< Large arc flag. */
    FI_SWEEP = 0x02,     /**< Sweep flag. */
} FI_SEG_FLAG;

/**
 * @brief Definition of a bezier curve/elliptic arc/segment portion of a curve.
 */
typedef struct {
    FI_SEG_TYPE type;   /**< Type of the segment. */
    FI_POINT_D *points; /**< Array of points. */
    FI_SEG_FLAG flag;   /**< Segment flag. */
    int n_point;        /**< Number of points. */
} FI_PATH_SECTION;

/**
 * @brief Wrapper structure for a path.
 */
typedef struct _FI_META {
    struct _FI_PATH *last;  /**< Pointer to the last path. */
    struct _FI_PATH *first; /**< Pointer to the first path. */
    int n_total;            /**< Total number of paths. */
    int n_max;              /**< Maximum number of paths. */
    int n_end;              /**< Number of end segments. */
    int n_move;             /**< Number of move segments. */
    int n_line;             /**< Number of line segments. */
    int n_arc;              /**< Number of arc segments. */
    int n_qbez;             /**< Number of quadratic Bezier segments. */
    int n_cbez;             /**< Number of cubic Bezier segments. */
} FI_META;

/**
 * @brief Actual path definition.
 */
typedef struct _FI_PATH {
    FI_PATH_SECTION section; /**< Path section information. */
    struct _FI_META *meta;   /**< Pointer to the metadata of the path. */
    struct _FI_PATH *next;   /**< Pointer to the next path. */
    struct _FI_PATH *prev;   /**< Pointer to the previous path. */
} FI_PATH;

/**
 * @brief Build the clipping path from paths "p1" and "p2" with operation "ops".
 *        Result in FIPATH **out, return integer error code.
 *
 * @param p1   The first path.
 * @param p2   The second path.
 * @param ops  The operation to be performed (AND, OR, XOR, DIFF).
 * @param out  Pointer to the result path.
 *
 * @return     Integer error code (0 if successful).
 */
int fi_clip(FI_PATH *p1, FI_PATH *p2, FI_OPS ops, FI_PATH **out);

/**
 * @brief Add a new segment of a given type to a FI_PATH.
 *
 * @param path   Pointer to the path.
 * @param type   Type of the segment to be added.
 *
 * @return       Integer error code (0 if successful).
 */
int fi_append_new_seg(FI_PATH **path, FI_SEG_TYPE type);

/**
 * @brief Free a FI_PATH structure.
 *
 * @param path  Pointer to the path to be freed.
 */
void fi_free_path(FI_PATH *path);

/**
 * @brief Copy a FI_PATH.
 *
 * @param in   Pointer to the input path.
 * @param out  Pointer to the copied path.
 */
void fi_copy_path(FI_PATH *in, FI_PATH **out);

/**
 * @brief Offset a FI_PATH.
 *
 * @param in  Pointer to the input path.
 * @param pt  Point by which the path will be offset.
 */
void fi_offset_path(FI_PATH *in, FI_POINT_D pt);

/**
 * @brief Convert a complex path with Arc and Bezier segments into a series of
 * line segemnts..
 *
 * @param in   Pointer to the input path.
 */
void fi_linearize(FI_PATH **in);

/**
 * @brief Rough function to parse an SVG like path string to create a FI_PATH.
 *
 * @param in    The input path string.
 * @param s_in  Length of the input string.
 * @param out   Pointer to the created path.
 *
 * @return      Integer error code (0 if successful).
 */
int fi_parse_path(const char *in, int s_in, FI_PATH **out);

/**
 * @brief Draw a FI_PATH like an SVG path (M x1,y1 L x2,y2 C x3,y3 x4,y4 x5,y5 A
 * x6,y6 x7,y7 Z).
 *
 * @param in   Pointer to the input path.
 * @param out  Pointer to the output file stream.
 */
void fi_draw_path(FI_PATH *in, FILE *out);
