#include <vector>
#include <cstdio>
#include <cstring>
#include "ast.h"
#include "token.h"
#include "parser.h"

using namespace std;

HybFSTokenizer::HybFSTokenizer (char *str)
{
	this->str = new char[strlen(str) + 1];
	strcpy (this->str, str);
	this->str_pos = this->str;
}

HybFSTokenizer::~HybFSTokenizer()
{
	delete this->str;
}

char * HybFSTokenizer::GetNextToken (int * type)
{
	char word[100];
	int is_word = 0;
	char* word_pos  = word;

	while (this->str_pos != (str + (sizeof(char) * strlen(str)))) {
		if (!is_word) {
			if (*str_pos == ' ')
				this->str_pos++;
			else if (*str_pos == '(') {
				this->str_pos++;
				*type = LPAR;
				return "(";
			}
			else if (*str_pos == ')') {
				this->str_pos++;
				*type = RPAR;
				return ")";
			}
			else if (*str_pos == '+') {
				this->str_pos++;
				*type = AND;
				return "+";
			}
			else if (*str_pos == '|') {
				this->str_pos++;
				*type = OR;
				return "|";
			}
			else if (*str_pos == '!') {
				this->str_pos++;
				*type = NOT;
				return "!";
			}
			else if (*str_pos == ':') {
				this->str_pos++;
				*type = COLON;
				return ":";
			}
			else {
				*word_pos = *str_pos;
				this->str_pos++;
				word_pos++;
				*type = WORD;
				is_word = 1;
			}
		}
		else {
			if ((*str_pos != ' ') && (*str_pos != '(')
					&& (*str_pos != ')')
					&& (*str_pos != '+')
					&& (*str_pos != '|')
					&& (*str_pos != ':')
					&& (*str_pos != '!')) {
				*word_pos = *str_pos;
				this->str_pos++;
				word_pos++;
			}
			else {
				*word_pos = '\0';
				return strdup(word);
			}
		}
	}// end while

	/* when the end of the string is reached */
	return NULL;
}

