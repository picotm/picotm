/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

