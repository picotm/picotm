/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BASE64_H
#define BASE64_H

unsigned char *
encode64(const unsigned char *in, size_t inlen, size_t *outlen);

unsigned char *
decode64(const unsigned char *in, size_t inlen, size_t *outlen);

#endif

