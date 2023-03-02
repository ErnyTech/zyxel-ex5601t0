/*
 * lucihttp - HTTP utility library - urlencoded data parser component
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

#ifndef __URLDECODED_PARSER_H
#define __URLDECODED_PARSER_H

#include <stdio.h>
#include <stdbool.h>


#define LH_UD_T_DEFAULT_SIZE_LIMIT 4096

enum lh_urldec_state {
	LH_UD_S_NAME_START = 0,
	LH_UD_S_NAME,
	LH_UD_S_VALUE_START,
	LH_UD_S_VALUE,
	LH_UD_S_END,
	LH_UD_S_ERROR
};

enum lh_urldec_token_type {
	LH_UD_T_NAME = 0,
	LH_UD_T_VALUE,
	__LH_UD_T_COUNT
};

enum lh_urldec_flag {
	LH_UD_F_GOT_NAME  = (1 << 0),
	LH_UD_F_GOT_VALUE = (1 << 1),
	LH_UD_F_BUFFERING = (1 << 2)
};

enum lh_urldec_callback_type {
	LH_UD_CB_TUPLE,
	LH_UD_CB_NAME,
	LH_UD_CB_VALUE,
	LH_UD_CB_EOF,
	LH_UD_CB_ERROR
};

struct lh_urldec;

typedef bool (*lh_urldec_callback)(struct lh_urldec *,
                                   enum lh_urldec_callback_type,
                                   const char *, size_t, void *);

struct lh_urldec_token
{
	char *value;
	size_t size;
	size_t len;
};

struct lh_urldec
{
	enum lh_urldec_state state;
	size_t offset;
	size_t total;
	size_t size_limit;
	char *error;
	unsigned int flags;
	struct lh_urldec_token token[__LH_UD_T_COUNT];
	FILE *trace;
	lh_urldec_callback cb;
	void *priv;
};


struct lh_urldec *
lh_urldec_new(FILE *);

void
lh_urldec_set_callback(struct lh_urldec *, lh_urldec_callback, void *);

void
lh_urldec_set_size_limit(struct lh_urldec *, size_t);

bool
lh_urldec_parse(struct lh_urldec *, const char *, size_t);

void
lh_urldec_free(struct lh_urldec *);


#endif /* __URLDECODED_PARSER_H */
