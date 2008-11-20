#include <vector>

using namespace std;

struct Token {
	char *value;
	vector<AstNode *> *ast_tree;
};

class HybFSTokenizer
{
	private:
		char * str;
		char * str_pos;
		int p;
	public:
		HybFSTokenizer (char * str);
		~HybFSTokenizer ();

		/* Extract the next token */
		char * GetNextToken (int * type);
		

};

