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

inline size_t
count_columns (const char *row, const char *delim)
{
    size_t cols;
    char *tok = NULL;
    while((tok = strpbrk(row, delim)) != NULL) {
        cols++;
    }
    return cols;
}

inline void
strtocellt (cell_t *cell, const char *str, char **saveptr, cell_mode_t mode)
{
    switch(mode) {
        case U64:
            cell->u = strtoull(str, saveptr, 10);
        case I64:
            cell->i = strtoll(str, saveptr, 10);
        case D64:
            cell->d = strtod(str, saveptr);
    }
}

int
iter_table (table_t *tab, void *data,
            int (*result_fn)(table_t *, size_t, int *, char *, cell_t *,
                             void *))
{
    if (!table_is_valid(tab)) {
        return -1;
    }
    size_t buffsize = 1<<8;
    char *buf = km_calloc(buffsize, sizeof(*buf), &km_onerr_print_exit);
    size_t cellbuffsize = 1<<8;
    cell_t *cell_buf = NULL;
    size_t row = 0;
    ssize_t rowlen = 0;
    size_t num_cols = 0;
    if (tab->rows != 0) {
        cell_buf = km_calloc(tab->cols, sizeof(*cell_buf),
                &km_onerr_print_exit);
    }
    while ((rowlen = km_readline_realloc(buf, tab->fp, &buffsize,
                                         &km_onerr_print_exit)) > 0) {
        int *results = km_calloc(tab->n_row_fns, sizeof(*results),
                &km_onerr_print_exit);
        size_t fn_num;
        size_t col = 0;
        size_t cell = 0;
        char *tok_tmp = NULL;
        char *token = NULL;
        char *tok_line = strdup(buf);
        if (row < tab->skiprow) {
            row++;
            fprintf(tab->outfp, "%s", buf);
            continue;
        } else if (cell_buf == NULL) {
            tab->cols = count_columns(buf, tab->sep) - tab->skipcol;
            cell_buf = km_calloc(tab->cols, sizeof(*cell_buf),
                    &km_onerr_print_exit);
        }
        token = strtok_r(tok_line, tab->sep, &tok_tmp);
        while (token != NULL && cell < tab->cols) {
            if (col < tab->skipcol) {
                col++;
               continue;
            }
            strtocellt(&(cell_buf[cell++]), token, NULL, tab->mode);
            col++;
            token = strtok_r(NULL, tab->sep, &tok_tmp);
        }
        for (fn_num = 0; fn_num < tab->n_row_fns; fn_num++) {
            results[fn_num] = (*(tab->row_fns[fn_num])) \
                    (cell_buf, col + 1, data, tab);
            (*result_fn)(tab, tab->n_row_fns, results, buf, cell_buf, data);
        }
        row++;
        tab->rows++;
        free(tok_line);
    }
    free(buf);
    free(cell_buf);
    return 1;
}

inline cell_t
quick_select(cell_t arr[], int n, cell_mode_t mode)
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
