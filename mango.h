/* global header file */
#ifndef _MANGO_H
#define _MANGO_H

/* constants */
#define HAS_PARSER				1 /* determine if we should include parser utilities */
#define HAS_TOKEN				1 /* token system */
#define HAS_NODE				1 /* node system */
#define HAS_FILE				1 /* file system */
#define HAS_OBJECT				1 /* object system */
#define HAS_VM					1 /* bytecode interpreter */
#define HAS_BYTECODE			1 /* bytecode compiler */
#define HAS_NAMES				1 /* name system */
#define HAS_ERROR				1 /* error handling */
#define HAS_EXTENSION			1 /* stdlib extensions */
#define HAS_ARGPARSE			1 /* argument parser */
#define HAS_RUN					1 /* run a file */

/* includes */
#if HAS_PARSER == 1 /* parser and lexer */
#include "lexer.h"
#include "parser.h"
#endif

#if HAS_TOKEN == 1 /* tokens */
#include "token.h"
#endif

#if HAS_NODE == 1 /* nodes */
#include "node.h"
#endif

#if HAS_FILE == 1 /* file stuff */
#include "file.h"
#endif

#if HAS_OBJECT == 1 /* object system */
#include "object.h"
#include "arrayobject.h"
#include "intobject.h"
#endif

#if HAS_VM == 1 /* bytecode virtual machine */
#include "vm.h"
#include "context.h"
#endif

#if HAS_BYTECODE == 1 /* bytecode compiler */
#include "bytecode.h"
#endif

#if HAS_NAMES == 1 /* variable name system */
#include "names.h"
#endif

#if HAS_ERROR == 1 /* error system */
#include "error.h"
#endif

#if HAS_EXTENSION == 1 /* stdlib extensions */
#include "stringext.h"
#endif

#if HAS_ARGPARSE == 1 /* command line argument parser */
#include "argparse.h"
#endif

#if HAS_RUN == 1 /* run a file */
#include "run.h"
#endif

/* good idea to undefine constants */
#undef HAS_PARSER
#undef HAS_TOKEN
#undef HAS_NODE
#undef HAS_FILE
#undef HAS_OBJECT
#undef HAS_VM
#undef HAS_BYTECODE
#undef HAS_NAMES
#undef HAS_ERROR
#undef HAS_EXTENSION
#undef HAS_ARGPARSE
#undef HAS_RUN

#endif /* _MANGO_H */