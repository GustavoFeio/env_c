let x         = 1        in
let f1 y      = y + x    in
let x         = 2        in
let f2 y      = (f1 y)*x in
let twice f x = f (f x)  in
twice f1 0 + twice f2 0
|> string_of_int
|> print_endline
