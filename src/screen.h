// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NATURALCAS_SCREEN_H
#define NATURALCAS_SCREEN_H
#include "editor.h"
namespace natural {
struct Surface:Painter {
  unsigned short *pixels;
  int left,top,right,bottom;
  Surface(unsigned short *p):pixels(p),left(0),top(0),right(384),bottom(216){}
  void clip(int x,int y,int w,int h);
  void rect(int x,int y,int w,int h,unsigned short color);
  void line(int x,int y,int xx,int yy,unsigned short color);
  void text(int x,int y,const char *s,int scale,unsigned short color);
  void label(int x,int y,int width,int height,const char *s,unsigned short color);
};
}
#endif
