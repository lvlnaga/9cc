#include "9cc.h"

// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;

  #ifdef DEBUG
    fprintf(stderr, "add new token. kind=> %d, str=> %s, len=> %d,\n", kind ,str, len);
  #endif

  return tok;
}

// 文字列が特定のフラグメントで開始しているどうかを確認
// トークナイズするために、期待する文字列一致すれば true, 一致してなければfalseを返す関数を用意している
static bool startswith(char *p, char *q)
{
  return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列 `user_input` をトークナイズしてそれを返す
Token *tokenize()
{
  char *p = user_input;
  Token head; // dummyのhead要素を作る
  head.next = NULL;
  Token *cur = &head; // 現在のtokenへのポインタにdummyのheadを設定

  while (*p)
  {
    // 空白文字をスキップ
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // 2文字区切り
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">="))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // 1文字区切り
    if (strchr("+-*/()<>;=", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // 数字
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // Identifier
    // a-zをトークナイズ
    // 複数の文字からなる識別子をTK_IDENT型のトークンとして読み込む
    if ('a' <= *p && *p <= 'z')
    {
      char *begin = p; //変数の開始地点
      while (('a' <= *p && *p <= 'z') || isdigit(*p))
      // 一文字目がa-zのためここでisdigit()も確認することで`foo1`みたいな変数も扱えるようにする
      {
        p++;
      }
      int len = p - begin; 
      // char *str;
      // strncpy(str,begin,len);// 文字列を取得
      // TODO: これでよいのかなぞ。
      cur = new_token(TK_IDENT, cur, begin, len);


      continue;
    }
    
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);

  return head.next; // 先頭のトークンを返す
}
