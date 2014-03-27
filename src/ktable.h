/*
 * ============================================================================
 *
 *       Filename:  table.h
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
#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "kdm.h"

/* Types */
typedef union _cell {
    uint64_t u;
    int64_t i;
    long double d;
} cell_t;

typedef enum _cell_mode {
    U64 = 0,
    I64 = 1,
    D64 = 2,
} cell_mode_t;

typedef struct _table {
    FILE *fp;
    char *fname;
    FILE *outfp;
    char *outfname;
    char *sep;
    uint64_t rows;
    uint64_t cols;
    uint64_t skiprow;
    uint64_t skipcol;
    cell_mode_t mode;
    void *data;
    int (*skipped_row_fn)(struct _table *, char *);
    int (*skipped_col_fn)(struct _table *, char *);
    void (*row_fn)(struct _table *, char *, cell_t *, size_t);
} table_t;

/* Macros */
#define	destroy_table_t(t) do {                                             \
    if ((t) != NULL) {                                                      \
        if ((t)->fname != NULL) free((t)->fname);                           \
        if ((t)->outfname != NULL) free((t)->outfname);                     \
        if ((t)->sep != NULL) free((t)->sep);                               \
        if ((t)->fp != NULL) fclose((t)->fp);                               \
        if ((t)->outfp != NULL) fclose((t)->outfp);                         \
        if ((t)->data != NULL) free((t)->data);                             \
        free((t));                                                          \
    }} while (0)

#define table_is_valid(t) (                                                 \
        ((t) != NULL) &&                                                    \
        ((t)->fname != NULL) &&                                             \
        ((t)->outfname != NULL) &&                                          \
        ((t)->sep != NULL) &&                                               \
        ((t)->fp != NULL) &&                                                \
        ((t)->outfp != NULL)                                                \
    )

#define cell_elem(cell, mode)                                             \
        ((mode) == D64 ? (cell).d : (mode) == I64 ? (cell).i : (cell).u)

/* Function prototypes */
extern void strtocellt(cell_t *cell, const char *str, char **saveptr,
        cell_mode_t mode);
extern size_t count_columns(const char *row, const char *delim, size_t len);
int iter_table (table_t *tab);

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */
#define ELEM_SWAP(a,b) { register cell_t t=(a);(a)=(b);(b)=t; }
extern cell_t median(cell_t arr[], int n, cell_mode_t mode);

#endif /* TABLE_H */
