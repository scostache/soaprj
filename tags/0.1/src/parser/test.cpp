#include <cstring>
#include <cstdio>
//#include "token.h"
#include "ast.h"
#include "parser.h"

int main (int argc, char ** argv)
{
	char str[]="(word2 + word3 | (word4 + !word5))";
	AstNode *a;

	a = vdir_parse_query(str);
/*
	char str[]="(word1 + !word2)";
	int i;
	char *response;
	//i = (int*) malloc (sizeof(int));
	HybFSTokenizer *ht = new HybFSTokenizer(str);
	while( (response = ht->GetNextToken(&i)) != NULL) {
		printf ("response: %s\n", response);
	} */
	//printf ("string: %s\n", ht->GetNextToken(&i));
	return 0;
}
