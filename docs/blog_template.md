Cコンパイラ作成入門のメモ #x (Stepx, x)

[:contents]

# はじめに

こちらをやってみたときのメモを書いていく。

[https://www.sigbus.info/compilerbook:embed:cite]

今回はStep6, 7

# Commit
## Step6
[https://github.com/lvlnaga/9cc/commit/0dd18e522522b32fd049d74d5fdd023eddced843](https://github.com/lvlnaga/9cc/commit/0dd18e522522b32fd049d74d5fdd023eddced843)

## Step7
[https://github.com/lvlnaga/9cc/commit/e89c0bbd2f42a5a409211f38822f7d1b8bc6c45b](https://github.com/lvlnaga/9cc/commit/e89c0bbd2f42a5a409211f38822f7d1b8bc6c45b)

# 調べたこと・理解したこと

## 見出し
### イメージ図/コード
### メモ
### 参考

## startswith
### コード
```c
bool startswith(char *p, char *q)
{
  return memcmp(p, q, strlen(q)) == 0;
}
```
### 詳細
-  文字列が特定のフラグメントで開始しているどうかを確認
    - トークナイズするために、期待する文字列一致すれば true, 一致してなければfalseを返す関数を用意している
- Javaとかでは用意されている関数の模様。

### 参考
[JavaのstartsWith、endsWithメソッドの使い方を現役エンジニアが解説【初心者向け】 | TechAcademyマガジン](https://magazine.techacademy.jp/magazine/45863)

[【Python】startswith・endswith（指定文字で始まる？終わる？） | 鎖プログラム (pg-chain.com)](https://pg-chain.com/python-startswith-endswith)



# 思ったこと
- 変更は大きいが変更している内容はおおよそ理解できた。
- ただ、これを自分で考えて開発できるかというとそんな気はしない。
  - 引き出しが増えればこのパターンとかを引き出せる様になるのかな。

# タグ
C言語  
コンパイラ  
Makefile
