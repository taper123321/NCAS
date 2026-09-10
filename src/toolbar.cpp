// SPDX-License-Identifier: GPL-3.0-or-later
#include "toolbar.h"
#include "symbols.h"
#include <string.h>
#include <stdio.h>
namespace natural {
static const char *categories[]={"Algebra","Solve","Calculus","Matrices","Vectors","Complex","Probability","Statistics","Number","Graphs","Further maths"};
// Human labels sized for the physical 64-pixel soft-key cells, independent of CAS syntax.
static const char *shortOps[]={"SIMPL","EXPAND","FACTOR","PFRAC","COLLECT","SUBST","PREM","PQUO","COEFF","SOLVE","NSOLVE","CSOLVE","SYSTEM","ROOTS","DIFF","N-DIFF","INT","DEFINT","LIMIT","TAYLOR","SUM","PROD","DE","RECUR","DET","INV","TRANS","RREF","RANK","EIGVAL","EIGVEC","IDENT","CHARP","LU","DOT","CROSS","NORM","RE","IM","ABS","ARG","CONJ","CFACT","BINPD","BINCD","NORMCD","NORMINT","INVNORM","POISPD","POISCD","NCR","MEAN","MEDIAN","POP SD","SAMP SD","LINREG","GCD","LCM","IFACT","PRIME?","MOD","Y=","PARAM","POLAR","IMPL","3D","STAT PT","UNITY","SINH","COSH","TANH"};
struct Item {const char *label;int action;};
Toolbar::Toolbar():menu(HOME),page(0),depth(0){trail[0]=0;}
void Toolbar::home(){menu=HOME;page=depth=0;}
void Toolbar::open(int m){if(depth<8){ancestors[depth]=menu;pages[depth++]=page;}menu=m;page=0;}
void Toolbar::back(){if(depth){--depth;menu=ancestors[depth];page=pages[depth];}else home();}
void Toolbar::build(const Status &st){
  Item items[24];int count=0;const char *title="";
  #define ITEM(labelValue,actionValue) do{items[count].label=labelValue;items[count++].action=actionValue;}while(0)
  for(int i=0;i<6;++i){labels[i][0]=0;actions[i]=NOTHING;}
  if(menu==HOME){page%=2;const char *names[]={"JUMP","EDIT","MAT","MATH","CALC","SETUP","GRAPH","CAT","APPS","VIEW"};int a[]={JUMP,EDIT,MAT,MATH,CALC,SETTINGS,CATEGORY+9,CATALOGUE,WORKSPACE,FULL_RESULT};for(int i=0;i<5;++i){strcpy(labels[i],names[page*5+i]);actions[i]=a[page*5+i];}strcpy(labels[5],">");actions[5]=PAGE;trail[0]=0;return;}
  if(menu==JUMP){title="Jump";ITEM("FIRST",FIRST);ITEM("LAST",LAST);ITEM("NEW",NEW_LINE);ITEM("PAN",PAN);}
  if(menu==EDIT){title="Edit";ITEM("UNDO",UNDO);ITEM("DELETE",DELETE_LINE);ITEM("FIELD",CLEAR_FIELD);ITEM("CLEAR",CONFIRM_CLEAR);ITEM("USE ANS",USE_ANSWER);ITEM("STORE",STORE);}
  if(menu==MAT){title="Matrix";ITEM("2*2",INSERT_MATRIX);ITEM("3*3",MATRIX_33);ITEM("3*1",MATRIX_31);ITEM("2*1",MATRIX_21);ITEM("m*n",MATRIX_CUSTOM);ITEM("SIZE",RESIZE);ITEM("OPS",CATEGORY+3);ITEM("VECT",CATEGORY+4);ITEM("LOAD",MATRIX_LOAD);}
  if(menu==MATH){title="Math";ITEM("FRAC",FRACTION_KEY);ITEM("POWER",POWER_KEY);ITEM("ROOT",ROOT_KEY);ITEM("SYMBOL",SYMBOLS);ITEM("ALG",CATEGORY);ITEM("SOLVE",CATEGORY+1);ITEM("NUMBER",CATEGORY+8);ITEM("CMPLX",CATEGORY+5);}
  if(menu==CALC){title="Calculate";ITEM("CALC",CATEGORY+2);ITEM("PROB",CATEGORY+6);ITEM("STAT",CATEGORY+7);ITEM("FURTH",CATEGORY+10);ITEM("S-D",DECIMAL_TOGGLE);}
  if(menu==SYMBOLS){title="Math > Symbols";ITEM("\x1e",PI_KEY);ITEM("i",I_KEY);ITEM("ANS",ANS_KEY);ITEM("=",EQUAL_KEY);ITEM("LIST",LIST_KEY);}
  if(menu==RESIZE){title="Matrix > Resize";ITEM("+ROW",ROW_ADD);ITEM("-ROW",ROW_DEL);ITEM("+COL",COL_ADD);ITEM("-COL",COL_DEL);}
  if(menu==CONFIRM_CLEAR){title="Clear sheet?";ITEM("KEEP",BACK);ITEM("CLEAR",CLEAR_SHEET);}
  if(menu==CONFIRM_DELETE){title="Delete line?";ITEM("KEEP",BACK);ITEM("DELETE",DELETE_LINE);}
  if(menu>=CATEGORY&&menu<CATEGORY+11){title=categories[menu-CATEGORY];for(int i=0;i<operationCount;++i)if(!strcmp(operations[i].category,title))ITEM(shortOps[i],OPERATION+i);}
  int pagesCount=(count+3)/4;if(pagesCount<1)pagesCount=1;page%=pagesCount;
  strcpy(labels[0],"<");actions[0]=BACK;
  for(int i=0;i<4&&page*4+i<count;++i){strncpy(labels[i+1],items[page*4+i].label,39);labels[i+1][39]=0;actions[i+1]=items[page*4+i].action;}
  if(pagesCount>1){strcpy(labels[5],">");actions[5]=PAGE;}
  strncpy(trail,title,sizeof(trail)-1);trail[sizeof(trail)-1]=0;
  #undef ITEM
}
void Toolbar::paint(Surface &s,const char *){
  const int y=192;s.clip(0,0,384,216);s.rect(0,y,384,24,PAPER);
  for(int i=0;i<6;++i){int x=i*64+1;if(actions[i]==NOTHING)continue;
    bool nav=actions[i]==PAGE||actions[i]==BACK;
    s.rect(x,y+1,62,22,INK);
    if(nav){s.rect(x+1,y+2,60,20,PAPER);int mid=x+31;
      if(actions[i]==PAGE){s.line(mid-5,y+5,mid+6,y+12,INK);s.line(mid+6,y+12,mid-5,y+19,INK);s.line(mid-5,y+5,mid-5,y+19,INK);}
      else {s.line(mid+5,y+5,mid-6,y+12,INK);s.line(mid-6,y+12,mid+5,y+19,INK);s.line(mid+5,y+5,mid+5,y+19,INK);}
    }else if(toolbarSymbol(actions[i])!=NO_SYMBOL)paintSymbol(s,toolbarSymbol(actions[i]),x+2,y+3,PAPER);
    else s.label(x+2,y+3,58,18,labels[i],PAPER);
  }
}
}
