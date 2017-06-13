typedef enum {
   FI_SEG_END = 0x00,
   FI_SEG_MOVE = 0x01,
   FI_SEG_LINE = 0x02,
   FI_SEG_ARC = 0x03,
   FI_SEG_BEZIER = 0x04,
} FI_SEG_TYPE;

typedef enum
{
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

int ficlip(FI_PATH p1, FI_PATH p2, FI_OPS operation);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
