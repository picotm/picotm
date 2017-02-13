/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "range.h"

int
clamp(int min, int n, int max)
{
    return (min < n) ? (n < max ? n : max) : min;
}

long
lclamp(long min, long n, long max)
{
    return (min < n) ? (n < max ? n : max) : min;
}

long long
llclamp(long long min, long long n, long long max)
{
    return (min < n) ? (n < max ? n : max) : min;
}

int
min(int a, int b)
{
    return a < b ? a : b;
}

long
lmin(long a, long b)
{
    return a < b ? a : b;
}

long long
llmin(long long a, long long b)
{
    return a < b ? a : b;
}

int
max(int a, int b)
{
    return a > b ? a : b;
}

long
lmax(long a, long b)
{
    return a > b ? a : b;
}

long long
llmax(long long a, long long b)
{
    return a > b ? a : b;
}

