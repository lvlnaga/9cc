#include "9cc.h"

// 式を左辺値として評価する
// 1. ベースポインタの値をraxへ格納
// 2. ベースポインタからoffset分引き算することで変数に対応するアドレスを計算
// 3. 変数に対応するアドレスの位置をスタックに積む
static void gen_lval(Node *node)
{
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node)
{
  // 左辺の評価
  switch (node->kind)
  {
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n"); // スタックに積まれているローカル変数のアドレスをpop
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR: // ローカル変数の場合
    // 概要：変数に格納してある値を取得してきてスタックに積む
    // 中身：
    // -変数のアドレスを計算@gen_lval
    // -当該アドレスから変数の値をロード
    // -ロードした値をスタックに積む
    gen_lval(node);
    // nodeの値
    printf("  pop rax\n");        // スタックに積まれているローカル変数のアドレスをpop
    printf("  mov rax, [rax]\n"); // ローカル変数の値をraxへロード
    printf("  push rax\n");       // ロードしてきた値をスタックに積む
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");        // スタックに積まれている右辺の値をrdiへロード
    printf("  pop rax\n");        // スタックに積まれている左辺の変数のアドレスをraxへロード
    printf("  mov [rax], rdi\n"); // 変数のアドレス(左辺)へrdiの値(右辺)をストア
    printf("  push rdi\n");       // 変数に格納された値をスタックへストア
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

void codegen(Node *node)
{
  // アセンブリ前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
}