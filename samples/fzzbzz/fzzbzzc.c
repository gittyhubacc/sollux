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

#define open_dg (1)
	int open_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string open_term = unwrap(S("`<`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, open_term);
	list_append(&terminals, node);

#define close_dg (1)
	int close_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string close_term = unwrap(S("`>`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, close_term);
	list_append(&terminals, node);

#define equals_dg (1)
	int equals_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string equals_term = unwrap(S("`=`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, equals_term);
	list_append(&terminals, node);

#define slash_dg (1)
	int slash_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string slash_term = unwrap(S("`/`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, slash_term);
	list_append(&terminals, node);

#define comma_dg (1)
	int comma_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string comma_term = unwrap(S("`,`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, comma_term);
	list_append(&terminals, node);

#define int_dg (1)
	int int_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string int_term = unwrap(S("`\\d*`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, int_term);
	list_append(&terminals, node);

#define string_dg (1)
	int string_v = terminal_cnt++;
	node = make(&root, list, 1);
	node->data = make(&root, nfa, 1);
	string string_term = unwrap(S("`\\a.\\w*`"));
	*(nfa *)node->data = circes_regex(&root, rtmp, string_term);
	list_append(&terminals, node);

#define over_dg (1)
	int over_v = terminal_cnt++;


	tok_cnt = 0;
	til_cnt = 0;
	arg_pos = 0;
	tok_list = NULL;
	til_list = NULL;
#ifndef lower_dg
#define lower_dg
	int lower_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef open_dg
#define open_dg
	int open_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef open_pos_dg
#define open_pos_dg
	int open_pos;
#endif

	open_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(open_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef int_dg
#define int_dg
	int int_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef i_pos_dg
#define i_pos_dg
	int i_pos;
#endif

	i_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(int_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = i_pos;
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
                 .head = T(lower_v),
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
#ifndef upper_dg
#define upper_dg
	int upper_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef int_dg
#define int_dg
	int int_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef i_pos_dg
#define i_pos_dg
	int i_pos;
#endif

	i_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(int_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef close_dg
#define close_dg
	int close_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef close_pos_dg
#define close_pos_dg
	int close_pos;
#endif

	close_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(close_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = i_pos;
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
                 .head = T(upper_v),
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
#ifndef jules_dg
#define jules_dg
	int jules_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef int_dg
#define int_dg
	int int_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef i_pos_dg
#define i_pos_dg
	int i_pos;
#endif

	i_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(int_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef equals_dg
#define equals_dg
	int equals_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef equals_pos_dg
#define equals_pos_dg
	int equals_pos;
#endif

	equals_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(equals_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef string_dg
#define string_dg
	int string_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef s_pos_dg
#define s_pos_dg
	int s_pos;
#endif

	s_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(string_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		state |= (i % '"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = i_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("' == 0) << checks;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		if (state & (1 << (checks++))) {\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'			printf(\"'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = s_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'\");\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'                       checked = 1;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		}\n'"));
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
                 .head = T(jules_v),
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
#ifndef jules_dg
#define jules_dg
	int jules_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef jules_dg
#define jules_dg
	int jules_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef r_pos_dg
#define r_pos_dg
	int r_pos;
#endif

	r_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(jules_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef comma_dg
#define comma_dg
	int comma_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef comma_pos_dg
#define comma_pos_dg
	int comma_pos;
#endif

	comma_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(comma_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef int_dg
#define int_dg
	int int_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef i_pos_dg
#define i_pos_dg
	int i_pos;
#endif

	i_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(int_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef equals_dg
#define equals_dg
	int equals_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef equals_pos_dg
#define equals_pos_dg
	int equals_pos;
#endif

	equals_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(equals_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef string_dg
#define string_dg
	int string_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef s_pos_dg
#define s_pos_dg
	int s_pos;
#endif

	s_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(string_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = r_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		state |= (i % '"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = i_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("' == 0) << checks;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		if (state & (1 << (checks++))) {\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'			printf(\"'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = s_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'\");\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'                       checked = 1;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		}\n'"));
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
                 .head = T(jules_v),
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
#ifndef fizzbuzz_dg
#define fizzbuzz_dg
	int fizzbuzz_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef lower_dg
#define lower_dg
	int lower_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef l_pos_dg
#define l_pos_dg
	int l_pos;
#endif

	l_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(lower_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef slash_dg
#define slash_dg
	int slash_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef slash_pos_dg
#define slash_pos_dg
	int slash_pos;
#endif

	slash_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(slash_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef jules_dg
#define jules_dg
	int jules_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef r_pos_dg
#define r_pos_dg
	int r_pos;
#endif

	r_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(jules_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef slash_dg
#define slash_dg
	int slash_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef slash_pos_dg
#define slash_pos_dg
	int slash_pos;
#endif

	slash_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(slash_v);
	list_append(&tok_list, node);
	tok_cnt++;

#ifndef upper_dg
#define upper_dg
	int upper_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef u_pos_dg
#define u_pos_dg
	int u_pos;
#endif

	u_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(upper_v);
	list_append(&tok_list, node);
	tok_cnt++;

	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'#include <stdio.h>\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'int main(int argc, char **argv)\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'{\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'	for (int i ='"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = l_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'; i <= '"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = u_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'; i++) {\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		int state = 0;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'               int checks = 0;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'               int checked = 0;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = r_pos;
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'               if (!checked) {\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'			printf(\"%i\", i);\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'               }\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'		printf(\"\\n\");\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'	}\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'       return 0;\n'"));
	list_append(&til_list, node);
	til_cnt++;


	node = make(&root, list, 1);
	node->data = make(&root, tile_val, 1);
	((tile_val *)node->data)->type = TILE_LITERAL;
	((tile_val *)node->data)->literal = unwrap(S("'}\n'"));
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
                 .head = T(fizzbuzz_v),
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
#ifndef start_dg
#define start_dg
	int start_v = terminal_cnt + (nonterminal_cnt++);
#endif

#ifndef fizzbuzz_dg
#define fizzbuzz_dg
	int fizzbuzz_v = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef fizzbuzz_pos_dg
#define fizzbuzz_pos_dg
	int fizzbuzz_pos;
#endif

	fizzbuzz_pos = arg_pos++;
	node = make(&root, list, 1);
	node->data = make(&root, token, 1);
	*((token *)node->data) = T(fizzbuzz_v);
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
	((tile_val *)node->data)->type = TILE_ARG;
	((tile_val *)node->data)->arg = fizzbuzz_pos;
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
                 .head = T(start_v),
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
