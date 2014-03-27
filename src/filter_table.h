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
#include <assert.h>
#include "ktable.h"
#include "kdm.h"

#define	FT_BUFFSIZE 1<<14

typedef struct _ft {
    cell_t threshold;
} ft_t;

#endif /* FILTER_TABLE_H */
