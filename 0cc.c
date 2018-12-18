#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token type
enum {
	TK_NUM = 256,
	TK_EOF,
};

typedef struct {
	int ty;
	int val;
	char *input;
} Token;

//save tokenized result. Only supports up to 100 tokens
Token tokens[100];
int pos = 0;

enum {
	ND_NUM = 256,
};

typedef struct Node {
	int ty;			//operator or ND_NUM
	struct Node *lhs;	//left hand side
	struct Node *rhs;       //right hand side
	int val;		//iff ty == ND_NUM
} Node;

Node *expr();

void error(int i) {
	fprintf(stderr, "unexpected token: %s\n",
	    tokens[i].input);
	exit(1);
}


Node *new_node(int ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

Node *term(void) {
	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokens[pos++].val);
	if (tokens[pos].ty == '(') {
		pos++;
		Node *node = expr();
		if (tokens[pos].ty != ')')
			fprintf(stderr, "parenthis does not match: %s",
			    tokens[pos].input);
		pos++;
		return node;
	}
	fprintf(stderr, "Neither number nor parenthis: %s",
	    tokens[pos].input);
}

Node *mul() {
	Node *lhs = term();
	if (tokens[pos].ty == '*') {
		pos++;
		return new_node('*', lhs, mul());
	}
	if (tokens[pos].ty == '/') {
		pos++;
		return new_node('/', lhs, mul());
	}
	return lhs;
}

Node *expr() {
	Node *lhs = mul();
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs, expr());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs, expr());
	}
	return lhs;
}


void tokenize(char *p) {
	int i = 0;
	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
		    *p == '(' || *p == ')') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}
		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}
		fprintf(stderr, "Failed to tokenize: %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}


void gen(Node *node) {
	fprintf(stderr, "%d\n", node->ty);
	if (node->ty == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->ty) {
	case '+':
		printf("	add rax, rdi\n");
		break;
	case '-':
		printf("	sub rax, rdi\n");
		break;
	case '*':
		printf("	mul rdi\n");
		break;
	case '/':
		printf("	mov rdx, 0\n");
		printf("	div rdi\n");
	}
	printf("	push rax\n");
}



int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid number of args\n");
		return 1;
	}

	tokenize(argv[1]);
	Node* node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
}
