/*
 * lucihttp - HTTP utility library - MIME parser component
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

#ifndef __MULTIPART_PARSER_H
#define __MULTIPART_PARSER_H

#include <stdio.h>
#include <stdbool.h>


#define LH_MP_T_DEFAULT_SIZE_LIMIT 4096

enum lh_mpart_state {
	LH_MP_S_START = 0,
	LH_MP_S_BOUNDARY_START,
	LH_MP_S_HEADER_START,
	LH_MP_S_HEADER,
	LH_MP_S_HEADER_END,
	LH_MP_S_HEADER_VALUE_START,
	LH_MP_S_HEADER_VALUE,
	LH_MP_S_HEADER_VALUE_END,
	LH_MP_S_PART_START,
	LH_MP_S_PART_DATA,
	LH_MP_S_PART_BOUNDARY_START,
	LH_MP_S_PART_BOUNDARY,
	LH_MP_S_PART_BOUNDARY_END,
	LH_MP_S_PART_END,
	LH_MP_S_PART_FINAL,
	LH_MP_S_END,
	LH_MP_S_ERROR
};

enum lh_mpart_token_type {
	LH_MP_T_HEADER_NAME = 0,
	LH_MP_T_HEADER_VALUE,
	LH_MP_T_DATA,
	LH_MP_T_BOUNDARY1,
	LH_MP_T_BOUNDARY2,
	LH_MP_T_BOUNDARY3,
	__LH_MP_T_COUNT
};

enum lh_mpart_flag {
	LH_MP_F_IS_NESTED = (1 << 0),
	LH_MP_F_IN_PART   = (1 << 1),
	LH_MP_F_PAST_NAME = (1 << 2),
	LH_MP_F_MULTILINE = (1 << 3),
	LH_MP_F_BUFFERING = (1 << 4)
};

enum lh_mpart_callback_type {
	LH_MP_CB_BODY_BEGIN,
	LH_MP_CB_PART_INIT,
	LH_MP_CB_HEADER_NAME,
	LH_MP_CB_HEADER_VALUE,
	LH_MP_CB_PART_BEGIN,
	LH_MP_CB_PART_DATA,
	LH_MP_CB_PART_END,
	LH_MP_CB_BODY_END,
	LH_MP_CB_EOF,
	LH_MP_CB_ERROR
};

struct lh_mpart;

typedef bool (*lh_mpart_callback)(struct lh_mpart *,
                                  enum lh_mpart_callback_type,
                                  const char *, size_t, void *);

struct lh_mpart_token
{
	char *value;
	size_t size;
	size_t len;
};

struct lh_mpart
{
	enum lh_mpart_state state;
	char *lookbehind;
	size_t lookbehind_size;
	size_t index;
	size_t offset;
	size_t total;
	size_t size_limit;
	char *error;
	int nesting;
	unsigned int flags;
	struct lh_mpart_token token[__LH_MP_T_COUNT];
	FILE *trace;
	lh_mpart_callback cb;
	void *priv;
};


struct lh_mpart *
lh_mpart_new(FILE *);

void
lh_mpart_set_callback(struct lh_mpart *, lh_mpart_callback, void *);

void
lh_mpart_set_size_limit(struct lh_mpart *, size_t);

char *
lh_mpart_parse_boundary(struct lh_mpart *, const char *, size_t *);

bool
lh_mpart_parse(struct lh_mpart *, const char *, size_t);

void
lh_mpart_free(struct lh_mpart *);


#endif /* __MULTIPART_PARSER_H */
