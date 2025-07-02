#include <circes.h>

#include <mf/memory.h>
#include <mf/string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct program_input {
        string program;
} program_input;

typedef struct program_output {
        int bytes_used;
        string program;
} program_output;

static const int part_sz = kibibyte / 4;
static program_input read_input(arena *a, int argc, char **argv)
{
        char c = 0;
        int parts_read = 0;
        int bytes_read = 0;
        char *buffer = make(a, char, part_sz);

        // eventually, optionally read from other places
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
        fprintf(stdout, "used %imib, %ikib, %i bytes\n",
                output.bytes_used / mebibyte, output.bytes_used / kibibyte,
                output.bytes_used);
        return 0;
}

typedef enum token_type {
        TTOK_OPNBR, // 0
        TTOK_CLSBR, // 1
        TTOK_OPNPR, // 2
        TTOK_CLSPR, // 3
        TTOK_COMMA, // 4
        TTOK_EQUAL, // 5
        TTOK_REGEX, // 6
        TTOK_ASCII, // 7
        TTOK_IDENT, // 8
        TTOK_OVER,  // 9

        NTOK_START,        // 10
        NTOK_PARAMD,       // 12
        NTOK_PARAML,       // 13
        NTOK_TILE_VAL,     // 14
        NTOK_TILE,         // 15
        NTOK_BINDING,      // 17
        NTOK_BINDING_LIST, // 18
} token_type;

int main(int argc, char **argv)
{
        /*
         * head, program initialization
         */

        int root_sz = gigabyte;
        char *mem = malloc(root_sz);
        arena root = make_arena_ptr(mem, root_sz);

        nfa_init_builtins(&root);
        program_input input = read_input(&root, argc, argv);

        /*
         * body, define grammar
         */

        int terminal_cnt = 0;
        int rule_cnt = 0;
        list *terminals = NULL;
        list *rules = NULL;
        list *node;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("{"));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("}"));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("\\("));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("\\)"));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S(","));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("="));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("`.\\x*.`*"));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("'.\\y*.'*"));
        list_append(&terminals, node);
        terminal_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, S("(\\w+_)*"));
        list_append(&terminals, node);
        terminal_cnt++;

        token paramd_rule_a[] = {T(TTOK_IDENT)};
        tile_val paramd_tile_a[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("\t\tint "),
            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S(" = 0;\n"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_PARAMD),
            .body = TS(paramd_rule_a),
            .tile = TILE(paramd_tile_a),
        };
        list_append(&rules, node);
        rule_cnt++;

        token paramd_rule_b[] = (token[]){T(TTOK_IDENT), T(TTOK_IDENT)};
        tile_val paramd_tile_b[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("\t\tint "),
            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 1,
            },
            {
                .type = TILE_LITERAL,
                .literal = S(" = 0;\n"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_PARAMD),
            .body = TS(paramd_rule_b),
            .tile = TILE(paramd_tile_b),
        };
        list_append(&rules, node);
        rule_cnt++;

        token paraml_rule_a[] = {T(NTOK_PARAMD)};
        tile_val paraml_tile_a[] = {
            {
                .type = TILE_ARG,
                .arg = 0,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_PARAML),
            .body = TS(paraml_rule_a),
            .tile = TILE(paraml_tile_a),
        };
        list_append(&rules, node);
        rule_cnt++;

        token paraml_rule_b[] = {T(NTOK_PARAML), T(TTOK_COMMA), T(NTOK_PARAMD)};
        tile_val paraml_tile_b[] = {
            {
                .type = TILE_ARG,
                .arg = 2,
            },
            {
                .type = TILE_ARG,
                .arg = 0,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_PARAML),
            .body = TS(paraml_rule_b),
            .tile = TILE(paraml_tile_b),
        };
        list_append(&rules, node);
        rule_cnt++;

        token tile_val_rule_a[] = {T(TTOK_IDENT)};
        tile_val tile_val_tile_a[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("[another]"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_TILE_VAL),
            .body = TS(tile_val_rule_a),
            .tile = TILE(tile_val_tile_a),
        };
        list_append(&rules, node);
        rule_cnt++;

        token tile_val_rule_b[] = {T(TTOK_ASCII)};
        tile_val tile_val_tile_b[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("[ascii]"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_TILE_VAL),
            .body = TS(tile_val_rule_b),
            .tile = TILE(tile_val_tile_b),
        };
        list_append(&rules, node);
        rule_cnt++;

        token tile_rule_a[] = {T(NTOK_TILE_VAL)};
        tile_val tile_tile_a[] = {
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_TILE),
            .body = TS(tile_rule_a),
            .tile = TILE(tile_tile_a),
        };
        list_append(&rules, node);
        rule_cnt++;

        token tile_rule_b[] = {T(NTOK_TILE), T(NTOK_TILE_VAL)};
        tile_val tile_tile_b[] = {
            {
                .type = TILE_ARG,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("\n"),
            },
            {
                .type = TILE_ARG,
                .arg = 1,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_TILE),
            .body = TS(tile_rule_b),
            .tile = TILE(tile_tile_b),
        };
        list_append(&rules, node);
        rule_cnt++;

        token tbind_rule[] = {T(TTOK_IDENT), T(TTOK_EQUAL), T(TTOK_REGEX)};
        tile_val tbind_tile[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("\n\tnode = make(&root, list, 1);\n"
                             "\tnode->data = make(&root, nfa, 1);\n"
                             "\tstring "),

            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("_term = S(\""),
            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 2,
            },
            {.type = TILE_LITERAL, .literal = S("\");\n\t")},
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("_term.len -= 2;\n\t"),
            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("_term.addr += 1;\n"
                             "\t*(nfa *)node->data = circes_regex(&root, "),
            },
            {
                .type = TILE_ARG_SDATA,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("_term);\n\tlist_append(&terminals, node);\n"
                             "\tterminal_cnt++;\n"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_BINDING),
            .body = TS(tbind_rule),
            .tile = TILE(tbind_tile),
        };
        list_append(&rules, node);
        rule_cnt++;

        token nbind_rule[] = {
            T(TTOK_IDENT), T(TTOK_OPNPR), T(NTOK_PARAML), T(TTOK_CLSPR),
            T(TTOK_OPNBR), T(NTOK_TILE),  T(TTOK_CLSBR),
        };
        tile_val nbind_tile[] = {
            {
                .type = TILE_LITERAL,
                .literal = S("\t{\n"),
            },
            {
                .type = TILE_ARG,
                .arg = 5,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("\t}\n"),
            },
            /*
        {
            .type = TILE_LITERAL,
            .literal = S(
                "\n\ttoken nbind_rule[] = {\n"
                "\t\tT(0), T(0), T(0), T(0), T(0), T(0),  T(0)"
                "};\n"
                "        tile_val nbind_tile[] = {\n"
                "            {\n"
                "                .type = 0,\n"
                "                .literal = S(\" nonterminal binding \"),\n"
                "            },\n"
                "        };\n"
                "        node = make(&root, list, 1);\n"
                "        node->data = make(&root, pda_rule, 1);\n"
                "        *(pda_rule *)node->data = (pda_rule){\n"
                "            .head = T(0),\n"
                "            .body = TS(0),\n"
                "            .tile = TILE(0),\n"
                "        };\n"
                "        list_append(&rules, node);\n"
                "        rule_cnt++;\n"),
        },
        */
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_BINDING),
            .body = TS(nbind_rule),
            .tile = TILE(nbind_tile),
        };
        list_append(&rules, node);
        rule_cnt++;

        token binding_list_rule_a[] = {T(NTOK_BINDING)};
        tile_val binding_list_tile_a[] = {
            {
                .type = TILE_ARG,
                .arg = 0,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_BINDING_LIST),
            .body = TS(binding_list_rule_a),
            .tile = TILE(binding_list_tile_a),
        };
        list_append(&rules, node);
        rule_cnt++;

        token binding_list_rule_b[] = {T(NTOK_BINDING_LIST), T(NTOK_BINDING)};
        tile_val binding_list_tile_b[] = {
            {
                .type = TILE_ARG,
                .arg = 1,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("\n"),
            },
            {
                .type = TILE_ARG,
                .arg = 0,
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_BINDING_LIST),
            .body = TS(binding_list_rule_b),
            .tile = TILE(binding_list_tile_b),
        };
        list_append(&rules, node);
        rule_cnt++;

        token augmented_rule[] = {T(NTOK_BINDING_LIST), T(TTOK_OVER)};
        tile_val augmented_tile[] = {
            {
                .type = TILE_LITERAL,
                .literal = S(
                    "#include <mf/string.h>\n"
                    "#include <mf/memory.h>\n"
                    "#include <circes.h>\n"
                    "#include <stdio.h>\n"
                    "#include <stdlib.h>\n"
                    "typedef struct program_input {\n"
                    "        string program;\n"
                    "} program_input;\n"
                    "\n"
                    "typedef struct program_output {\n"
                    "        int bytes_used;\n"
                    "        string program;\n"
                    "} program_output;\n"
                    "\n"
                    "static const int part_sz = kibibyte / 4;\n"
                    "static program_input read_input(arena *a, int argc, \n"
                    "char **argv)\n"
                    "{\n"
                    "        char c = 0;\n"
                    "        int parts_read = 0;\n"
                    "        int bytes_read = 0;\n"
                    "        char *buffer = make(a, char, part_sz);\n"
                    "\n"
                    "        // eventually, optionally read from other places\n"
                    "        while ((c = fgetc(stdin)) != EOF) {\n"
                    "                buffer[(parts_read * part_sz) + \n"
                    "bytes_read++] = c;\n"
                    "                if (bytes_read >= part_sz) {\n"
                    "                        steal(a, char, part_sz);\n"
                    "                        bytes_read = 0;\n"
                    "                        parts_read++;\n"
                    "                }\n"
                    "        }\n"
                    "        return (program_input){\n"
                    "            .program = (string){buffer, parts_read * \n"
                    "part_sz + bytes_read},\n"
                    "        };\n"
                    "}\n"
                    "\n"
                    "static int write_output(int argc, char **argv, \n"
                    "program_output output)\n"
                    "{\n"
                    "        char *path = \"program.c\";\n"
                    "        if (argc >= 2) {\n"
                    "                path = argv[1];\n"
                    "        }\n"
                    "        FILE *out = fopen(path, \"w\");\n"
                    "        fwrite(output.program.addr, sizeof(char), \n"
                    "output.program.len, out);\n"
                    "        fclose(out);\n"
                    "        return 0;\n"
                    "}\n"
                    "int main(int argc, char **argv)\n"
                    "{\n"
                    "\tint root_sz = gigabyte;\n"
                    "\tchar *mem = malloc(root_sz);\n"
                    "\tarena root = make_arena_ptr(mem, root_sz);\n"
                    "\tnfa_init_builtins(&root);\n"
                    "\tprogram_input input = read_input(&root, argc, argv);\n"
                    "\tint terminal_cnt = 0;\n"
                    "\tint rule_cnt = 0;\n"
                    "\tlist *terminals = NULL;\n"
                    "\tlist *rules = NULL;\n"
                    "\tlist *node;\n"),
            },
            {
                .type = TILE_ARG,
                .arg = 0,
            },
            {
                .type = TILE_LITERAL,
                .literal = S("\n}\n"),
            },
        };
        node = make(&root, list, 1);
        node->data = make(&root, pda_rule, 1);
        *(pda_rule *)node->data = (pda_rule){
            .head = T(NTOK_START),
            .body = TS(augmented_rule),
            .tile = TILE(augmented_tile),
        };
        list_append(&rules, node);
        rule_cnt++;

        /* tail, condense grammar, lex, parse, select, and write */
        grammar sollux;
        sollux.lexer.len = 0;
        sollux.lexer.addr = make(&root, nfa, terminal_cnt);
        node = terminals;
        while (node) {
                sollux.lexer.addr[sollux.lexer.len++] = *(nfa *)node->data;
                node = node->next;
        }

        sollux.rule_cnt = 0;
        sollux.rules = make(&root, pda_rule, rule_cnt);
        node = rules;
        while (node) {
                sollux.rules[sollux.rule_cnt++] = *(pda_rule *)node->data;
                node = node->next;
        }

        sollux.start = sollux.rules + (sollux.rule_cnt - 1);

        token_string tokens = circes_lex(&root, &sollux, input.program);
        parse_tree *tree = circes_parse(&root, &sollux, tokens);
        string program = circes_select(&root, &sollux, tree);

        program_output output;
        output.program = program;
        output.bytes_used = bytes_used(root);

        return write_output(argc, argv, output);
}
