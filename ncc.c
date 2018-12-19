#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// トークンの型を表す数値
enum {
    TK_NUM = 256, // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
    int ty;          // トークンの型
    int val;         // tyがTK_NUMの場合、その数値
    char *input;     // トークン文字列(エラーメッセージ用)
} Token;

enum {
    ND_NUM = 256, // 整数のノードの型
};

typedef struct Node {
    int ty;             // 演算子かND_NUM
    struct Node *lhs;   // 左辺
    struct Node *rhs;   // 右辺
    int val;            // tyがND_NUMの場合のみ扱う 
} Node;

Token tokens[100];
int pos = 0;

Node *expr();
Node *mul();
Node *term();

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





Node *term() {
    if (tokens[pos].ty == TK_NUM) {
        return new_node_num(tokens[pos++].val);
    }
    if (tokens[pos].ty == '(') {
        pos++;
        Node *node = expr();
        if (tokens[pos].ty != ')') {
            fprintf(stderr, "開き括弧に対応する閉じ括弧がありません: %s\n", tokens[pos].input);
            exit(1);
        }
        pos++;
        return node;
    }
    fprintf(stderr, "数値でも開き括弧でもでもないトークンです: %s\n", tokens[pos].input);
    exit(1);
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

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty) {
        case '+':
            printf("    add rax, rdi\n");
            break;

        case '-':
            printf("    sub rax, rdi\n");
            break;

        case '*':
            printf("    mul rdi\n");
            break;

        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
    }

    printf("    push rax\n");
}

void error(int i) {
    fprintf(stderr, "予期せぬトークンです: %s\n", tokens[i].input);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    tokenize(argv[1]);

    // 抽象構文木を構築
    Node *node = expr();

    // アセンブリの前半部分
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコードを生成
    gen(node);

    // 式全体を評価した値がスタックトップにあるので、それをraxにpopして返り値とする
    printf("    pop rax\n");
    printf("    ret\n");

    return 0;
}