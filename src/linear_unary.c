/*
 * Lua Linear unary vector functions
 *
 * Copyright (C) 2017-2023 Andre Naef
 */


#include <math.h>
#include <lauxlib.h>
#include "linear_core.h"
#include "linear_unary.h"


static double linear_sum_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_sum(lua_State *L);
static double linear_mean_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_mean(lua_State *L);
static double linear_var_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_var(lua_State *L);
static double linear_std_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_std(lua_State *L);
static double linear_nrm2_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_nrm2(lua_State *L);
static double linear_asum_handler(int size, double *values, int inc, union linear_arg *args);
static int linear_asum(lua_State *L);


static struct linear_param LINEAR_PARAMS_NONE[] = {
	{NULL, '\0', {0.0}}
};
static struct linear_param LINEAR_PARAMS_DDOF[] = {
	{"ddof", 'd', {.defd = 0}},
	{NULL, '\0', {0.0}}
};


int linear_unary (lua_State *L, linear_unary_function f, struct linear_param *params) {
	size_t                 i;
	union linear_arg       args[LINEAR_PARAMS_MAX];
	struct linear_vector  *x, *y;
	struct linear_matrix  *X;

	x = luaL_testudata(L, 1, LINEAR_VECTOR);
	if (x != NULL) {
		/* vector */
		linear_checkargs(L, params, x->length, 2, args);
		lua_pushnumber(L, f(x->length, x->values, x->inc, args));
		return 1;
	}
	X = luaL_testudata(L, 1, LINEAR_MATRIX);
	if (X != NULL) {
		/* matrix-vector */
		y = luaL_checkudata(L, 2, LINEAR_VECTOR);
		if (linear_checkorder(L, 3) == CblasRowMajor) {
			luaL_argcheck(L, y->length == X->rows, 2, "dimension mismatch");
			linear_checkargs(L, params, X->cols, 4, args);
			if (X->order == CblasRowMajor) {
				for (i = 0; i < X->rows; i++) {
					y->values[i * y->inc] = f(X->cols, &X->values[i * X->ld],
							1, args);
				}
			} else {
				for (i = 0; i < X->rows; i++) {
					y->values[i * y->inc] = f(X->cols, &X->values[i], X->ld,
							args);
				}
			}
		} else {
			luaL_argcheck(L, y->length == X->cols, 2, "dimension mismatch");
			linear_checkargs(L, params, X->rows, 4, args);
			if (X->order == CblasColMajor) {
				for (i = 0; i < X->cols; i++) {
					y->values[i * y->inc] = f(X->rows, &X->values[i * X->ld],
							1, args);
				}
			} else {
				for (i = 0; i < X->cols; i++) {
					y->values[i * y->inc] = f(X->rows, &X->values[i], X->ld,
							args);
				}
			}
		}
		return 0;
	}
	return linear_argerror(L, 1, 0);
}

static double linear_sum_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  sum;

	(void)args;
	sum = 0.0;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			sum += x[i];
		}
	} else {
		for (i = 0; i < size; i++) {
			sum += *x;
			x += incx;
		}
	}
	return sum;
}

static int linear_sum (lua_State *L) {
	return linear_unary(L, linear_sum_handler, LINEAR_PARAMS_NONE);
}

static double linear_mean_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  sum;

	(void)args;
	sum = 0.0;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			sum += x[i];
		}
	} else {
		for (i = 0; i < size; i++) {
			sum += *x;
			x += incx;
		}
	}
	return sum / size;
}

static int linear_mean (lua_State *L) {
	return linear_unary(L, linear_mean_handler, LINEAR_PARAMS_NONE);
}

static double linear_var_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  sum, mean;

	sum = 0.0;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			sum += x[i];
		}
		mean = sum / size;
		sum = 0.0;
		for (i = 0; i < size; i++) {
			sum += (x[i] - mean) * (x[i] - mean);
		}
	} else {
		for (i = 0; i < size; i++) {
			sum += *x;
			x += incx;
		}
		mean = sum / size;
		x -= (size_t)size * (size_t)incx;
		sum = 0.0;
		for (i = 0; i < size; i++) {
			sum += (*x - mean) * (*x - mean);
			x += incx;
		}
	}
	return sum / (size - args[0].d);
}

static int linear_var (lua_State *L) {
	return linear_unary(L, linear_var_handler, LINEAR_PARAMS_DDOF);
}

static double linear_std_handler (int size, double *x, int incx, union linear_arg *args) {
	return sqrt(linear_var_handler(size, x, incx, args));
}

static int linear_std (lua_State *L) {
	return linear_unary(L, linear_std_handler, LINEAR_PARAMS_DDOF);
}

static double linear_nrm2_handler (int size, double *x, int incx, union linear_arg *args) {
	(void)args;
	return cblas_dnrm2(size, x, incx);
}

static int linear_nrm2 (lua_State *L) {
	return linear_unary(L, linear_nrm2_handler, LINEAR_PARAMS_NONE);
}

static double linear_asum_handler (int size, double *x, int incx, union linear_arg *args) {
	(void)args;
	return cblas_dasum(size, x, incx);
}

static int linear_asum (lua_State *L) {
	return linear_unary(L, linear_asum_handler, LINEAR_PARAMS_NONE);
}

int linear_open_unary (lua_State *L) {
	static const luaL_Reg FUNCTIONS[] = {
		{ "sum", linear_sum },
		{ "mean", linear_mean },
		{ "var", linear_var },
		{ "std", linear_std },
		{ "nrm2", linear_nrm2 },
		{ "asum", linear_asum },
		{ NULL, NULL }
	};
#if LUA_VERSION_NUM >= 502
	luaL_setfuncs(L, FUNCTIONS, 0);
#else
	const luaL_Reg  *reg;

	for (reg = FUNCTIONS; reg->name; reg++) {
		lua_pushcfunction(L, reg->func);
		lua_setfield(L, -2, reg->name);
	}
#endif
	return 0;
}
