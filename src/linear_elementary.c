/*
 * Lua Linear elementary functions
 *
 * Copyright (C) 2017-2023 Andre Naef
 */


#include <stdlib.h>
#include <math.h>
#include <lauxlib.h>
#include "linear_core.h"
#include "linear_elementary.h"


static void linear_inc_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_inc(lua_State *L);
static void linear_scal_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_scal(lua_State *L);
static void linear_pow_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_pow(lua_State *L);
static void linear_exp_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_exp(lua_State *L);
static void linear_log_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_log(lua_State *L);
static void linear_sgn_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_sgn(lua_State *L);
static void linear_abs_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_abs(lua_State *L);
static void linear_logistic_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_logistic(lua_State *L);
static void linear_tanh_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_tanh(lua_State *L);
static void linear_apply_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_apply(lua_State *L);
static void linear_set_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_set(lua_State *L);
static void linear_uniform_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_uniform(lua_State *L);
static void linear_normal_handler(int size, double *x, int incx, union linear_arg *args);
static int linear_normal(lua_State *L);


static struct linear_param LINEAR_PARAMS_NONE[] = {
	{NULL, '\0', {0.0}}
};
static struct linear_param LINEAR_PARAMS_ALPHA[] = {
	{"alpha", 'n', {1.0}},
	{NULL, '\0', {0.0}}
};

static __thread lua_State  *linear_TL;


int linear_elementary (lua_State *L, linear_elementary_function f, struct linear_param *params) {
	int                    isnum;
	size_t                 i;
	double                 n;
	union linear_arg       args[LINEAR_PARAMS_MAX];
	struct linear_vector  *x;
	struct linear_matrix  *X;

	linear_checkargs(L, params, 0, 2, args);
#if LUA_VERSION_NUM >= 502
	n = lua_tonumberx(L, 1, &isnum);
#else
	isnum = lua_isnumber(L, 1);
	n = lua_tonumber(L, 1);
#endif
	if (isnum) {
		f(1, &n, 1, args);
		lua_pushnumber(L, n);
		return 1;
	}
	x = luaL_testudata(L, 1, LINEAR_VECTOR);
	if (x != NULL) {
		f(x->length, x->values, x->inc, args);
		return 0;
	}
	X = luaL_testudata(L, 1, LINEAR_MATRIX);
	if (X != NULL) {
		if (X->order == CblasRowMajor) {
			if (X->cols == X->ld && X->rows * X->cols <= INT_MAX) {
				f(X->rows * X->cols, X->values, 1, args);
			} else {
				for (i = 0; i < X->rows; i++) {
					f(X->cols, &X->values[i * X->ld], 1, args);
				}
			}
		} else {
			if (X->rows == X->ld && X->cols * X->rows <= INT_MAX) {
				f(X->cols * X->rows, X->values, 1, args);
			} else {
				for (i = 0; i < X->cols; i++) {
					f(X->rows, &X->values[i * X->ld], 1, args);
				}
			}
		}
		return 0;
	}
	return linear_argerror(L, 0, 1);
}

static void linear_inc_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  alpha;

	alpha = args[0].n;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			x[i] += alpha;
		}
	} else {
		for (i = 0; i < size; i++) {
			*x += alpha;
			x += incx;
		}
	}
}

static int linear_inc (lua_State *L) {
	return linear_elementary(L, linear_inc_handler, LINEAR_PARAMS_ALPHA);
}

static void linear_scal_handler (int size, double *x, int incx, union linear_arg *args) {
	cblas_dscal(size, args[0].n, x, incx);
}

static int linear_scal (lua_State *L) {
	return linear_elementary(L, linear_scal_handler, LINEAR_PARAMS_ALPHA);
}

static void linear_pow_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  alpha;

	alpha = args[0].n;
	if (alpha == -1.0) {
		if (incx == 1) {
			for (i = 0; i < size; i++) {
				x[i] = 1 / x[i];
			}
		} else {
			for (i = 0; i < size; i++) {
				*x = 1 / *x;
				x += incx;
			}
		}
	} else if (alpha == 0.0) {
		if (incx == 1) {
			for (i = 0; i < size; i++) {
				x[i] = 1.0;
			}
		} else {
			for (i = 0; i < size; i++) {
				*x = 1.0;
				x += incx;
			}
		}
	} else if (alpha == 0.5) {
		for (i = 0; i < size; i++) {
			*x = sqrt(*x);
			x += incx;
		}
	} else if (alpha != 1.0) {
		for (i = 0; i < size; i++) {
			*x = pow(*x, alpha);
			x += incx;
		}
	}
}

static int linear_pow (lua_State *L) {
	return linear_elementary(L, linear_pow_handler, LINEAR_PARAMS_ALPHA);
}

static void linear_exp_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			x[i] = exp(x[i]);
		}
	} else {
		for (i = 0; i < size; i++) {
			*x = exp(*x);
			x += incx;
		}
	}
}

static int linear_exp (lua_State *L) {
	return linear_elementary(L, linear_exp_handler, LINEAR_PARAMS_NONE);
}

static void linear_log_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			x[i] = log(x[i]);
		}
	} else {
		for (i = 0; i < size; i++) {
			*x = log(*x);
			x += incx;
		}
	}
}

static int linear_log (lua_State *L) {
	return linear_elementary(L, linear_log_handler, LINEAR_PARAMS_NONE);
}

static void linear_sgn_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		if (*x > 0) {
			*x = 1;
		} else if (*x < 0) {
			*x = -1;
		}
		x += incx;
	}
}

static int linear_sgn (lua_State *L) {
	return linear_elementary(L, linear_sgn_handler, LINEAR_PARAMS_NONE);
}

static void linear_abs_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		*x = fabs(*x);
		x += incx;
	}
}

static int linear_abs (lua_State *L) {
	return linear_elementary(L, linear_abs_handler, LINEAR_PARAMS_NONE);
}

static void linear_logistic_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		*x = 1.0 / (1.0 + exp(-*x));
		x += incx;
	}
}

static int linear_logistic (lua_State *L) {
	return linear_elementary(L, linear_logistic_handler, LINEAR_PARAMS_NONE);
}

static void linear_tanh_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		*x = tanh(*x);
		x += incx;
	}
}

static int linear_tanh (lua_State *L) {
	return linear_elementary(L, linear_tanh_handler, LINEAR_PARAMS_NONE);
}

static void linear_apply_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		lua_pushvalue(linear_TL, -1);
		lua_pushnumber(linear_TL, *x);
		lua_call(linear_TL, 1, 1);
		*x = lua_tonumber(linear_TL, -1);
		x += incx;
		lua_pop(linear_TL, 1);
	}
}

static int linear_apply (lua_State *L) {
	luaL_checktype(L, 2, LUA_TFUNCTION);
	lua_settop(L, 2);
	linear_TL = L;
	return linear_elementary(L, linear_apply_handler, LINEAR_PARAMS_NONE);
}

static void linear_set_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  alpha;

	alpha = args[0].n;
	if (incx == 1) {
		for (i = 0; i < size; i++) {
			x[i] = alpha;
		}
	} else {
		for (i = 0; i < size; i++) {
			*x = alpha;
			x += incx;
		}
	}
}

static int linear_set (lua_State *L) {
	return linear_elementary(L, linear_set_handler, LINEAR_PARAMS_ALPHA);
}

static void linear_uniform_handler (int size, double *x, int incx, union linear_arg *args) {
	int  i;

	(void)args;
	for (i = 0; i < size; i++) {
		*x = random() / (RAND_MAX + 1.0);
		x += incx;
	}
}

static int linear_uniform (lua_State *L) {
	return linear_elementary(L, linear_uniform_handler, LINEAR_PARAMS_NONE);
}

static void linear_normal_handler (int size, double *x, int incx, union linear_arg *args) {
	int     i;
	double  u1, u2, r, s, c;

	(void)args;

	/* Box-Muller transform */
	for (i = 0; i < size - 1; i += 2) {
		u1 = (random() + 1.0) / (RAND_MAX + 1.0);
		u2 = (random() + 1.0) / (RAND_MAX + 1.0);
		r = sqrt(-2.0 * log(u1));
		sincos(2 * M_PI * u2, &s, &c);
		*x = r * c;
		x += incx;
		*x = r * s;
		x += incx;
	}
	if (i < size) {
		u1 = (random() + 1.0) / (RAND_MAX + 1.0);
		u2 = (random() + 1.0) / (RAND_MAX + 1.0);
		*x = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
	}
}

static int linear_normal (lua_State *L) {
	return linear_elementary(L, linear_normal_handler, LINEAR_PARAMS_NONE);
}

int linear_open_elementary (lua_State *L) {
	static const luaL_Reg FUNCTIONS[] = {
		{ "inc", linear_inc },
		{ "scal", linear_scal },
		{ "pow", linear_pow },
		{ "exp", linear_exp },
		{ "log", linear_log },
		{ "sgn", linear_sgn },
		{ "abs", linear_abs },
		{ "logistic", linear_logistic },
		{ "tanh", linear_tanh },
		{ "apply", linear_apply },
		{ "set", linear_set },
		{ "uniform", linear_uniform },
		{ "normal", linear_normal },
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
