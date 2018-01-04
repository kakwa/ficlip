#include "ficlip.h"

int main() {
    FI_PATH *out;
    parse_path("M 0.0 1.1 L 10.0 23.5432 L 0.5 42.987 Z", &out);
    FILE *fp = stdout;
    fi_draw_path(out, fp);
    return 1;
}
