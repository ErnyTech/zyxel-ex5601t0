/*
 * lucihttp - HTTP utility library - utility functions
 *
 * Copyright 2018 Jo-Philipp Wich <jo@mein.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __UTILS_H
#define __UTILS_H

#include <stddef.h>

enum lh_urlencode_flags {
	LH_URLENCODE_FULL       = (1 << 0),
	LH_URLENCODE_IF_NEEDED  = (1 << 1),
	LH_URLENCODE_SPACE_PLUS = (1 << 2)
};

enum lh_urldecode_flags {
	LH_URLDECODE_STRICT     = (1 << 0),
	LH_URLDECODE_IF_NEEDED  = (1 << 1),
	LH_URLDECODE_KEEP_PLUS  = (1 << 2),
	LH_URLDECODE_PLUS       = (1 << 3)
};

char *lh_urlencode(const char *, size_t, size_t *, unsigned int);
char *lh_urldecode(const char *, size_t, size_t *, unsigned int);

char *lh_header_attribute(const char *, size_t, const char *, size_t *);

#endif /* __UTILS_H */
