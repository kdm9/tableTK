/*
 * ============================================================================
 *
 *       Filename:  table.c
 *
 *    Description:  Common table functions
 *
 *        Version:  1.0
 *        Created:  19/03/14 12:37:45
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "ktable.h"

size_t
count_columns (const char *row, const char *delim, size_t len)
{
    size_t cols = 0;
    size_t bytes_used = 0;
    char *tok_line = strdup(row);
    char *tok_tmp = NULL;
    char *tok = strtok_r(tok_line, delim, &tok_tmp);
    while (tok != NULL && bytes_used < len) {
        bytes_used = (tok - tok_line);
        cols++;
        tok = strtok_r(NULL, delim, &tok_tmp);
    }
    km_free(tok_line);
    return cols + 1; /* Count the last column */
}

inline void
strtocellt (cell_t *cell, const char *str, char **saveptr, cell_mode_t mode)
{
    switch(mode) {
        case U64:
            cell->u = strtoull(str, saveptr, 10);
            break;
        case I64:
            cell->i = strtoll(str, saveptr, 10);
            break;
        case D64:
            cell->d = strtold(str, saveptr);
            break;
    }
}

int
iter_table (table_t *tab, void *data)
{
    if (!table_is_valid(tab)) {
        return -1;
    }
    size_t buffsize = 1<<15;
    char *line = km_calloc(buffsize, sizeof(*line), &km_onerr_print_exit);
    cell_t *cells = NULL;
    size_t row = 0;
    ssize_t rowlen = 0;
    int res = 0;
    while ((rowlen = km_readline_realloc(&line, tab->fp, &buffsize,
                                         &km_onerr_print_exit)) > 0) {
        size_t col = 0;
        size_t cell = 0;
        char *tok_tmp = NULL;
        char *token = NULL;
        char *tok_line = NULL;
        /* Skip rows we don't want */
        if (km_unlikely(row < tab->skiprow)) {
            row++;
            fprintf(tab->outfp, "%s", line);
            continue;
        } else if (km_unlikely(cells == NULL)) {
            /* Get the number of data columns */
            tab->cols = count_columns(line, tab->sep, rowlen) - tab->skipcol;
            cells = km_calloc(tab->cols, sizeof(*cells),
                    &km_onerr_print_exit);
        }
        /* Tokenise the remainder of the line, reading in cells */
        tok_line = strdup(line);
        token = strtok_r(tok_line, tab->sep, &tok_tmp);
        while (token != NULL && cell < tab->cols) {
            if (col++ < tab->skipcol) {
                token = strtok_r(NULL, tab->sep, &tok_tmp);
                continue;
            }
            strtocellt(&(cells[cell++]), token, NULL, tab->mode);
            token = strtok_r(NULL, tab->sep, &tok_tmp);
        }
        km_free(tok_line);
        (*(tab->row_fn))(tab, line, cells, cell, data);
        row++;
        tab->rows++;
    }
    km_free(line);
    km_free(cells);
    return res;
}

inline cell_t
median(cell_t arr[], int n, cell_mode_t mode)
{
    int low, high;
    int median;
    int middle, ll, hh;
    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (cell_elem(arr[low], mode) > cell_elem(arr[high], mode))
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }
    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (cell_elem(arr[middle], mode) > cell_elem(arr[high], mode))
        ELEM_SWAP(arr[middle], arr[high]);
    if (cell_elem(arr[low], mode) > cell_elem(arr[high], mode))
        ELEM_SWAP(arr[low], arr[high]);
    if (cell_elem(arr[middle], mode) > cell_elem(arr[low], mode))
        ELEM_SWAP(arr[middle], arr[low]);
    /* Swap low item (now in position middle) into position (low+1) */
    ELEM_SWAP(arr[middle], arr[low+1]) ;
    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (cell_elem(arr[low], mode) > cell_elem(arr[ll], mode));
        do hh--; while (cell_elem(arr[hh], mode)  > cell_elem(arr[low], mode));
        if (hh < ll)
            break;
        ELEM_SWAP(arr[ll], arr[hh]) ;
    }
    /* Swap middle item (in position low) back into correct position */
    ELEM_SWAP(arr[low], arr[hh]) ;
    /* Re-set active partition */
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }
}
