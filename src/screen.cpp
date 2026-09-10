// SPDX-License-Identifier: GPL-3.0-or-later
#include "screen.h"
#include <string.h>
#include <ctype.h>
namespace natural {
// Original compact 5 x 7 bitmap alphabet. Each byte is a row, MSB left.
static const unsigned char letters[26][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
 {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}};
static const unsigned char lower[26][7]={
 {0,0,14,1,15,17,15},{16,16,30,17,17,17,30},{0,0,14,16,16,17,14},
 {1,1,15,17,17,17,15},{0,0,14,17,31,16,14},{6,9,8,28,8,8,8},
 {0,0,15,17,15,1,14},{16,16,30,17,17,17,17},{4,0,12,4,4,4,14},
 {2,0,6,2,2,18,12},{16,16,18,20,24,20,18},{12,4,4,4,4,4,14},
 {0,0,26,21,21,21,21},{0,0,30,17,17,17,17},{0,0,14,17,17,17,14},
 {0,0,30,17,30,16,16},{0,0,15,17,15,1,1},{0,0,22,25,16,16,16},
 {0,0,15,16,14,1,30},{8,8,28,8,8,9,6},{0,0,17,17,17,19,13},
 {0,0,17,17,17,10,4},{0,0,17,17,21,21,10},{0,0,17,10,4,10,17},
 {0,0,17,17,15,1,14},{0,0,31,2,4,8,31}};
static const unsigned char digits[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,2,12}};
static void glyph(char c,unsigned char *g){
  memset(g,0,7);if(c>='A'&&c<='Z'){memcpy(g,letters[c-'A'],7);return;}
  if(c>='a'&&c<='z'){memcpy(g,lower[c-'a'],7);return;}
  if(c>='0'&&c<='9'){memcpy(g,digits[c-'0'],7);return;}
  switch(c){
  case '+':g[1]=g[2]=g[4]=g[5]=4;g[3]=31;break;
  case '-':g[3]=31;break;case '=':g[2]=g[4]=31;break;
  case '*':g[1]=17;g[2]=10;g[3]=4;g[4]=10;g[5]=17;break;
  case '/':g[0]=g[1]=1;g[2]=2;g[3]=4;g[4]=8;g[5]=g[6]=16;break;
  case '\x1f':g[0]=g[6]=4;g[3]=31;break; // division sign
  case '\x1e':g[1]=31;g[2]=g[3]=g[4]=g[5]=10;g[6]=18;break; // pi
  case '\x1d':g[1]=15;g[2]=18;g[3]=g[4]=18;g[5]=12;break; // sigma
  case '\x1a':g[1]=4;g[2]=2;g[3]=31;g[4]=2;g[5]=4;break;
  case '\x1c':g[0]=14;g[1]=g[2]=g[4]=g[5]=17;g[3]=31;g[6]=14;break; // theta
  case '\x1b':g[1]=2;g[2]=4;g[3]=8;g[4]=16;g[5]=31;break; // angle
  case '.':g[5]=g[6]=6;break;case ',':g[5]=6;g[6]=4;break;
  case ':':g[1]=g[2]=g[4]=g[5]=4;break;case ';':g[1]=g[2]=g[4]=4;g[5]=4;g[6]=8;break;
  case '(':g[0]=2;g[1]=4;g[2]=g[3]=g[4]=8;g[5]=4;g[6]=2;break;
  case ')':g[0]=8;g[1]=4;g[2]=g[3]=g[4]=2;g[5]=4;g[6]=8;break;
  case '[':g[0]=g[6]=14;g[1]=g[2]=g[3]=g[4]=g[5]=8;break;
  case ']':g[0]=g[6]=14;g[1]=g[2]=g[3]=g[4]=g[5]=2;break;
  case '<':g[1]=2;g[2]=4;g[3]=8;g[4]=4;g[5]=2;break;
  case '>':g[1]=8;g[2]=4;g[3]=2;g[4]=4;g[5]=8;break;
  case '^':g[0]=4;g[1]=10;g[2]=17;break;
  case '_':g[6]=31;break;case '|':for(int i=0;i<7;++i)g[i]=4;break;
  case '!':g[0]=g[1]=g[2]=g[3]=g[5]=4;break;
  case '?':g[0]=14;g[1]=17;g[2]=2;g[3]=4;g[5]=4;break;
  case '\'':g[0]=g[1]=4;break;case '"':g[0]=g[1]=10;break;
  case '%':g[0]=25;g[1]=26;g[2]=2;g[3]=4;g[4]=8;g[5]=11;g[6]=19;break;
  case '&':g[0]=12;g[1]=18;g[2]=20;g[3]=8;g[4]=21;g[5]=18;g[6]=13;break;
  case ' ':break;default:g[0]=g[6]=31;g[1]=g[5]=17;g[3]=4;
  }
}
void Surface::clip(int x,int y,int w,int h){left=x<0?0:x;top=y<0?0:y;right=x+w>384?384:x+w;bottom=y+h>216?216:y+h;}
void Surface::rect(int x,int y,int w,int h,unsigned short c){
  int xx=x+w,yy=y+h;if(x<left)x=left;if(y<top)y=top;if(xx>right)xx=right;if(yy>bottom)yy=bottom;
  for(int r=y;r<yy;++r)for(int col=x;col<xx;++col)pixels[r*384+col]=c;
}
void Surface::line(int x,int y,int xx,int yy,unsigned short color){
  int dx=xx>x?xx-x:x-xx,dy=yy>y?yy-y:y-yy,sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx-dy;
  for(;;){rect(x,y,1,1,color);if(x==xx&&y==yy)break;int e=2*err;if(e>-dy){err-=dy;x+=sx;}if(e<dx){err+=dx;y+=sy;}}
}
void Surface::text(int x,int y,const char *str,int scale,unsigned short color){
  for(int i=0;str[i];++i){unsigned char g[7];glyph(str[i],g);for(int r=0;r<7;++r)for(int c=0;c<5;++c)if(g[r]&(1<<(4-c)))rect(x+i*6*scale+c*scale,y+r*scale,scale,scale,color);}
}
void Surface::label(int x,int y,int width,int height,const char *str,unsigned short color){
  int n=strlen(str);if(!n)return;
  int advance=width/n;if(advance>12)advance=12;
  int gw=advance-2;if(gw<5)gw=5;
  x+=(width-(n*advance-2))/2;
  for(int i=0;i<n;++i){unsigned char g[7];glyph(str[i],g);
    for(int r=0;r<7;++r)for(int c=0;c<5;++c)if(g[r]&(1<<(4-c)))
      rect(x+i*advance+c*gw/5,y+r*height/7,(c+1)*gw/5-c*gw/5,(r+1)*height/7-r*height/7,color);
  }
}
}
