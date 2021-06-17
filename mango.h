/* global header file */
#ifndef _MANGO_H
#define _MANGO_H

/* constants */
#define HAS_PARSER				1 /* determine if we should include parser utilities */
#define HAS_TOKEN				1 /* token system */
#define HAS_NODE				1 /* node system */
#define HAS_FILE				1 /* file system */
#define HAS_OBJECT				1 /* object system */
#define HAS_VM					0 /* bytecode interpreter */
#define HAS_BYTECODE			0 /* bytecode compiler */
#define HAS_NAMES				1 /* name system */
#define HAS_ERROR				1 /* error handling */

/* includes */
#if HAS_PARSER == 1 /* parser and lexer */
#include "lexer.h"
#include "parser.h"
#endif

#if HAS_TOKEN == 1 /* tokens */
#include "token.h"
#endif

#if HAS_NODE == 1
#include "node.h"
#endif

#if HAS_FILE == 1
#include "file.h"
#endif

#if HAS_OBJECT == 1
#include "object.h"
#endif

#if HAS_VM == 1
#include "vm.h"
#endif

#if HAS_BYTECODE == 1
#include "bytecode.h"
#endif

#if HAS_NAMES == 1
#include "names.h"
#endif

#if HAS_ERROR == 1
#include "error.h"
#endif

#endif /* _MANGO_H */