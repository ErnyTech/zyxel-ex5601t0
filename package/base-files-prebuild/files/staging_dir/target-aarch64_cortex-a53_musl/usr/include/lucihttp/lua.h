/*
 * lucihttp - HTTP utility library - Lua binding
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

#ifndef __LUA_H
#define __LUA_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LUCIHTTP_META "lucihttp"
#define LUCIHTTP_MPART_META LUCIHTTP_META ".multipart"
#define LUCIHTTP_URLDEC_META LUCIHTTP_META ".urlencoded"

LUALIB_API int luaopen_lucihttp(lua_State *L);

#endif /* __LUA_H */
