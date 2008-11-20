#define WORD                            1
#define LPAR                            2
#define RPAR                            3
#define COLON                           4
#define AND                             5
#define OR                              6
#define NOT                             7


/* main function that generates the "AST tree"
 * based on the query given;
 * Returns NULL if the query is bad formated
 * In case of SUCCESS returns the root
 * of the "AST tree"
 */
AstNode* vdir_parse_query (char *query);


