# Sollux
Sollux is an unoptimized metacompiler, written for personal experimentation.
Sollux accepts as input a *language definition* decorated with *tiles*.
I'm not going to get into the theory here and now (probably later), 
but this is probably the coolest thing I've ever made. 

## Dependencies
Sollux (and the compiler it generates) needs to be linked with libc, [libcirces](https://github.com/gittyhubacc/circes), and [libmf](https://github.com/gittyhubacc/mf).
Circes is a library I wrote that generates parsers and recognizes regular expressions. Mf is a personal util library.

## Examples
A non trivial example.
```
$ ./bin/sollux <samples/fzzbzz fzzbzzc.c                   # feed language definition to sollux, produce source for compiler
used 0mib, 56kib, 58019 bytes
$ clang -c fzzbzzc.c -o fzzbzzc.o                          # link and compile compiler
$ clang fzzbzzc.o -lcirces -lmf -o fzzbzzc
$ echo '<1/3=fizz,5=buzz/20>' | ./fzzbzzc one_to_twenty.c  # use new compiler to take fzzbzz code to C code
$ clang one_to_twenty.c -o one_to_twenty                   # link and compile target code
$ ./one_to_twenty                                          # execute program that we wrote in fzzbzz before compiling to C
1
2
fizz
4
buzz
fizz
7
8
fizz
buzz
11
fizz
13
14
fizzbuzz
16
17
fizz
19
buzz
```

A trivial example.
```
$ ./bin/sollux <samples/hello hello.c         # feed language definition to sollux, produce source for compiler
used 0mib, 19kib, 20076 bytes
$ clang -c hello.c -o hello.o                 # compile and link compiler
$ clang hello.o -lcirces -lmf -o hello
$ echo 'Michael' | ./hello greet_michael.lua  # use new compiler to compile source to target as defined
$ chmod +x greet_michael.lua                  # in this case, compiles "hello" language to lua
$ ./greet_michael.lua
Hello, Michael!
$ echo "BrianK DennisR" | ./hello greet_kr.lua
$ chmod +x greet_kr.lua
$ ./greet_kr.lua
Hello, BrianK!
Hello, DennisR!
```

## Grammar
It's not BNF or EBNF or even formal probably, 
but this is the grammar for the language definitions
sollux accepts:
```
lowchar = 'a' | 'b' | ... | 'z'
capchar = 'A' | 'B' | ... | 'Z'
alpha = lowchar | capchar
digit = '0' | '1' | ... | '9'
alphanumeric = alpha | digit

escape_char = 's' | 't' | 'n' | 'a' | 'd' | 'w'
escape_seq = '\', escape_char
re_p = alphanumeric | escape_seq | '(', re, ')'
re_s = re_p | re_s, '*'
re_t = re_s | re_t, '.' re_t
re = re_t | re, '+', re_t

regex = '`', re, '`'
ascii_block = ''', ascii_char, '''
identifier = alpha | identifier, alphanumeric

param_decl = identifier | identifier, identifier
param_list = param_decl | param_list, ',', param_decl
tile_template_value = identifier | ''', ascii, '''
tile_template = tile_template_value | tile_template, tile_template_value
terminal_binding = identifier, '=', regex
nonterminal_binding = identifier, '(', param_list, ')', '{', tile_template, '}'
terminal_list = terminal_binding | terminal_list, terminal_binding
nonterminal_list = nonterminal_binding | nonterminal_list, nonterminal_binding
binding_list = terminal_list, nonterminal_list
```
