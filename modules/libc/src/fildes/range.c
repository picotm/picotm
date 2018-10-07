/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
