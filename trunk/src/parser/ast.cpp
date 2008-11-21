#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "ast.h"

AstNode::AstNode()
{
	type = TAG;
	left = NULL;
	right = NULL;
	tag_name = NULL;
	tag_value = NULL;
}

void AstNode::AddRight(AstNode * new_obj)
{
	right = new_obj;
}

void AstNode::AddLeft(AstNode * new_obj)
{
	left = new_obj;
}


void AstNode::PrintString()
{
	//printf("message: \n");
}

/* if the tag name for a tag or tag:value object */
void AstNode::SetTag(char *c)
{
	tag_name = (char *) malloc (strlen (c) * sizeof (char));
	strcpy (tag_name, c);
}

/* set the value vor an tag:value object */
void AstNode::SetValue(char *c)
{
	tag_value = (char *) malloc (strlen (c) * sizeof (char));
	strcpy (tag_value, c);
}

void AstNode::SetType(node_type type)
{
	this->type = type;
}

AstNode * AstNode::GetLeft()
{
	return this->left;
}

AstNode * AstNode::GetRight()
{
	return this->right;
}

char * AstNode::GetTag()
{
	return tag_name;
}

char * AstNode::GetValue()
{
	return tag_value;
}

node_type AstNode::GetType()
{
	return this->type;
}

void AstNode::ToString()
{
	if (this->type == TAG | this->type == TAGVALUE)
		printf ("AST terminal: %s", this->tag_name);
	if (this->type == TAGVALUE)
		printf (":%s\n", this->tag_value);
	if (this->type == TAG)
		printf ("\n");
	if (this->type == LOGIC_AND)
		printf ("AST node: AND \n");
	if (this->type == LOGIC_OR)
		printf ("AST node: OR \n");
	if (this->type == LOGIC_NOT)
		printf ("Ast node: NOT \n");
}


/* AstNode destructor */
AstNode::~AstNode()
{
	printf ("Destructor\n");
	if (tag_name != NULL) {
		free (tag_name);
		if (tag_value != NULL)
			free (tag_value);
	}

}

/* prints the AST */
void PrintAST (AstNode *node)
{
	AstNode *left, *right;
	node->ToString();
	left	= node->GetLeft();
	right	= node->GetRight();
	if (left != NULL)
		PrintAST (left);
	if (right != NULL)
		PrintAST (right);
		
}


/*int main(int argc, char ** argv)
{
	AstNode a, c;

	char b[20];
	strcpy(b, "Hello");
	a.PrintString(b);


	return 0;
}*/

