#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];

  /* 最初のおまじない */
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  printf("main:\n");

  //最初の数字を取得してその次の文字にポインタがセットされる
  printf("  mov rax, %ld\n", strtol(p, &p, 10)); 
  
  while (*p) //引数がなくなるまで続ける
  {
    if (*p == '+') //+があったら、ポインタを次にすすめて、+の次の数字を足してあげる
    {
      p++;
      printf("  add rax, %ld\n", strtol(p, &p, 10)); 
      continue;
    }
    
    if (*p == '-') ////-があったら、ポインタを次にすすめて、-の次の数字を引いてあげる
    {
      p++;
      printf("  sub rax, %ld\n", strtol(p, &p, 10)); 
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    return 1;
  }
  
  
  printf("  ret\n");
  return 0;
}