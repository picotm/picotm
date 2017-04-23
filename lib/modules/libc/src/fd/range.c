/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

