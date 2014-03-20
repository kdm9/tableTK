/*
 * ============================================================================
 *
 *       Filename:  dist.c
 *
 *    Description:  Distance matrix calc
 *
 *        Version:  1.0
 *        Created:  19/03/14 17:49:05
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "kdm.h"
#include "ktable.h"

typedef struct _distmat {
    size_t samples;
    size_t pairs;
    cell_t *matrix;
} dist_mat_t;


#define	KM_ABS_DIFF(a, b) ((a) > (b)? (a) - (b) : (b) - (a))
#define	KM_ABS_SUM(a, b) (((a) + (b)) >= 0 ? (a) + (b) : (a) - (b))

static inline cell_t
calc_canberra (cell_t left, cell_t right, cell_mode_t mode)
{
    cell_t ret;
    switch(mode) {
        case U64:
            ret.u = KM_ABS_DIFF(left.u, right.u) / KM_ABS_SUM(left.u, right.u);
            break;
        case I64:
            ret.i = KM_ABS_DIFF(left.i, right.i) / KM_ABS_SUM(left.i, right.i);
            break;
        case D64:
            ret.d = KM_ABS_DIFF(left.d, right.d) / KM_ABS_SUM(left.d, right.d);
            break;
    }
    return ret;
}

static inline cell_t
calc_manhattan (cell_t left, cell_t right, cell_mode_t mode)
{
    cell_t ret;
    switch(mode) {
        case U64:
            ret.u = KM_ABS_DIFF(left.u, right.u);
            break;
        case I64:
            ret.i = KM_ABS_DIFF(left.i, right.i);
            break;
        case D64:
            ret.d = KM_ABS_DIFF(left.d, right.d);
            break;
    }
    return ret;
}

static inline void
do_pairwise (void *data, cell_t *cells, size_t count, cell_mode_t mode,
        cell_t (*calc)(cell_t, cell_t, cell_mode_t))
{

    dist_mat_t *mat = (dist_mat_t *)data;
    size_t aaa, bbb, iii = 0;
    for (aaa = 0; aaa < count; aaa++) {
        for (bbb = aaa + 1; bbb < count; bbb++) {
            cell_t res = (*calc)(cells[aaa], cells[bbb], mode);
            switch(mode) {
                case U64:
                    mat->matrix[iii++].u += res.u;
                    break;
                case I64:
                    mat->matrix[iii++].i += res.i;
                    break;
                case D64:
                    mat->matrix[iii++].d += res.d;
                    break;
            }
        }
    }
}

static inline void
dm_canberra (table_t *tab, char *line, cell_t *cells, size_t count,
               void *data)
{
    dist_mat_t *mat = (dist_mat_t *)data;
    if (km_unlikely(mat->samples == 0)) {
        mat->samples = count;
    }
    if (km_unlikely(mat->pairs == 0)) {
        mat->pairs = ((count * (count - 1)) / 2);
    }
    if (km_unlikely(mat->matrix == NULL)) {
        mat->matrix = km_calloc(mat->pairs, sizeof(*(mat->matrix)),
                &km_onerr_print_exit);
        for (size_t iii = 0; iii < mat->pairs; iii++)
            mat->matrix[iii].d = 0.0l;
    }
    do_pairwise(data, cells, count, tab->mode, &calc_canberra);
}

static inline void
dm_manhattan (table_t *tab, char *line, cell_t *cells, size_t count,
               void *data)
{
    dist_mat_t *mat = (dist_mat_t *)data;
    if (km_unlikely(mat->samples == 0)) {
        mat->samples = count;
    }
    if (km_unlikely(mat->pairs == 0)) {
        mat->pairs = ((count * (count - 1)) / 2);
    }
    if (km_unlikely(mat->matrix == NULL)) {
        mat->matrix = km_calloc(mat->pairs, sizeof(*(mat->matrix)),
                &km_onerr_print_exit);
    }
    do_pairwise(data, cells, count, tab->mode, &calc_manhattan);
}

void
print_dist_mat (dist_mat_t *mat)
{
    size_t rrr, ccc, iii=0;
    for (rrr = 0; rrr < mat->samples; rrr++) {
        for (ccc = 0; ccc < mat->samples; ccc++) {
            if (rrr == ccc) printf("%Lf\t", 0.0l);
            else if (ccc < rrr + 1) printf(".\t");
            else printf("%Lf\t", mat->matrix[iii++].d);
        }
        printf("\n");
    }
}

int
calc_dist_matrix_of_table(table_t *tab)
{
    dist_mat_t *mat = km_calloc(1, sizeof(*mat), &km_onerr_print_exit);
    tab->mode = D64;
    iter_table(tab, mat);
    print_dist_mat(mat);
    return 1;
}

void
print_usage()
{
    fprintf(stderr, "tableDist\n\n");
    fprintf(stderr, "Calculate a distance matrix between columns in a table.\n\n");
    fprintf(stderr, "USAGE:\n\n");
    fprintf(stderr, "tableDist [-r ROWS -c COLS -i INFILE -o OUTFILE -s SEP] -C | -m | -e\n");
    fprintf(stderr, "tableDist -h\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "\t-C | -m | -e\t Use Canberra, Manhattan or Euclidean distance measures.\n");
    fprintf(stderr, "\t-r ROWS\t\tSkip ROWS rows from start of table.\n");
    fprintf(stderr, "\t-c COLS\t\tSkip COLS columns from start of each row.\n");
    fprintf(stderr, "\t-s SEP\t\tUse string SEP as field seperator, not \"\\t\".\n");
    fprintf(stderr, "\t-i INFILE\tInput from INFILE, not stdin (or '-' for stdin).\n");
    fprintf(stderr, "\t-o OUTFILE\tOutput to OUTFILE, not stdout (or '-' for stdout).\n");
    fprintf(stderr, "\t-h \t\tPrint this help message.\n");
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
    while((c = getopt(argc, argv, "mCr:c:o:i:s:h")) >= 0) {
        switch (c) {
            case 'm':
                haveflags |= 1;
                tab->row_fn = &dm_manhattan;
                break;
            case 'C':
                haveflags |= 1;
                tab->row_fn = &dm_canberra;
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
            case 'h':
                print_usage();
                destroy_table_t(tab);
                exit(EXIT_SUCCESS);
        }
    }
    if (tab->sep == NULL) {
        tab->sep = strdup("\t");
    }
    /* Setup input fp */
    if ((!(haveflags & 4)) || tab->fname == NULL || \
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
    if ((!(haveflags & 2)) || tab->outfname == NULL || \
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
        return 0;
    }
    return 1; /* Successful */
}


/*
 * ===  FUNCTION  =============================================================
 *         Name:  main
 * ============================================================================
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
        fprintf(stderr, "Cannot allocate memory for a table struct.\n");
        exit(EXIT_FAILURE);
    }
    if (!parse_args(argc, argv, tab)) {
        destroy_table_t(tab);
        fprintf(stderr, "Cannot parse arguments.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }
    if (!calc_dist_matrix_of_table(tab)) {
        destroy_table_t(tab);
        fprintf(stderr, "Error during distance matrix calculation.\n");
        exit(EXIT_FAILURE);
    }
    destroy_table_t(tab);
    return EXIT_SUCCESS;
} /* ----------  end of function main  ---------- */
