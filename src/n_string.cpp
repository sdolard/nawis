/*
 *  n_string.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 04/02/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "n_string.h"


static int compare_right(const QString & a, int ia, const QString & b, int ib)
{
    int bias = 0;

    /* The longest run of digits wins.  That aside, the greatest
	 value wins, but we can't know that it will until we've scanned
	 both numbers to know that they have the same magnitude, so we
	 remember it in BIAS. */
    for (;; ia++, ib++) {
        if (ia >= a.size() &&  ib >= b.size())
            return bias;
        if (ia >= a.size())
            return -1;
        if (ib >= b.size())
            return +1;
        if (!a[ia].isNumber() && !b[ib].isNumber())
            return bias;
        if (!a[ia].isNumber())
            return -1;
        if (!b[ib].isNumber())
            return +1;
        if (a[ia].digitValue() < b[ib].digitValue()) {
            if (!bias)
                bias = -1;
        } else if (a[ia].digitValue() > b[ib].digitValue()) {
            if (!bias)
                bias = +1;
        }
    }

    return 0;
}


static int compare_left(const QString & a, int ia, const QString & b, int ib)
{
    /* Compare two left-aligned numbers: the first to have a
	 different value wins. */
    for (;; ia++, ib++) {
        if (ia >= a.size() &&  ib >= b.size())
            return 0;
        if (ia >= a.size())
            return -1;
        if (ib >= b.size())
            return +1;
        if (!a[ia].isNumber() && !b[ib].isNumber())
            return 0;
        if (!a[ia].isNumber())
            return -1;
        if (!b[ib].isNumber())
            return +1;
        if (a[ia].digitValue() < b[ib].digitValue())
            return -1;
        if (a[ia].digitValue() > b[ib].digitValue())
            return +1;
    }

    return 0;
}


int NString_n::naturalCompare(const QString & a, const QString & b, Qt::CaseSensitivity cs)
{
    int ia, ib;
    QChar ca, cb;
    int fractional, result;

    ia = ib = 0;
    while (true) {
        if (ia >= a.size() &&  ib >= b.size())
            return 0;
        if (ia >= a.size())
            return -1;
        if (ib >= b.size())
            return +1;

        ca = a[ia]; cb = b[ib];

        /* skip over leading spaces or zeros */
        while (ia < (a.size() - 1) && ca.isSpace())
            ca = a[++ia];
        while (ib < (b.size() - 1)  && cb.isSpace())
            cb = b[++ib];


        /* process run of digits */
        if (ca.isNumber()  &&  cb.isNumber()) {
            fractional = (ca == '0' || cb == '0');

            if (fractional) {
                if ((result = compare_left(a, ia, b, ib)) != 0)
                    return result;
            } else {
                if ((result = compare_right(a, ia, b, ib)) != 0)
                    return result;
            }
        }


        if ( cs == Qt::CaseInsensitive) {
            ca = ca.toUpper();
            cb = cb.toUpper();
        }

        if (ca < cb)
            return -1;
        else if (ca > cb)
            return +1;

        ++ia; ++ib;
    }
}

