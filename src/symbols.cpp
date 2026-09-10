// SPDX-License-Identifier: GPL-3.0-or-later
#include "symbols.h"
#include "toolbar.h"
#include <string.h>
namespace natural {
MathSymbol toolbarSymbol(int action){
  if(action==FRACTION_KEY)return FRACTION_SYMBOL;
  if(action==POWER_KEY)return POWER_SYMBOL;
  if(action==ROOT_KEY)return ROOT_SYMBOL;

  if(action<OPERATION||action>=OPERATION+operationCount)return NO_SYMBOL;
  const Operation &op=operations[action-OPERATION];
  if(!strcmp(op.command,"diff"))return op.count==2?DERIVATIVE_SYMBOL:NTH_DERIVATIVE_SYMBOL;
  if(!strcmp(op.command,"integrate"))return op.count==2?INTEGRAL_SYMBOL:DEFINITE_INTEGRAL_SYMBOL;
  static const struct {const char *command;MathSymbol symbol;} map[]={
    {"limit",LIMIT_SYMBOL},{"sum",SUM_SYMBOL},{"product",PRODUCT_SYMBOL},
    {"det",DETERMINANT_SYMBOL},{"inv",INVERSE_SYMBOL},{"tran",TRANSPOSE_SYMBOL},{"idn",IDENTITY_SYMBOL},
    {"dot",DOT_SYMBOL},{"cross",CROSS_SYMBOL},{"l2norm",NORM_SYMBOL},
    {"re",REAL_SYMBOL},{"im",IMAG_SYMBOL},{"abs",MODULUS_SYMBOL},{"arg",ARGUMENT_SYMBOL},{"conj",CONJUGATE_SYMBOL},
    {"comb",CHOOSE_SYMBOL},{"mean",MEAN_SYMBOL},{"stddev",POP_SD_SYMBOL},{"stddevp",SAMPLE_SD_SYMBOL},{"plot",PLOT_SYMBOL}
  };
  for(unsigned i=0;i<sizeof(map)/sizeof(map[0]);++i)if(!strcmp(op.command,map[i].command))return map[i].symbol;
  return NO_SYMBOL;
}
static void integral(Surface &s,int x,int y,unsigned short c){
  s.line(x+7,y,x+4,y,c);s.line(x+4,y,x+2,y+3,c);s.line(x+2,y+3,x+2,y+14,c);s.line(x+2,y+14,x,y+17,c);s.line(x,y+17,x-3,y+17,c);
  s.line(x+8,y+1,x+5,y+1,c);s.line(x+3,y+3,x+3,y+14,c);s.line(x,y+16,x-3,y+16,c);
}
static void arrow(Surface &s,int x,int y,unsigned short c){s.line(x,y,x+9,y,c);s.line(x+7,y-2,x+9,y,c);s.line(x+7,y+2,x+9,y,c);}
void paintSymbol(Surface &s,MathSymbol k,int x,int y,unsigned short c){
  if(k==FRACTION_SYMBOL){s.rect(x+25,y,8,5,c);s.line(x+18,y+8,x+40,y+8,c);s.rect(x+25,y+12,8,5,c);return;}
  if(k==POWER_SYMBOL||k==INVERSE_SYMBOL||k==TRANSPOSE_SYMBOL){
    s.text(x+18,y+4,k==POWER_SYMBOL?"x":"A",2,c);s.text(x+31,y,k==POWER_SYMBOL?"n":k==INVERSE_SYMBOL?"-1":"T",1,c);return;}
  if(k==ROOT_SYMBOL){s.line(x+13,y+9,x+16,y+7,c);s.line(x+16,y+7,x+20,y+16,c);s.line(x+20,y+16,x+25,y+1,c);s.line(x+25,y+1,x+45,y+1,c);s.text(x+30,y+4,"x",2,c);return;}
  if(k==MATRIX_SYMBOL){
    s.line(x+16,y,x+16,y+17,c);s.line(x+16,y,x+20,y,c);s.line(x+16,y+17,x+20,y+17,c);
    s.line(x+41,y,x+41,y+17,c);s.line(x+37,y,x+41,y,c);s.line(x+37,y+17,x+41,y+17,c);
    for(int row=0;row<2;++row)for(int col=0;col<2;++col)s.rect(x+23+col*9,y+3+row*9,4,4,c);return;}
  if(k==DERIVATIVE_SYMBOL||k==NTH_DERIVATIVE_SYMBOL){
    if(k==DERIVATIVE_SYMBOL){s.text(x+26,y,"d",1,c);s.line(x+19,y+8,x+39,y+8,c);s.text(x+24,y+11,"dx",1,c);}
    else {s.label(x+23,y+2,7,6,"d",c);s.label(x+30,y,7,5,"n",c);s.line(x+17,y+9,x+41,y+9,c);
      s.label(x+19,y+12,14,6,"dx",c);s.label(x+33,y+10,7,5,"n",c);}return;}
  if(k==INTEGRAL_SYMBOL||k==DEFINITE_INTEGRAL_SYMBOL){
    integral(s,x+21,y,c);if(k==DEFINITE_INTEGRAL_SYMBOL){s.text(x+31,y,"b",1,c);s.text(x+12,y+11,"a",1,c);}else s.text(x+34,y+6,"dx",1,c);return;}
  if(k==LIMIT_SYMBOL){s.text(x+11,y+2,"lim",2,c);return;}
  if(k==SUM_SYMBOL){
    s.rect(x+19,y,22,2,c);s.line(x+19,y+1,x+29,y+8,c);s.line(x+20,y+1,x+30,y+8,c);s.line(x+29,y+8,x+19,y+16,c);s.line(x+30,y+8,x+20,y+16,c);s.rect(x+19,y+16,22,2,c);return;}
  if(k==PRODUCT_SYMBOL){s.rect(x+16,y,28,2,c);s.rect(x+21,y+2,2,16,c);s.rect(x+37,y+2,2,16,c);return;}
  if(k==DETERMINANT_SYMBOL||k==MODULUS_SYMBOL||k==NORM_SYMBOL){
    s.text(x+24,y+2,k==DETERMINANT_SYMBOL?"A":k==MODULUS_SYMBOL?"z":"v",2,c);
    s.line(x+19,y,x+19,y+17,c);s.line(x+39,y,x+39,y+17,c);
    if(k==NORM_SYMBOL){s.line(x+15,y,x+15,y+17,c);s.line(x+43,y,x+43,y+17,c);}return;}
  if(k==IDENTITY_SYMBOL){s.text(x+20,y,"I",2,c);s.text(x+34,y+11,"n",1,c);return;}
  if(k==DOT_SYMBOL||k==CROSS_SYMBOL){s.text(x+7,y+4,"a",2,c);s.text(x+40,y+4,"b",2,c);arrow(s,x+7,y+2,c);arrow(s,x+40,y+2,c);
    if(k==DOT_SYMBOL)s.rect(x+28,y+11,3,3,c);else s.text(x+24,y+4,"*",2,c);return;}
  if(k==REAL_SYMBOL||k==IMAG_SYMBOL||k==ARGUMENT_SYMBOL||k==PLOT_SYMBOL){s.label(x,y,58,18,k==REAL_SYMBOL?"Re(z)":k==IMAG_SYMBOL?"Im(z)":k==ARGUMENT_SYMBOL?"arg(z)":"y=f(x)",c);return;}
  if(k==CONJUGATE_SYMBOL||k==MEAN_SYMBOL){s.text(x+24,y+4,k==CONJUGATE_SYMBOL?"z":"x",2,c);s.line(x+23,y+1,x+34,y+1,c);return;}
  if(k==CHOOSE_SYMBOL){s.text(x+27,y,"n",1,c);s.text(x+27,y+11,"r",1,c);
    s.line(x+21,y,x+18,y+4,c);s.line(x+18,y+4,x+18,y+13,c);s.line(x+18,y+13,x+21,y+17,c);
    s.line(x+38,y,x+41,y+4,c);s.line(x+41,y+4,x+41,y+13,c);s.line(x+41,y+13,x+38,y+17,c);return;}
  if(k==POP_SD_SYMBOL){s.label(x+18,y+1,24,16,"\x1d",c);return;}
  if(k==SAMPLE_SD_SYMBOL)s.text(x+24,y+2,"s",2,c);
}
}
