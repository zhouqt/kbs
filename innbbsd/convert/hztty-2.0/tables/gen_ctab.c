/*
 * generate xxx_tables.c file for hztty
 */

#include<stdio.h>

static int linemode = 0;

main(argc, argv)
int argc;
char *argv[];
{
    char *prog = argv[0];

    if (argc > 1 && (strcmp(argv[1], "-l") == 0)) {
        linemode = 1;
        argc--;
        argv++;
    }
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [-l] map_file variable_name\n", prog);
        fprintf(stderr, "\t-l: one-char-per-line mode\n");
        exit(1);
    }
    gen(argv[1], argv[2], stdout);
    exit(0);
}

gen(file, varname, of)
char *file;
char *varname;
FILE *of;
{
    FILE *f;
    char buf[80];
    int c1, c2;
    int count = 0;

    f = fopen(file, "r");
    if (!f) {
        perror(file);
        exit(1);
    }

    fprintf(of, "unsigned char %s[] = {\n", varname);

    while (1) {
        c1 = getc(f);
        c2 = getc(f);
        if (c1 == EOF || c2 == EOF)
            break;
        fprintf(of, "0x%02x,0x%02x,", c1, c2);
        if ((++count % 7) == 0)
            putc('\n', of);
        else
            putc(' ', of);
        if (linemode)
            if (getc(f) == EOF) /* to eat the '\n' */
                break;
    }
    fprintf(of, "\n");
    fprintf(of, "};\n");
    fprintf(of, "int %s_count = %d;\n", varname, count);
    fprintf(of, "\n");
}
