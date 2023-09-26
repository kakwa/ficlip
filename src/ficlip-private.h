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
