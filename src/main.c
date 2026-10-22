#include <stdio.h>

#include "env.h"

#define UNUSED(x) (void)x

int thunk_0(int y, Environment *env)
{
	return y + env_get_var(env, x).as_var;
}

int thunk_1(int y, Environment *env)
{
	Symbol f1 = env_get_fun(env, thunk_0);
	return f1.as_fun(y, f1.env) * env_get_var(env, x).as_var;
}

int thunk_2(Symbol f, int x, Environment *env)
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
		Symbol f1 = env_bind_fun(env, thunk_0);
		env = env_push(env);
			env_bind_var(env, x, 2);
			env = env_push(env);
				Symbol f2 = env_bind_fun(env, thunk_1);
				env = env_push(env);
					Symbol twice = env_bind_fun(env, thunk_2);
					int res = twice.as_fun(f1, 0, twice.env) + twice.as_fun(f2, 0, twice.env);
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
		Symbol f1 = env_bind_fun(env, thunk_0);

		int res = f1.as_fun(env_get_var(env, y).as_var, f1.env);
		printf("%d\n", res);
	env = env_pop(env);
	env_free(env);
}

/* TODO
 * let c = fun f g -> fun x -> f (g x) in
 * let i = fun x -> x + 1 in
 * let d = c i i in
 * d 2
 */

int main(void)
{
	example_1();
	example_2();
	example_3();
	return 0;
}
