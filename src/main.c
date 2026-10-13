#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)x

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

#define ENV_NO_SUCH_SYM(x) \
	(Symbol) { \
		.name = x, \
		.type = SYM_ERR, \
		.as_err = "Could not find symbol", \
		.env = NULL, \
	}

#define ENV_DUPLICATE_SYM(x) \
	(Symbol) { \
		.name = x, \
		.type = SYM_ERR, \
		.as_err = "Cannot define multiple symbols with the same name", \
		.env = NULL, \
	}

typedef struct Environment {
	Symbol *items;
	size_t count;
	size_t capacity;
	struct Environment *next;
} Environment;

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
#define env_bind_fun(env, fun)      env__bind_fun(env, #fun, fun)

#define env_get_var(env, var) env__get_symbol(env, #var, SYM_VAR)
#define env_get_fun(env, fun) env__get_symbol(env, #fun, SYM_FUN)

int f1(int y, Environment *env)
{
	return y + env_get_var(env, x).as_var;
}

int f2(int y, Environment *env)
{
	Symbol f1 = env_get_fun(env, f1);
	return f1.as_fun(y, f1.env) * env_get_var(env, x).as_var;
}

int twice(Symbol f, int x, Environment *env)
{
	UNUSED(env);
	return f.as_fun(f.as_fun(x, f.env), f.env);
}

/*
 * let x = 1 in
 * let f1 y = y + x in
 * let x = 2 in
 * let f2 y = (f1 y) * x in
 * let twice f x = f (f x) in
 * twice f1 0 + twice f2 0
 */
void example_1(void)
{
	Environment *env = env_new();
	env_bind_var(env, x, 1);
	env = env_push(env);
		Symbol f1_f = env_bind_fun(env, f1);
		env = env_push(env);
			env_bind_var(env, x, 2);
			env = env_push(env);
				Symbol f2_f = env_bind_fun(env, f2);
				env = env_push(env);
					Symbol twice_f = env_bind_fun(env, twice);
					int res = twice_f.as_fun(f1_f, 0, twice_f.env) + twice_f.as_fun(f2_f, 0, twice_f.env);
					printf("%d\n", res);
				env = env_pop(env);
			env = env_pop(env);
		env = env_pop(env);
	env = env_pop(env);
	env_free(env);
}

/*
 * let x = 1 in
 * let y = x + 2 in
 * let x = 4 in
 * x + y
 */
void example_2(void)
{
	Environment *env = env_new();
	env_bind_var(env, x, 1);
	env_bind_var(env, y, env_get_var(env, x).as_var + 2);
	env = env_push(env);
		env_bind_var(env, x, 4);
		int res = env_get_var(env, x).as_var + env_get_var(env, y).as_var;
		printf("%d\n", res);
	env = env_pop(env);
	env_free(env);
}

/*
 * let x = 1 in
 * let y = x + 2 in
 * let x = 9 in
 * let f1 y = y + x in
 * f1 y
 */
void example_3(void)
{
	Environment *env = env_new();
	env_bind_var(env, x, 1);
	env_bind_var(env, y, env_get_var(env, x).as_var + 2);
	env = env_push(env);
		env_bind_var(env, x, 9);
		Symbol f1_f = env_bind_fun(env, f1);

		int res = f1_f.as_fun(env_get_var(env, y).as_var, f1_f.env);
		printf("%d\n", res);
	env = env_pop(env);
	env_free(env);
}

int main(void)
{
	example_1();
	example_2();
	example_3();
	return 0;
}
