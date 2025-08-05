#include <mf/string.h>
#include <mf/memory.h>
#include <circes.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct program_input {
	string program;
} program_input;

typedef struct program_output {
	int bytes_used;
	string program;
} program_output;

static string unwrap(string some){
	string result = some;
	result.addr += 1;
	result.len -= 2;
	return result;
}
static const int part_sz = kibibyte / 4;
static program_input read_input(arena *a, int argc, char **argv)
{
	char c = 0;
	int parts_read = 0;
	int bytes_read = 0;
	char *buffer = make(a, char, part_sz);

	while ((c = fgetc(stdin)) != EOF) {
		buffer[(parts_read * part_sz) + bytes_read++] = c;
		if (bytes_read >= part_sz) {
			steal(a, char, part_sz);
			bytes_read = 0;
			parts_read++;
		}
	}
	return (program_input){
		.program = (string){buffer, parts_read * part_sz + bytes_read},
	};
}

static int write_output(int argc, char **argv, program_output output)
{
	char *path = "program.c";
	if (argc >= 2) {
		path = argv[1];
		}
	FILE *out = fopen(path, "w");
	fwrite(output.program.addr, sizeof(char), output.program.len, out);
	fclose(out);
	return 0;
}
int main(int argc, char **argv)
{
	int root_sz = gigabyte;
	char *mem = malloc(root_sz);
	arena root = make_arena_ptr(mem, root_sz);
	arena rtmp = reserve(root, char, root_sz / 2);
	nfa_init_builtins(&root);
	program_input input = read_input(&root, argc, argv);
	int terminal_cnt = 0;
	int nonterminal_cnt = 0;
	int rule_cnt = 0;
	int tok_cnt = 0;
	int til_cnt = 0;
	int arg_pos = 0;
	token_string rule_body;
	tile tile_body;
	list *tok_list = NULL;
	list *til_list = NULL;
	list *terminals = NULL;
	list *rules = NULL;
	list *node;

#define name_dg (1)
	int name_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string name_term = unwrap(S("`\\a.\\a*`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, name_term);
	list_append(&terminals, node);

#define over_dg (1)
	int over_v = terminal_cnt++;


	tok_cnt = 0;
	til_cnt = 0;
	arg_pos = 0;
	tok_list = NULL;
	til_list = NULL;
#ifndef greeting_dg
#define greeting_dg
	int greeting_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef name_dg
#define name_dg
	int name_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef n_pos_dg
#define n_pos_dg
	int n_pos;
#endif

	n_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(name_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'print(\"Hello, '"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = n_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'!\")'"));
	list_append(&til_list, node);
	til_cnt++;
        tile_body.addr = make(&root, tile_val, til_cnt);
        tile_body.len = 0;
        node = til_list;
        while (node) {
                tile_body.addr[tile_body.len++] = *((tile_val *)node->data);
                node = node->next;
        }
        rule_body.addr = make(&root, token, tok_cnt);
        rule_body.len = 0;
        node = tok_list;
        while (node) {
                rule_body.addr[rule_body.len++] = *((token *)node->data);
                node = node->next;
        }
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
                 .head = T(greeting_v),
                 .body = rule_body,
                 .tile = tile_body,
        };
        list_append(&rules, node);
        rule_cnt++;

	tok_cnt = 0;
	til_cnt = 0;
	arg_pos = 0;
	tok_list = NULL;
	til_list = NULL;
#ifndef greeting_list_dg
#define greeting_list_dg
	int greeting_list_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef greeting_dg
#define greeting_dg
	int greeting_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef g_pos_dg
#define g_pos_dg
	int g_pos;
#endif

	g_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(greeting_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = g_pos;
	list_append(&til_list, node);
	til_cnt++;
        tile_body.addr = make(&root, tile_val, til_cnt);
        tile_body.len = 0;
        node = til_list;
        while (node) {
                tile_body.addr[tile_body.len++] = *((tile_val *)node->data);
                node = node->next;
        }
        rule_body.addr = make(&root, token, tok_cnt);
        rule_body.len = 0;
        node = tok_list;
        while (node) {
                rule_body.addr[rule_body.len++] = *((token *)node->data);
                node = node->next;
        }
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
                 .head = T(greeting_list_v),
                 .body = rule_body,
                 .tile = tile_body,
        };
        list_append(&rules, node);
        rule_cnt++;

	tok_cnt = 0;
	til_cnt = 0;
	arg_pos = 0;
	tok_list = NULL;
	til_list = NULL;
#ifndef greeting_list_dg
#define greeting_list_dg
	int greeting_list_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef greeting_list_dg
#define greeting_list_dg
	int greeting_list_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef l_pos_dg
#define l_pos_dg
	int l_pos;
#endif

	l_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(greeting_list_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef greeting_dg
#define greeting_dg
	int greeting_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef g_pos_dg
#define g_pos_dg
	int g_pos;
#endif

	g_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(greeting_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = l_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = g_pos;
	list_append(&til_list, node);
	til_cnt++;
        tile_body.addr = make(&root, tile_val, til_cnt);
        tile_body.len = 0;
        node = til_list;
        while (node) {
                tile_body.addr[tile_body.len++] = *((tile_val *)node->data);
                node = node->next;
        }
        rule_body.addr = make(&root, token, tok_cnt);
        rule_body.len = 0;
        node = tok_list;
        while (node) {
                rule_body.addr[rule_body.len++] = *((token *)node->data);
                node = node->next;
        }
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
                 .head = T(greeting_list_v),
                 .body = rule_body,
                 .tile = tile_body,
        };
        list_append(&rules, node);
        rule_cnt++;

	tok_cnt = 0;
	til_cnt = 0;
	arg_pos = 0;
	tok_list = NULL;
	til_list = NULL;
#ifndef hello_lang_dg
#define hello_lang_dg
	int hello_lang_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef greeting_list_dg
#define greeting_list_dg
	int greeting_list_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef l_pos_dg
#define l_pos_dg
	int l_pos;
#endif

	l_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(greeting_list_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef over_dg
#define over_dg
	int over_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef over_pos_dg
#define over_pos_dg
	int over_pos;
#endif

	over_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(over_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'#!/usr/bin/lua\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = l_pos;
	list_append(&til_list, node);
	til_cnt++;
        tile_body.addr = make(&root, tile_val, til_cnt);
        tile_body.len = 0;
        node = til_list;
        while (node) {
                tile_body.addr[tile_body.len++] = *((tile_val *)node->data);
                node = node->next;
        }
        rule_body.addr = make(&root, token, tok_cnt);
        rule_body.len = 0;
        node = tok_list;
        while (node) {
                rule_body.addr[rule_body.len++] = *((token *)node->data);
                node = node->next;
        }
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
                 .head = T(hello_lang_v),
                 .body = rule_body,
                 .tile = tile_body,
        };
        list_append(&rules, node);
        rule_cnt++;

	grammar grmr;
	grmr.lexer.len = 0;
	grmr.lexer.addr = make(&root, nfa, terminal_cnt);
	node = terminals;
	while (node) {
	grmr.lexer.addr[grmr.lexer.len++] = *(nfa *)node->data;
		node = node->next;
	}
	grmr.rule_cnt = 0;
	grmr.rules = make(&root, pda_rule, rule_cnt);
	node = rules;
	while (node) {
		grmr.rules[grmr.rule_cnt++] = *(pda_rule *)node->data;
		node = node->next;
	}
	grmr.start = grmr.rules + (grmr.rule_cnt - 1);
	token_string tokens = circes_lex(&root, rtmp, &grmr, input.program);
	parse_tree *tree = circes_parse(&root, rtmp, &grmr, tokens);
	string program = circes_select(&root, rtmp, &grmr, tree);
	program_output output;
	output.program = program;
	output.bytes_used = bytes_used(root);
	return write_output(argc, argv, output);
}
