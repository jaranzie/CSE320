#include <stdlib.h>

extern int orig_main(int argc, const char *const *argv);

int main(int argc, const char *const *argv) {
    orig_main(argc, argv);
}
