#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_LINE 1024

static int parse_number(char **p, float *out) {
    while (isspace(**p)) (*p)++;
    if (**p == '\0') return 0;

    char *end;
    errno = 0;
    float x = strtof(*p, &end);

    if (end == *p) return -1;
    if (errno == ERANGE) return -1;
    if (*end != '\0' && !isspace(*end)) return -1;

    *out = x;
    *p = end;
    return 1;
}

static int process_line(char *line, FILE *f) {
    char *p = line;
    float result, x;

    int r = parse_number(&p, &result);
    if (r == 0) return 0;
    if (r < 0) {
        fprintf(stderr, "Invalid number\n");
        return 1;
    }

    while (1) {
        r = parse_number(&p, &x);
        if (r == 0) break;
        if (r < 0) {
            fprintf(stderr, "Invalid number\n");
            return 1;
        }
        if (x == 0) return 2;
        result /= x;
    }

    if (fprintf(f, "%g\n", result) < 0 || fflush(f) != 0) {
        perror("W");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argument amount. Requires a filename for output\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "w");
    if (f == NULL) {
        perror("fopen");
        return 1;
    }

    int code = 0;
    char line[MAX_LINE + 2];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        int r = process_line(line, f);
        if (r != 0) {
            code = r;
            break;
        }
    }

    if (fclose(f) != 0) {
        perror("fclose");
        if (code == 0) code = 1;
    }
    return code;
}
