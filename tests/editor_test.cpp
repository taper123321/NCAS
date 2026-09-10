// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/editor.h"
#include "../src/screen.h"
#include "../src/catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace natural;
static int checks=0;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static char source[MAX_SOURCE];
static void expect(Editor &e,const char *s){CHECK(e.validate());CHECK(e.serialize(source,sizeof(source)));if(strcmp(source,s)){fprintf(stderr,"Expected %s\nActual   %s\n",s,source);exit(1);}}
static void image(const char *filename,unsigned short *pix){
  FILE *f=fopen(filename,"wb");CHECK(f);fprintf(f,"P6\n384 216\n255\n");
  for(int i=0;i<384*216;++i){unsigned c=pix[i];unsigned char rgb[3]={(unsigned char)(((c>>11)&31)*255/31),(unsigned char)(((c>>5)&63)*255/63),(unsigned char)((c&31)*255/31)};fwrite(rgb,1,3,f);}fclose(f);
}
int main(int argc,char **argv){
  Editor e;
  // Integer coefficients have the same pixels/width as juxtaposition, without changing CAS input.
  Editor juxtaposed;unsigned short shown[384*216],expectedPixels[384*216];Surface shownSurface(shown),expectedSurface(expectedPixels);
  const char *products[]={"2*x","-12*x^2","3*y+2*x"};const char *adjacent[]={"2x","-12x^2","3y+2x"};
  for(int i=0;i<3;++i){e.load(products[i]);juxtaposed.load(adjacent[i]);e.layout();juxtaposed.layout();CHECK(e.n[e.root].w==juxtaposed.n[juxtaposed.root].w);
    memset(shown,255,sizeof(shown));memset(expectedPixels,255,sizeof(expectedPixels));e.paint(shownSurface,10,70,false);juxtaposed.paint(expectedSurface,10,70,false);CHECK(!memcmp(shown,expectedPixels,sizeof(shown)));CHECK(e.serialize(source,sizeof(source)));CHECK(strchr(source,'*'));}
  const char *explicitCases[]={"2*3","2.5*x","x*x","2*xyz","1e-2*x","\"a+2*x\""};
  for(unsigned i=0;i<sizeof(explicitCases)/sizeof(*explicitCases);++i){e.clear();e.insert(explicitCases[i]);e.layout();for(int c=e.n[e.root].first;c;c=e.n[c].next)if(!strcmp(e.n[c].text,"*"))CHECK(e.n[c].w>0);}
  e.clear();e.insert("12*x");e.layout();e.before=e.n[e.root].first;int lastX=e.cursorX();for(int i=0;i<3;++i){e.move(RIGHT);CHECK(e.cursorX()>lastX);lastX=e.cursorX();}CHECK(!e.before);
  for(int i=0;i<3;++i){e.move(LEFT);CHECK(e.cursorX()<lastX);lastX=e.cursorX();}CHECK(e.before==e.n[e.root].first);
  e.move(RIGHT);e.move(RIGHT);CHECK(e.backspace());expect(e,"1*x");CHECK(e.backspace());expect(e,"x");
  e.clear();
  CHECK(e.fraction());CHECK(e.insert("1"));e.move(DOWN);CHECK(e.insert("2"));e.leave();CHECK(e.insert("+"));CHECK(e.fraction());CHECK(e.insert("1"));e.move(DOWN);CHECK(e.insert("6"));e.leave();expect(e,"((1)/(2))+((1)/(6))");
  e.clear();e.insert("2x");e.square();expect(e,"2*((x)^(2))");
  e.clear();e.insert("x+12");e.fraction();e.insert("5");e.leave();expect(e,"x+((12)/(5))");
  e.clear();e.insert("ans()");e.square();expect(e,"((ans())^(2))");
  e.clear();e.call("gcd",1);e.insert("12");e.nextArgument();e.insert("8");expect(e,"gcd(12,8)");
  e.clear();e.matrix(2,2);e.insert("1");e.fraction();e.insert("2");e.leave();e.clearField();CHECK(!e.n[e.row].first);CHECK(e.activeMatrix());CHECK(e.validate());
  e.clear();e.matrix(2,2);int m=e.activeMatrix();CHECK(m);e.insert("1");e.move(RIGHT);CHECK(e.row==e.child(m,1));e.insert("2");e.move(DOWN);CHECK(e.row==e.child(m,3));e.insert("4");e.move(LEFT);e.move(LEFT);CHECK(e.row==e.child(m,2));e.insert("3");expect(e,"[[1,2],[3,4]]");
  CHECK(e.resizeMatrix(1,0));CHECK(!e.serialize(source,sizeof(source)));CHECK(strstr(e.error,"Fill"));CHECK(e.row==e.child(m,4));e.insert("5");e.move(RIGHT);e.insert("6");expect(e,"[[1,2],[3,4],[5,6]]");
  CHECK(!e.resizeMatrix(-1,0));CHECK(strstr(e.error,"Clear"));CHECK(e.validate());
  e.backspace();e.move(LEFT);e.backspace();CHECK(e.resizeMatrix(-1,0));expect(e,"[[1,2],[3,4]]");
  e.clear();e.matrix(2,2);e.insert("1");e.fraction();e.insert("2");e.move(DOWN);CHECK(e.activeMatrix());CHECK(e.validate());
  e.clear();CHECK(e.radical());CHECK(!e.serialize(source,sizeof(source)));e.insert("2");e.leave();expect(e,"sqrt(2)");
  e.clear();e.insert("x");CHECK(e.call("diff",2,true));int fn=e.n[e.row].parent;CHECK(e.setArgument(fn,1,"x"));expect(e,"diff(x,x)");
  // Indexed radical, direct exponential, empty nested templates and horizontal escape.
  e.clear();CHECK(e.radical(true));int rt=e.n[e.row].parent;e.insert("8");e.move(UP);CHECK(e.row==e.child(rt,1));e.insert("3");expect(e,"surd(8,3)");e.move(RIGHT);CHECK(e.row==e.root);
  e.clear();CHECK(e.exponential());CHECK(e.n[e.n[e.row].parent].kind==POWER);e.insert("x");expect(e,"((e)^(x))");
  e.clear();e.fraction();e.radical();e.backspace();e.backspace();CHECK(!e.n[e.root].first);
  e.clear();e.fraction();e.move(DOWN);e.fraction();e.row=e.root;e.before=0;CHECK(e.backspace());CHECK(!e.n[e.root].first);
  const char *naturalCases[]={"matrix[[1,2],[3,4]]","[ [ 1, 2 ], [ 3, 4 ] ]","sum(k^2,k,1,n)","product(k,k,1,5)","limit(sin(x)/x,x=0)","diff(x^4,x,3)","exp(x+1)","abs(1+i)","conj(1+i)","ncas_polar(2,45)","surd(x,3)"};
  for(unsigned i=0;i<sizeof(naturalCases)/sizeof(*naturalCases);++i){CHECK(e.load(naturalCases[i]));CHECK(e.validate());if(i<2)CHECK(e.n[e.n[e.root].first].kind==MATRIX);e.layout();CHECK(e.serialize(source,sizeof(source)));unsigned char packed[4096];int size=e.pack(packed,sizeof(packed));CHECK(size>0);CHECK(e.unpack(packed,size));for(int step=0;step<700;++step)e.move(LEFT);CHECK(e.row==e.root&&e.before==e.n[e.root].first);for(int step=0;step<700;++step)e.move(RIGHT);CHECK(e.row==e.root&&!e.before);}
  const char *cases[]={"1/2+1/6","-x^2","x^-2","x^2^3","(x+1)/(x-1)","sqrt(2)","[[1,2],[3,4]]","det([[1,2],[3,4]])","sin(30)","2*x+3","[1,2,3]","x=2","ans()","normald_cdf(0,1,-1.96,1.96)"};
  FILE *roundtrips=argc>1?fopen("roundtrips.tsv","wb"):0;
  for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);++i){CHECK(e.load(cases[i]));CHECK(e.validate());CHECK(e.serialize(source,sizeof(source)));if(roundtrips)fprintf(roundtrips,"%s\t%s\n",cases[i],source);e.layout();CHECK(e.n[e.root].w>0);}
  if(roundtrips)fclose(roundtrips);
  e.clear();for(int i=0;i<100;++i){if(!e.radical())break;CHECK(e.validate());}CHECK(e.validate());CHECK(e.error[0]);
  e.clear();for(int i=0;i<MAX_NODES*2;++i)e.insert("7");CHECK(e.validate());CHECK(e.available()==0);CHECK(!e.fraction());
  e.clear();e.insert("123");char small[3];CHECK(!e.serialize(small,3));CHECK(!small[0]);
  // Deterministic stress of edits, nesting, grid resizing and traversal.
  unsigned state=81289;
  for(int trial=0;trial<30;++trial){e.clear();for(int step=0;step<700;++step){state=state*1664525+1013904223;switch((state>>16)%15){
    case 0:e.insert("x");break;case 1:e.insert("2");break;case 2:e.insert("+");break;case 3:e.fraction();break;case 4:e.power();break;case 5:e.radical();break;case 6:e.matrix(2,2);break;case 7:e.backspace();break;case 8:e.leave();break;case 9:e.move(LEFT);break;case 10:e.move(RIGHT);break;case 11:e.move(UP);break;case 12:e.move(DOWN);break;case 13:e.resizeMatrix(1,0);break;case 14:e.removeStructure();break;}
    if(!e.validate()){fprintf(stderr,"stress trial=%d step=%d action=%u row=%d kind=%d before=%d root=%d\n",trial,step,(state>>16)%15,e.row,e.n[e.row].kind,e.before,e.root);for(int z=1;z<MAX_NODES;++z)if(e.n[z].kind)fprintf(stderr,"node %d k%d p%d f%d l%d prev%d next%d r%d c%d\n",z,e.n[z].kind,e.n[z].parent,e.n[z].first,e.n[z].last,e.n[z].prev,e.n[z].next,e.n[z].rows,e.n[z].cols);}
    CHECK(e.validate());e.layout();CHECK(e.serialize(source,sizeof(source),false)||e.error[0]);}}
  CHECK(operationCount>=65);
  if(argc>1){FILE *f=fopen("operations.tsv","wb");CHECK(f);for(int i=0;i<operationCount;++i){const Operation &o=operations[i];fprintf(f,"%s\t%s\t%d",o.category,o.command,o.count);for(int k=0;k<o.count;++k)fprintf(f,"\t%s",o.defaults[k]?o.defaults[k]:"");fprintf(f,"\n");}fclose(f);}
  printf("PASS: %d checks; %d guided operations; Editor %zu bytes\n",checks,operationCount,sizeof(Editor));
}
