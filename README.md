# Environments in C

Create environments to allow shadowing and capturing variables.

## Example
```ml
let x = 1 in
let y = x + 2 in
let x = 4 in
x + y
```
can be written as
```c
Environment *env = env_new();
env_bind_var(env, x, 1);
env_bind_var(env, y, env_get_var(env, x).as_var + 2);
env = env_push(env);
	env_bind_var(env, x, 4);
	int res = env_get_var(env, x).as_var + env_get_var(env, y).as_var;
	printf("%d\n", res);
env = env_pop(env);
env_free(env);
```

*This is a purely recreational project and should not be expected to be used in serious development.*
