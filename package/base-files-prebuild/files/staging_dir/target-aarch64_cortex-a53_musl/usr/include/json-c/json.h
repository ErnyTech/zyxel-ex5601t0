/*
 * $Id: json.h,v 1.6 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

/**
 * @file
 * @brief A convenience header that may be included instead of other individual ones.
 */
#ifndef _json_h_
#define _json_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "arraylist.h"
#include "debug.h"
#include "json_c_version.h"
#include "json_object.h"
#include "json_object_iterator.h"
#include "json_pointer.h"
#include "json_tokener.h"
#include "json_util.h"
#include "linkhash.h"

#ifdef ZCFG_MEMORY_LEAK_DETECT
#define json_object_new_object(void) _json_object_new_object(__FILE__, __LINE__)
#define json_object_new_array(void) _json_object_new_array(__FILE__, __LINE__)
#define json_object_new_string(strParameter) _zmld_json_object_new_string(strParameter, __FILE__, __LINE__)
#define json_object_from_file(strParameter) _json_object_from_file(strParameter, __FILE__, __LINE__)
#define json_tokener_parse(strParameter) _json_tokener_parse(strParameter, __FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif
