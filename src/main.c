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

/*
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

        NTOK_START,         // 10
        NTOK_PARAMD,        // 11
        NTOK_PARAML,        // 12
        NTOK_TILE_VAL,      // 13
        NTOK_TILE,          // 14
        NTOK_TBINDING,      // 15
        NTOK_TBINDING_LIST, // 16
        NTOK_NBINDING,      // 17
        NTOK_NBINDING_LIST, // 18
        NTOK_BINDING_LIST,  // 19
} token_type;
*/

int main(int argc, char **argv)
{
        /*
         * head, program initialization
         */

        int root_sz = gigabyte;
        char *mem = malloc(root_sz);
        arena root = make_arena_ptr(mem, root_sz);
        arena root_tmp = reserve(root, char, root_sz / 2);

        nfa_init_builtins(&root);
        program_input input = read_input(&root, argc, argv);

        /*
         * body, define grammar
         */

        int terminal_cnt = 0;
        int nonterminal_cnt = 0;
        int rule_cnt = 0;
        list *terminals = NULL;
        list *rules = NULL;
        list *node;

#define opnbr_dg (1)
        int opnbr = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("{"));
        list_append(&terminals, node);

#define clsbr_dg (1)
        int clsbr = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("}"));
        list_append(&terminals, node);

#define opnpr_dg (1)
        int opnpr = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("\\("));
        list_append(&terminals, node);

#define clspr_dg (1)
        int clspr = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("\\)"));
        list_append(&terminals, node);

#define comma_dg (1)
        int comma = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S(","));
        list_append(&terminals, node);

#define equal_dg (1)
        int equal = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("="));
        list_append(&terminals, node);

#define regex_dg (1)
        int regex = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("`.\\x*.`*"));
        list_append(&terminals, node);

#define ascii_dg (1)
        int ascii = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("'.\\y*.'*"));
        list_append(&terminals, node);

#define ident_dg (1)
        int ident = terminal_cnt++;
        node = make(&root, list, 1);
        node->data = make(&root, nfa, 1);
        *(nfa *)node->data = circes_regex(&root, root_tmp, S("(\\w+_)*"));
        list_append(&terminals, node);

#define over_dg (1)
        int over = terminal_cnt++;
#ifndef start_dg
#define start_dg
        int start = terminal_cnt + (nonterminal_cnt++);
#endif

        int til_cnt;
        int tok_cnt;
        int arg_pos;
        list *tok_list;
        list *til_list;
        tile tile_body;
        token_string rule_body;

        tok_cnt = 0;
        til_cnt = 0;
        arg_pos = 0;
        tok_list = NULL;
        til_list = NULL;
#ifndef paramd_dg
#define paramd_dg
        int paramd = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef ident_dg
#define ident_dg
        int ident = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef ident_pos_dg
#define ident_pos_dg
        int ident_pos = arg_pos++;
#endif
        node = make(&root, list, 1);
        node->data = make(&root, token, 1);
        *((token *)node->data) = T(ident);
        list_append(&tok_list, node);
        tok_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("\n#ifndef ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_dg\n#define ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_dg\n\tint ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal =
            S("_v = terminal_cnt + (nonterminal_cnt++);\n"
              "#endif\n");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("\n#ifndef ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_pos_dg\n#define ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_pos_dg\n\tint ");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = 0;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_pos;\n"
                                              "#endif\n\t");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = 0;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal = S("_pos = arg_pos++;\n");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal =
            S("        node = make(&root, list, 1);\n"
              "        node->data = make(&root, token, 1);\n"
              "        *((token *)node->data) = T(");
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_ARG_SDATA;
        ((tile_val *)node->data)->arg = ident_pos;
        list_append(&til_list, node);
        til_cnt++;

        node = make(&root, list, 1);
        node->data = make(&root, tile_val, 1);
        ((tile_val *)node->data)->type = TILE_LITERAL;
        ((tile_val *)node->data)->literal =
            S("_v);\n\t\tlist_append(&tok_list, node);\n"
              "        tok_cnt++;\n");
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
            .head = T(paramd),
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
#ifndef paramd_dg
#define paramd_dg
        int paramd = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef ident_dg
#define ident_dg
        int ident = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = (token[]){T(ident), T(ident)};

                node = make(&root, list, 1);
                node->data = make(&root, token, 1);
                *((token *)node->data) = T(ident);
                list_append(&tok_list, node);
                tok_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("\n#ifndef ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 0;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_dg\n#define ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 0;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_dg\n\tint ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 0;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal =
                    S("_v = terminal_cnt + (nonterminal_cnt++);\n"
                      "#endif\n");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("\n#ifndef ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 1;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_pos_dg\n#define ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 1;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_pos_dg\n\tint ");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 1;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_pos;\n"
                                                      "#endif\n\t");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = 1;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal = S("_pos = arg_pos++;\n");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal =
                    S("        node = make(&root, list, 1);\n"
                      "        node->data = make(&root, token, 1);\n"
                      "        *((token *)node->data) = T(");
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_ARG_SDATA;
                ((tile_val *)node->data)->arg = ident_pos;
                list_append(&til_list, node);
                til_cnt++;

                node = make(&root, list, 1);
                node->data = make(&root, tile_val, 1);
                ((tile_val *)node->data)->type = TILE_LITERAL;
                ((tile_val *)node->data)->literal =
                    S("_v);\n\t\tlist_append(&tok_list, node);\n"
                      "        tok_cnt++;\n");
                list_append(&til_list, node);
                til_cnt++;

                tile_body.addr = make(&root, tile_val, til_cnt);
                tile_body.len = 0;
                node = til_list;
                while (node) {
                        tile_body.addr[tile_body.len++] =
                            *((tile_val *)node->data);
                        node = node->next;
                }
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(paramd),
                    .body = TS(rule),
                    .tile = tile_body,
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef paraml_dg
#define paraml_dg
        int paraml = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef paramd_dg
#define paramd_dg
        int paramd = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(paramd)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(paraml),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef paraml_dg
#define paraml_dg
        int paraml = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef comma_dg
#define comma_dg
        int comma = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef paramd_dg
#define paramd_dg
        int paramd = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(paraml), T(comma), T(paramd)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 2,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(paraml),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef ident_dg
#define ident_dg
        int ident = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef til_val_dg
#define til_val_dg
        int til_val = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(ident)};
                tile_val _tile[] = {
                    {
                        .type = TILE_LITERAL,
                        .literal =
                            S("\n\tnode = make(&root, list, 1);\n"
                              "\tnode->data = make(&root, tile_val, 1);\n"
                              "\t((tile_val *)node->data)->type = TILE_ARG;\n"
                              "\t((tile_val *)node->data)->arg = "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {

                        .type = TILE_LITERAL,
                        .literal = S("_pos;\n"
                                     "\tlist_append(&til_list, node);\n"
                                     "\ttil_cnt++;\n"),
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(til_val),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef ascii_dg
#define ascii_dg
        int ascii = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef til_val_dg
#define til_val_dg
        int til_val = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(ascii)};
                tile_val _tile[] = {
                    {
                        .type = TILE_LITERAL,
                        .literal = S(
                            "\n\tnode = make(&root, list, 1);\n"
                            "\tnode->data = make(&root, tile_val, 1);\n"
                            "\t((tile_val *)node->data)->type = TILE_LITERAL;\n"
                            "\t((tile_val *)node->data)->literal = S(\""),
                    },
                    {
                        .type = TILE_ARG_QDATA,
                        .arg = 0,
                    },
                    {

                        .type = TILE_LITERAL,
                        .literal = S("\");\n"
                                     "\tlist_append(&til_list, node);\n"
                                     "\ttil_cnt++;\n"),
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(til_val),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef til_dg
#define til_dg
        int til = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef til_val_dg
#define til_val_dg
        int til_val = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(til_val)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(til),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef til_dg
#define til_dg
        int til = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef til_val_dg
#define til_val_dg
        int til_val = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(til), T(til_val)};
                tile_val _tile[] = {
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
                    .head = T(til),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef tbinding_dg
#define tbinding_dg
        int tbinding = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef ident_dg
#define ident_dg
        int ident = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef equal_dg
#define equal_dg
        int equal = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef regex_dg
#define regex_dg
        int regex = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token tbind_rule[] = {T(ident), T(equal), T(regex)};
                tile_val tbind_tile[] = {
                    {
                        .type = TILE_LITERAL,
                        .literal = S("\n#define "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("_dg (1)\n\tint "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("_v = terminal_cnt++;\n\tnode = "
                                     "make(&root, list, 1);\n"
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
                        .literal =
                            S("_term.addr += 1;\n"
                              "\t*(nfa *)node->data = circes_regex(&root, "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal =
                            S("_term);\n\tlist_append(&terminals, node);\n"),
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(tbinding),
                    .body = TS(tbind_rule),
                    .tile = TILE(tbind_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef nbinding_dg
#define nbinding_dg
        int nbinding = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef ident_dg
#define ident_dg
        int ident = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef opnpr_dg
#define opnpr_dg
        int opnpr = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef clspr_dg
#define clspr_dg
        int clspr = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef opnbr_dg
#define opnbr_dg
        int opnbr = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef clsbr_dg
#define clsbr_dg
        int clsbr = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef paraml_dg
#define paraml_dg
        int paraml = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef til_dg
#define til_dg
        int til = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {
                    T(ident), T(opnpr), T(paraml), T(clspr),
                    T(opnbr), T(til),   T(clsbr),
                };
                tile_val _tile[] = {
                    {
                        .type = TILE_LITERAL,
                        .literal = S("\n\ttok_cnt = 0;\n"
                                     "\ttil_cnt = 0;\n"
                                     "\targ_pos = 0;\n"
                                     "\ttok_list = NULL;\n"
                                     "\ttil_list = NULL;\n"
                                     "#ifndef "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("_dg\n#define "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("_dg\n\tint "),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal =
                            S("_v = terminal_cnt + (nonterminal_cnt++);\n"
                              "#endif\n"),
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 2,
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 5,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal =
                            S("        tile_body.addr = make(&root, tile_val, "
                              "til_cnt);\n"
                              "        tile_body.len = 0;\n"
                              "        node = til_list;\n"
                              "        while (node) {\n"
                              "                tile_body.addr[tile_body.len++] "
                              "= *((tile_val *)node->data);\n"
                              "                node = node->next;\n"
                              "        }\n"
                              "        rule_body.addr = make(&root, token, "
                              "tok_cnt);\n"
                              "        rule_body.len = 0;\n"
                              "        node = tok_list;\n"
                              "        while (node) {\n"
                              "                rule_body.addr[rule_body.len++] "
                              "= *((token *)node->data);\n"
                              "                node = node->next;\n"
                              "        }\n"
                              "        node = make(&root, list, 1);\n"
                              "        node->data = make(&root, pda_rule, 1);\n"
                              "        *(pda_rule *)node->data = (pda_rule){\n"
                              "                 .head = T("),
                    },
                    {
                        .type = TILE_ARG_SDATA,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("_v),\n"
                                     "                 .body = rule_body,\n"
                                     "                 .tile = tile_body,\n"
                                     "        };\n"
                                     "        list_append(&rules, node);\n"
                                     "        rule_cnt++;\n"),
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(nbinding),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef tbinding_list_dg
#define tbinding_list_dg
        int tbinding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef tbinding_dg
#define tbinding_dg
        int tbinding = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(tbinding)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(tbinding_list),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef tbinding_list_dg
#define tbinding_list_dg
        int tbinding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef tbinding_dg
#define tbinding_dg
        int tbinding = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(tbinding_list), T(tbinding)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 1,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(tbinding_list),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef nbinding_list_dg
#define nbinding_list_dg
        int nbinding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef nbinding_dg
#define nbinding_dg
        int nbinding = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(nbinding)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(nbinding_list),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef nbinding_list_dg
#define nbinding_list_dg
        int nbinding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef nbinding_dg
#define nbinding_dg
        int nbinding = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token nbinding_list_rule_b[] = {T(nbinding_list), T(nbinding)};
                tile_val nbinding_list_tile_b[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 1,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(nbinding_list),
                    .body = TS(nbinding_list_rule_b),
                    .tile = TILE(nbinding_list_tile_b),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef binding_list_dg
#define binding_list_dg
        int binding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef nbinding_list_dg
#define nbinding_list_dg
        int nbinding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef nbinding_dg
#define nbinding_dg
        int nbinding = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(tbinding_list), T(nbinding_list)};
                tile_val _tile[] = {
                    {
                        .type = TILE_ARG,
                        .arg = 0,
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("\n#define over_dg (1)\n"
                                     "\tint over_v = terminal_cnt++;\n\n"),
                    },
                    {
                        .type = TILE_ARG,
                        .arg = 1,
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(binding_list),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

#ifndef start_dg
#define start_dg
        int start = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef binding_list_dg
#define binding_list_dg
        int binding_list = terminal_cnt + (nonterminal_cnt++);
#endif
#ifndef over_dg
#define over_dg
        int over = terminal_cnt + (nonterminal_cnt++);
#endif
        {
                token rule[] = {T(binding_list), T(over)};
                tile_val _tile[] = {
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
                            "static program_input read_input(arena *a, int "
                            "argc, \n"
                            "char **argv)\n"
                            "{\n"
                            "        char c = 0;\n"
                            "        int parts_read = 0;\n"
                            "        int bytes_read = 0;\n"
                            "        char *buffer = make(a, char, part_sz);\n"
                            "\n"
                            "        // eventually, optionally read from other "
                            "places\n"
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
                            "            .program = (string){buffer, "
                            "parts_read * \n"
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
                            "        fwrite(output.program.addr, sizeof(char), "
                            "\n"
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
                            "\tprogram_input input = read_input(&root, argc, "
                            "argv);\n"
                            "\tint terminal_cnt = 0;\n"
                            "\tint nonterminal_cnt = 0;\n"
                            "\tint rule_cnt = 0;\n"
                            "\tint tok_cnt = 0;\n"
                            "\tint til_cnt = 0;\n"
                            "\tint arg_pos = 0;\n"
                            "\ttoken_string rule_body;\n"
                            "\ttile tile_body;\n"
                            "\tlist *tok_list = NULL;\n"
                            "\tlist *til_list = NULL;\n"
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
                        .literal = S(
                            "\tgrammar grmr;\n"
                            "\tgrmr.lexer.len = 0;\n"
                            "\tgrmr.lexer.addr = make(&root, nfa, "
                            "terminal_cnt);\n"
                            "\tnode = terminals;\n"
                            "\twhile (node) {\n"
                            "\tgrmr.lexer.addr[grmr.lexer.len++] = *(nfa "
                            "*)node->data;\n"
                            "\t\tnode = node->next;\n"
                            "\t}\n"
                            "\tgrmr.rule_cnt = 0;\n"
                            "\tgrmr.rules = make(&root, pda_rule, rule_cnt);\n"
                            "\tnode = rules;\n"
                            "\twhile (node) {\n"
                            "\t\tgrmr.rules[grmr.rule_cnt++] = *(pda_rule "
                            "*)node->data;\n"
                            "\t\tnode = node->next;\n"
                            "\t}\n"
                            "\tgrmr.start = grmr.rules + (grmr.rule_cnt - 1);\n"
                            "\ttoken_string tokens = circes_lex(&root, "
                            "&grmr, input.program);\n"
                            "\tparse_tree *tree = circes_parse(&root, &grmr, "
                            "tokens);\n"
                            "\tstring program = circes_select(&root, &grmr, "
                            "tree);\n"
                            "\tprogram_output output;\n"
                            "\toutput.program = program;\n"
                            "\toutput.bytes_used = bytes_used(root);\n"
                            "\treturn write_output(argc, argv, output);\n"),
                    },
                    {
                        .type = TILE_LITERAL,
                        .literal = S("\n}\n"),
                    },
                };
                node = make(&root, list, 1);
                node->data = make(&root, pda_rule, 1);
                *(pda_rule *)node->data = (pda_rule){
                    .head = T(start),
                    .body = TS(rule),
                    .tile = TILE(_tile),
                };
                list_append(&rules, node);
                rule_cnt++;
        }

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

        int special = bytes_used(root);

        token_string tokens =
            circes_lex(&root, root_tmp, &sollux, input.program);
        parse_tree *tree = circes_parse(&root, root_tmp, &sollux, tokens);
        string program = circes_select(&root, root_tmp, &sollux, tree);

        program_output output;
        output.program = program;
        output.bytes_used = bytes_used(root) - special;

        return write_output(argc, argv, output);
}
