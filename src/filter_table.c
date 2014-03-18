/*
 * ============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  filterTable main script
 *
 *        Version:  1.0
 *        Created:  14/03/14 14:11:29
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>


#include "filter_table.h"

static inline int
ft_median (FT_CELL_TYPE *data, FT_CELL_TYPE threshold, size_t count)
{
    FT_CELL_TYPE med = quick_select(data, count);
    return med >= (FT_CELL_TYPE) threshold;
}

static inline int
ft_num_nonzero (FT_CELL_TYPE *data, FT_CELL_TYPE threshold, size_t count)
{
    size_t iii = 0;
    size_t passes = 0;
    while ((iii < count) && (passes < (size_t)threshold)) {
        if (data[iii++] > 0.) passes++;
    }
    return passes >= threshold;
}

int
filter_table (table_t *tab)
{
    char *buf = calloc(FT_BUFFSIZE, sizeof(*buf));
    FT_CELL_TYPE *cel_buf = calloc(FT_BUFFSIZE, sizeof(*cel_buf));
    size_t row = 0;
    ssize_t rowlen = 0;
    while (fgets(buf, FT_BUFFSIZE, tab->fp) != NULL) {
        if (row < tab->skiprow) {
            row++;
            fprintf(tab->outfp, "%s", buf);
            continue;
        }
        size_t col = 0;
        size_t cell = 0;
        char *tok_tmp = NULL;
        char *token = NULL;
        char *tok_line = strdup(buf);
        token = strtok_r(tok_line, tab->sep, &tok_tmp);
        while (token != NULL) {
            if (col < tab->skipcol) {
                col++;
                continue;
            }
            cel_buf[cell++] = strtod(token, NULL);
            col++;
            token = strtok_r(NULL, tab->sep, &tok_tmp);
        }
        if ((*(tab->filter_fn))(cel_buf, tab->threshold, col + 1)) {
            fprintf(tab->outfp, "%s", buf);
        }
        row++;
        free(tok_line);
        memset(buf, 0, FT_BUFFSIZE);
        memset(cel_buf, 0, FT_BUFFSIZE);
        if (row % 100000 == 0) {fprintf(stderr, "."); fflush(stderr);}
    }
    free(buf);
    free(cel_buf);
    return 1;
}

void
print_usage()
{
    fprintf(stderr, "filterTable\n\n");
    fprintf(stderr, "Filter a large table row-wise.\n\n");
    fprintf(stderr, "USAGE:\n\n");
    fprintf(stderr, "filterTable [-r ROWS -c COLS -i INFILE -o OUTFILE -s SEP] -m | -z THRESH\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "\t-m THRESH\tUse median method of filtering, with threshold THRESH.\n");
    fprintf(stderr, "\t-z THRESH\tUse number of non-zero cells to filter, with threshold THRESH.\n");
    fprintf(stderr, "\t-r ROWS\t\tSkip ROWS rows from start of table.\n");
    fprintf(stderr, "\t-c COLS\t\tSkip COLS columns from start of each row.\n");
    fprintf(stderr, "\t-s SEP\t\tUse string SEP as field seperator, not \"\\t\".\n");
    fprintf(stderr, "\t-i INFILE\tInput from INFILE, not stdin (or '-' for stdin).\n");
    fprintf(stderr, "\t-o OUTFILE\tOutput to OUTFILE, not stdout (or '-' for stdout).\n");
}

int
parse_args (int argc, char *argv[], table_t *tab)
{
    unsigned char haveflags = 0;
    /*
        1 1 1 1 1 1 1 1
            | | | | | \- method
            | | | | \--- out fname
            | | | \----- in fname
            | | \------- cols to skip
            | \--------- rows to skip
            \----------- Field sep
    */
    char c = '\0';
    while((c = getopt(argc, argv, "m:z:r:c:o:i:s:")) >= 0) {
        switch (c) {
            case 'm':
                haveflags |= 1;
                tab->filter_fn = &ft_median;
                tab->threshold = atof(optarg);
                break;
            case 'z':
                haveflags |= 1;
                tab->filter_fn = &ft_num_nonzero;
                tab->threshold = atof(optarg);
                break;
            case 'o':
                haveflags |= 2;
                tab->outfname = strdup(optarg);
                break;
            case 'i':
                haveflags |= 4;
                tab->fname = strdup(optarg);
                break;
            case 'c':
                haveflags |= 8;
                tab->skipcol = atol(optarg);
                break;
            case 'r':
                haveflags |= 16;
                tab->skiprow = atol(optarg);
                break;
            case 's':
                haveflags |= 32;
                tab->sep = strdup(optarg);
                break;
            /* case '?':
                fprintf(stderr, "[parse_args] Bad argument '-%c'\n", optopt);
                print_usage();
                return 0;*/
        }
    }
    if (tab->sep == NULL) {
        tab->sep = strdup("\t");
    }
    /* Setup input fp */
    if ((!haveflags & 4) || tab->fname == NULL || \
            strncmp(tab->fname, "-", 1) == 0) {
        tab->fp = fdopen(fileno(stdout), "r");
        tab->fname = strdup("stdin");
        haveflags |= 4;
    } else {
        tab->fp = fopen(tab->fname, "r");
    }
    if (tab->fp == NULL) {
        fprintf(stderr, "Could not open file '%s'\n%s\n", tab->fname,
                strerror(errno));
        return 0;
    }
    /* Setup output fp */
    if ((!haveflags & 2) || tab->outfname == NULL || \
            strncmp(tab->outfname, "-", 1) == 0) {
        tab->outfp = fdopen(fileno(stdout), "w");
        tab->outfname = strdup("stdin");
        haveflags |= 2;
    } else {
        tab->outfp = fopen(tab->outfname, "w");
    }
    if (tab->outfp == NULL) {
        fprintf(stderr, "Could not open file '%s'\n%s\n", tab->outfname,
                strerror(errno));
        return 0;
    }
    if ((haveflags & 7) != 7) {
        fprintf(stderr, "[parse_args] Required arguments missing\n");
        print_usage();
        return 0;
    } else {
        return 1;
    }
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 * =====================================================================================
 */
int
main (int argc, char *argv[])
{
    if (argc == 1) {
        print_usage();
        exit(EXIT_SUCCESS);
    }
    table_t *tab = calloc(1, sizeof(*tab));
    if (tab == NULL) {
        fprintf(stderr, "Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    if (!parse_args(argc, argv, tab)) {
        destroy_table_t(tab);
        fprintf(stderr, "Cannot parse arguments.\n");
        exit(EXIT_FAILURE);
    }

    if (!filter_table(tab)) {
        destroy_table_t(tab);
        fprintf(stderr, "Error during table filtering.\n");
        exit(EXIT_FAILURE);
    }
    destroy_table_t(tab);
    return EXIT_SUCCESS;
} /* ----------  end of function main  ---------- */
