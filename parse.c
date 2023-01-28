#include "9cc.h"

// 次のトークンが期待している記号のときには、トークンを１つ読み進めて
// Trueを返す。それ以外の場合はFalseを返す。
static bool consume(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||         // トークンと文字サイズが違う場合はfalse
      memcmp(token->str, op, token->len)) // 期待するトークンと不一致の場合はfalse
    return false;
  token = token->next;
  return true;
}

// 次のトークンがidentのときには、トークンを１つ読み進めて
// そのトークンを返す。それ以外の場合はNULLを返す。
static Token *consume_ident()
{
  if (token->kind != TK_IDENT)
    return NULL; // 期待するトークンと不一致(変数でなければ)の場合はNULL

  // 次のトークンがIdentのときはreturnするtokenを退避
  Token *ret = token;
  // tokenを次にconsumeする
  token = token->next;
  return ret;
}

// 次のトークンが期待している記号のときには、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
static void expect(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||         // トークンと文字サイズが違う場合はfalse
      memcmp(token->str, op, token->len)) // 期待するトークンと不一致の場合はfalse
    error_at(token->str, "\"%s\"ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
static int expect_number()
{
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// トークンの終端を示す
static bool at_eof()
{
  return token->kind == TK_EOF;
}

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

// 変数を名前で検索する。見つからなかった場合はNULLを返す
static LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
  {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
    // TODO: memcmpのところ何やっているか理解する
    // たぶん...memcmpはメモリ比較だから、トークンの文字とvarの文字が一致しているかをチェックしている
    {
      // すでにlocalsに登録されている場合はその変数を返す。
#ifdef DEBUG      
      fprintf(stderr, "lvar is found \n");
#endif      
      return var;
    }
  }
#ifdef DEBUG      
      fprintf(stderr, "lvar is not found \n");
#endif      

  return NULL;
}

/*
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
*/
void *program();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// program    = stmt*
void *program()
{
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL; // ここで末尾にNULLを入れているから, mainのコード生成のfor文の終了条件判定がcode[i]でOKなのか
}

// stmt       = expr ";"
static Node *stmt()
{
  Node *node = expr();
  expect(";");
  return node;
}

// expr    = assign
static Node *expr()
{
  return assign();
}

// assign     = equality ("=" assign)?
// ()ってどういう意味だっけ？ ( ... ) -> グループ化
// ?ってどういう意味だっけ？  A? -> Aまたはε
static Node *assign()
{
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
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
// NOTE Referenceだと違うが、テキストに合わせる
static Node *unary()
{
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), primary());
  return primary();
}

// 新：primary    = num | ident | "(" expr ")"
static Node *primary()
{
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }
  // 変数(ident)の場合
  Token *tok = consume_ident();
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) //既に出現していた変数の場合はその変数のoffsetを使う
    {
      node->offset = lvar->offset;
    }
    else //新たな変数の場合
    {
#ifdef DEBUG
      fprintf(stderr, "-------locals pre------------\n");
      fprintf(stderr, "locals: %p\n",locals);
      fprintf(stderr, "locals->next: %p\n",locals->next);
      fprintf(stderr, "locals->tok: %s\n",locals->name);
      fprintf(stderr, "locals->offset: %d\n",locals->offset);
#endif
      // 新しいLVarを作る
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset +8; //TODO:ここでsegumentationフォールと起こったことを書く
      node->offset = lvar-> offset;
      locals = lvar; //lvarのアドレスをlocalsへコピー

#ifdef DEBUG
      fprintf(stderr, "-------locals post------------\n");
      fprintf(stderr, "locals: %p\n",locals);
      fprintf(stderr, "locals->next: %p\n",locals->next);
      fprintf(stderr, "locals->tok: %s\n",locals->name);
      fprintf(stderr, "locals->offset: %d\n",locals->offset);
#endif
      // TODO: localsの連結リストを作っていく流れがよくわからない。next=NULLになっている感じがしない。
      // 最後に出てきた変数のnextはlocalsを指していそう。ただ、localsのnextはNULLじゃなさそう
    }

    return node;
  }

  // （）でもidentでもなければ数値のはず
  return new_num(expect_number());
}
