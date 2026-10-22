#ifndef _ENV_C
#define _ENV_C

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define da_append(xs, x)                                                               \
	do {                                                                               \
		if ((xs)->count >= (xs)->capacity) {                                           \
			(xs)->capacity = (xs)->capacity == 0 ? 16 : (xs)->capacity * 2;            \
			(xs)->items = realloc((xs)->items, sizeof((xs)->items[0])*(xs)->capacity); \
			assert((xs)->items != NULL && "Could not reallocate memory");              \
		}                                                                              \
		(xs)->items[(xs)->count++] = x;                                                \
	} while (0)

typedef int(*Fn_t)();

typedef enum {
	SYM_FUN,
	SYM_VAR,
	SYM_ERR,
	SYM_MAX,
} SymType;

struct Environment;

typedef struct {
	const char *name;
	SymType type;
	union {
		Fn_t as_fun;
		int  as_var;
		const char *as_err;
	};
	struct Environment *env;
} Symbol;

typedef struct Environment {
	Symbol *items;
	size_t count;
	size_t capacity;
	struct Environment *next;
} Environment;

#define ENV_NO_SUCH_SYM(x)                 \
	(Symbol) {                             \
		.name = x,                         \
		.type = SYM_ERR,                   \
		.as_err = "Could not find symbol", \
		.env = NULL,                       \
	}

#define ENV_DUPLICATE_SYM(x)                                           \
	(Symbol) {                                                         \
		.name = x,                                                     \
		.type = SYM_ERR,                                               \
		.as_err = "Cannot define multiple symbols with the same name", \
		.env = NULL,                                                   \
	}

Environment *env_new(void)
{
	Environment *env = (void *)malloc(sizeof(*env));
	assert(env != NULL && "Could not allocate new environment.");
	env->items    = NULL;
	env->count    = 0;
	env->capacity = 0;
	env->next     = NULL;
	return env;
}

Environment *env_push(Environment *env)
{
	Environment *new_env = (void *)malloc(sizeof(*new_env));
	assert(new_env != NULL && "Could not allocate new environment.");
	memset((void *)new_env, 0, sizeof(*new_env));
	new_env->next = env;
	return new_env;
}

Environment *env_pop(Environment *env)
{
	if (env == NULL) return NULL;

	Environment *next = env->next;
	if (env->items != NULL) {
		free(env->items);
	}
	memset((void *)env, 0, sizeof(*env));
	free(env);
	return next;
}

void env_free(Environment *env)
{
	Environment *next = env;
	while (next != NULL) {
		next = env_pop(next);
	}
}

Symbol env__get_symbol(Environment *env, const char *sym_name, SymType type)
{
	Symbol sym;
	bool found = false;
	while (!found && env != NULL) {
		size_t i = 0;
		while (i < env->count && !found) {
			sym = env->items[i];
			if (sym.type == type && strcmp(sym.name, sym_name) == 0) {
				found = true;
			} else {
				i += 1;
			}
		}
		env = env->next;
	}

	if (!found) {
		return ENV_NO_SUCH_SYM(sym_name);
	}
	return sym;
}

Symbol env__get_var(Environment *env, const char *var_name)
{
	Symbol sym = env__get_symbol(env, var_name, SYM_VAR);
	return sym;
}

Symbol env__get_fun(Environment *env, const char *fun_name)
{
	Symbol sym = env__get_symbol(env, fun_name, SYM_FUN);
	return sym;
}

bool env__sym_is_bound(Environment *env, const char *sym_name)
{
	bool found = false;
	size_t i = 0;
	while (i < env->count && !found) {
		Symbol sym = env->items[i];
		if (strcmp(sym.name, sym_name) == 0) {
			found = true;
		} else {
			i += 1;
		}
	}
	return found;
}

Symbol env__bind_var(Environment *env, const char *var_name, int val)
{
	if (env__sym_is_bound(env, var_name)) {
		return ENV_DUPLICATE_SYM(var_name);
	}

	Symbol sym = {
		.name = var_name,
		.type = SYM_VAR,
		.as_var = val,
		.env = env,
	};
	da_append(env, sym);
	return sym;
}

Symbol env__bind_fun(Environment *env, const char *fun_name, Fn_t fun)
{
	if (env__sym_is_bound(env, fun_name)) {
		return ENV_DUPLICATE_SYM(fun_name);
	}

	Symbol sym = {
		.name = fun_name,
		.type = SYM_FUN,
		.as_fun = fun,
		.env = env,
	};
	da_append(env, sym);
	return sym;
}

#define env_bind_var(env, var, val) env__bind_var(env, #var, val)
#define env_bind_fun(env, fun, val) env__bind_fun(env, #fun, val)

#define env_get_var(env, var) env__get_symbol(env, #var, SYM_VAR)
#define env_get_fun(env, fun) env__get_symbol(env, #fun, SYM_FUN)

#endif // _ENV_C
