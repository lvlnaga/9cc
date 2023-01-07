#include "9cc.h"

static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// ノード作成関数for2項演算子
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = new_node(kind);
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// ノード作成関数for数値
// 左辺, 右辺は設定不要
Node *new_num(int val)
{
  Node *node = new_node(ND_NUM);
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// expr    = mul ("+" mul | "-" mul)*
// +,- : 左結合演算子
// expr    = equality
Node *expr()
{
  return equality();
}

// equality   = relational ("==" relational | "!=" relational)*
static Node *equality()
{
  Node *node = relational();
  for (;;)
  {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
  Node *node = add();
  for (;;)
  {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node); // "<" へ変換し 左右項をひっくり返している
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node); // "<=" へ変換し 左右項をひっくり返している
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add()
{
  Node *node = mul();
  for (;;)
  {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul     = unary ("*" unary | "/" unary)*
// *,/ : 左結合演算子
static Node *mul()
{
  Node *node = unary();
  for (;;)
  {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary  =  ("+" | "-")? primary
// TODO Referenceだと違うが、テキストに合わせる
static Node *unary()
{
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), primary());
  return primary();
}

// primary = num | "(" expr ")"
static Node *primary()
{
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  // そうでなければ数値のはず
  return new_num(expect_number());
}

