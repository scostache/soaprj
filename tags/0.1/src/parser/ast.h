

enum node_type
{
	TAG,
	TAGVALUE,
	LOGIC_AND,
	LOGIC_OR,
	LOGIC_NOT
};

class AstNode {
	private:
		char 		*tag_name, *tag_value;
		node_type	type;
		AstNode		*left, *right;
		
	public:
		AstNode();

		void PrintString ();
			
		/* add the right child for the AST node */
		void AddRight(AstNode* new_obj);

		/* add the left child for the AST node */
		void AddLeft(AstNode* new_obj);

		/* set Tag */
		void SetTag(char *c);

		/* set Value */
		void SetValue(char *c);

		/* set the type of the node */
		void SetType(node_type type);

		/* get the left child */
		AstNode * GetLeft();

		/* get the right child */
		AstNode * GetRight();
		
		/* get tag name */
		char * GetTag();

		/* get tag value */
		char * GetValue();

		/* get node type */
		node_type GetType();

		/* prints the node information */
		void ToString();


		// class destructor
		~AstNode();

};

/* ========== some cute functions ========== */

/* Test function that prints the AST for the query */
void PrintAST (AstNode * node);
