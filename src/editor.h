// NaturalCAS - Copyright (C) 2026. SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NATURALCAS_EDITOR_H
#define NATURALCAS_EDITOR_H

namespace natural {
const int MAX_NODES=512, MAX_DEPTH=24, MAX_TEXT=32, MAX_SOURCE=2048;
enum Kind { FREE, ROW, TEXT, FRACTION, POWER, ROOT, MATRIX, CALL, GROUP };
enum Direction { LEFT, RIGHT, UP, DOWN, NEXT, PREVIOUS };
struct Node {
  short kind,parent,first,last,next,prev;
  short rows,cols;
  char text[MAX_TEXT];
  int x,y,w,up,down,scale;
};
struct Painter {
  virtual void rect(int x,int y,int w,int h,unsigned short color)=0;
  virtual void line(int x,int y,int xx,int yy,unsigned short color)=0;
  virtual void text(int x,int y,const char *s,int scale,unsigned short color)=0;
  virtual ~Painter() {}
};
struct Editor {
  Node n[MAX_NODES];
  int root,row,before;
  char error[96];
  Editor();
  void clear();
  bool insert(const char *text);
  bool fraction();
  bool power();
  bool square();
  bool radical(bool indexed=false);
  bool exponential(const char *base="e");
  bool empty(int id) const;
  bool group();
  bool matrix(int rows,int cols);
  bool resizeMatrix(int deltaRows,int deltaCols);
  bool call(const char *name,int arguments,bool wrapAll=false);
  bool setArgument(int call,int argument,const char *text);
  bool nextArgument();
  void move(Direction direction);
  bool backspace();
  bool removeStructure();
  void clearField();
  void swap(Editor &other);
  void leave();
  bool load(const char *source);
  bool serialize(char *out,int size,bool requireComplete=true);
  // Compact lossless edit form: preserves a typed division and manual brackets.
  int pack(unsigned char *out,int size) const;
  bool unpack(const unsigned char *data,int size);
  void layout(int scale=2);
  void paint(Painter &p,int x,int baseline,bool cursor=true);
  int cursorX() const;
  int cursorY() const;
  int activeMatrix() const;
  const char *hint();
  bool validate() const;
  // Public inspection helpers also used by the calculator adapter and tests.
  int child(int id,int index) const;
  int childCount(int id) const;
  int available() const;
  bool implicitProduct(int id) const;
  void seekCursor(int x,int y);
private:
  int allocate(Kind kind);
  void release(int id);
  void append(int parent,int id);
  void linkBefore(int parent,int id,int before);
  void unlink(int id);
  int makeRow(int parent);
  int depth(int id) const;
  int subtreeHeight(int id) const;
  int operandStart(int previous) const;
  bool structure(Kind kind,int count,const char *name,bool capture,bool all);
  bool writeNode(int id,char *out,int size,int &pos,bool complete,int level);
  bool write(char *out,int size,int &pos,const char *text);
  void measure(int id,int scale,int level);
  void position(int id,int x,int y);
  void draw(int id,Painter &p,int x,int y,bool cursor);
  int notation(int id) const;
  bool measureNotation(int id,int scale,int level);
  bool positionNotation(int id,int x,int y);
  bool drawNotation(int id,Painter &p,int x,int y);
  bool parseRow(int id,const char *s,int begin,int end,int level);
  bool parsePart(int id,const char *s,int begin,int end,int level);
  bool packNode(int id,unsigned char *out,int size,int &pos) const;
  int unpackNode(int parent,const unsigned char *data,int size,int &pos,int level);
};
const unsigned short INK=0x1949, MUTED=0x6bd1, ACCENT=0x0472,
  PAPER=0xffff, BACKGROUND=0xef9d, SELECT=0xd77c, RULE=0xce79, ERROR=0xb9e7;
}
#endif
