# Sollux
Sollux is an unoptimized metacompiler, written for personal experimentation.
Sollux accepts as input a *language definition* decorated with *tiles*.
I'm not going to get into the theory here and now, 
but there's a lot that excites me about this project.

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

## Examples

