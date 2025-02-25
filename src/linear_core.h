/*
 * Lua Linear core
 *
 * Copyright (C) 2017-2023 Andre Naef
 */


#ifndef _LINEAR_CORE_INCLUDED
#define _LINEAR_CORE_INCLUDED


#include <stdint.h>
#include <lua.h>
#include <cblas.h>


#define LINEAR_VECTOR       "linear.vector"  /* vector metatable */
#define LINEAR_MATRIX       "linear.matrix"  /* matrix metatable */
#define LINEAR_RANDOM       "linear.random"  /* random state */
#define LINEAR_PARAMS_MAX   5                /* maximum number of extra parameters */
#define LINEAR_PARAMS_LAST  {'\0', {0.0}}    /* params termination */


typedef struct linear_data_s {
	size_t   refs;  /* number of references */
} linear_data_t;

typedef struct linear_vector_s {
	size_t          length;  /* length */
	size_t          inc;     /* increment to next value */
	linear_data_t  *data;    /* shared data */
	double         *values;  /* components */
} linear_vector_t;

typedef struct linear_matrix_s {
	size_t          rows;    /* number of rows */
	size_t          cols;    /* number of columns */
	size_t          ld;      /* increment to next major vector */
	CBLAS_ORDER     order;   /* order */
	linear_data_t  *data;    /* shared data */
	double         *values;  /* elements */
} linear_matrix_t;

typedef struct linear_param_s {
	char                 type;   /* see linear_arg_u below */
	union {
		lua_Number   n;      /* default number */
		lua_Integer  i;      /* default integer */
		const char **e;      /* enum; default = e[0] */
		size_t       d;      /* default ddof */
	} def;
} linear_param_t;

typedef union linear_arg {
	lua_Number   n;  /* number */
	lua_Integer  i;  /* integer */
	int          e;  /* enum */
	size_t       d;  /* ddof */
	lua_State   *L;  /* Lua state */
	uint64_t    *r;  /* random state */
} linear_arg_u;


CBLAS_ORDER linear_checkorder(lua_State *L, int index);
void linear_checkargs(lua_State *L, int index, size_t size, linear_param_t *params,
		linear_arg_u *args);
int linear_argerror(lua_State *L, int index, int numok);
int linear_rawgeti(lua_State *L, int index, int n);
int linear_getfield(lua_State *L, int index, const char *key);
#if LUA_VERSION_NUM < 502
void *linear_testudata(lua_State *L, int index, const char *name);
#endif
double linear_random(uint64_t *r);
int linear_comparison_handler(const void *a, const void *b);
linear_vector_t *linear_create_vector(lua_State *L, size_t length);
linear_matrix_t *linear_create_matrix(lua_State *L, size_t rows, size_t cols, CBLAS_ORDER order);
int luaopen_linear(lua_State *L);


#endif /* _LINEAR_CORE_INCLUDED */
