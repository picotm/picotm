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

#ifndef RANGE_H
#define RANGE_H

int
clamp(int min, int n, int max);

long
lclamp(long min, long n, long max);

long long
llclamp(long long min, long long n, long long max);

int
max(int a, int b);

long
lmax(long a, long b);

long long
llmax(long long a, long long b);

int
min(int a, int b);

long
lmin(long a, long b);

long long
llmin(long long a, long long b);

#endif

