/*
 * ============================================================================
 *
 *       Filename:  filter_table.h
 *
 *    Description:  filter_table header
 *
 *        Version:  1.0
 *        Created:  14/03/14 14:49:07
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef FILTER_TABLE_H
#define FILTER_TABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#define	FT_CELL_TYPE uint64_t
#define FT_CELL_STRTO_FN(ptr, endptr) strtoull(ptr, endptr, 10)

#define	FT_BUFFSIZE 1<<14

typedef struct _table {
    FILE *fp;
    char *fname;
    FILE *outfp;
    char *outfname;
    int64_t skiprow;
    int64_t skipcol;
    FT_CELL_TYPE threshold;
    char *sep;
    int (*filter_fn)(FT_CELL_TYPE *, FT_CELL_TYPE, size_t);
} table_t;


#define	destroy_table_t(t) do {                                             \
    if ((t) != NULL) {                                                      \
        if ((t)->fname != NULL) free((t)->fname);                           \
        if ((t)->outfname != NULL) free((t)->outfname);                        \
        if ((t)->sep != NULL) free((t)->sep);                               \
        if ((t)->fp != NULL) fclose((t)->fp);                               \
        if ((t)->outfp != NULL) fclose((t)->outfp);                         \
        free((t));                                                          \
    }} while (0)

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */

#define ELEM_SWAP(a,b) { register FT_CELL_TYPE t=(a);(a)=(b);(b)=t; }


static inline FT_CELL_TYPE quick_select(FT_CELL_TYPE arr[], int n)
{
    int low, high;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

    /* Swap low item (now in position middle) into position (low+1) */
    ELEM_SWAP(arr[middle], arr[low+1]) ;

    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (arr[low] > arr[ll]) ;
        do hh--; while (arr[hh]  > arr[low]) ;

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

#undef ELEM_SWAP

#endif /* FILTER_TABLE_H */
