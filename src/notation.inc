// SPDX-License-Identifier: GPL-3.0-or-later
// Included in editor.cpp: display-only notation keeps the engine command unchanged.
enum Notation { PLAIN, SIGMA, PRODUCT, LIMIT_FORM, EXP_FORM, ABS_FORM, NORM_FORM,
 CONJ_FORM, TRANS_FORM, INV_FORM, POLAR_FORM, INDEX_ROOT, NTH_DIFF, DOT_FORM, CROSS_FORM, CHOOSE_FORM, IDENTITY_FORM };
int Editor::notation(int id) const {
 const Node &a=n[id];int count=childCount(id);
 if(a.kind==ROOT&&count==2)return INDEX_ROOT;
 if(a.kind!=CALL)return PLAIN;
 if(count==4&&!strcmp(a.text,"sum"))return SIGMA;
 if(count==4&&!strcmp(a.text,"product"))return PRODUCT;
 if(count==2&&!strcmp(a.text,"limit"))return LIMIT_FORM;
 if(count==3&&!strcmp(a.text,"diff"))return NTH_DIFF;
 if(count==2&&!strcmp(a.text,"ncas_polar"))return POLAR_FORM;
 if(count==2&&!strcmp(a.text,"dot"))return DOT_FORM;
 if(count==2&&!strcmp(a.text,"cross"))return CROSS_FORM;
 if(count==2&&!strcmp(a.text,"comb"))return CHOOSE_FORM;
 if(count!=1)return PLAIN;
 if(!strcmp(a.text,"idn"))return IDENTITY_FORM;
 if(!strcmp(a.text,"exp"))return EXP_FORM;
 if(!strcmp(a.text,"abs")||!strcmp(a.text,"det"))return ABS_FORM;
 if(!strcmp(a.text,"l2norm"))return NORM_FORM;
 if(!strcmp(a.text,"conj"))return CONJ_FORM;
 if(!strcmp(a.text,"tran"))return TRANS_FORM;
 if(!strcmp(a.text,"inv"))return INV_FORM;
 return PLAIN;
}
bool Editor::measureNotation(int id,int scale,int level){
 Node &a=n[id];int f=notation(id);if(!f)return false;
 for(int c=a.first;c;c=n[c].next)measure(c,((f==SIGMA||f==PRODUCT||f==INDEX_ROOT)&&c!=a.first)||(f==LIMIT_FORM&&c==a.last)||f==EXP_FORM||f==IDENTITY_FORM||(f==NTH_DIFF&&c==a.last)?1:scale,level+1);
 Node &b=n[a.first];a.up=b.up;a.down=b.down;a.w=b.w;
 if(f==SIGMA||f==PRODUCT){int l=child(id,1),lo=child(id,2),hi=a.last;int col=maximum(30,maximum(n[l].w+8+n[lo].w,n[hi].w));
   a.w=col+12+b.w;a.up=maximum(b.up,18+n[hi].up+n[hi].down);a.down=maximum(b.down,14+maximum(n[l].up+n[l].down,n[lo].up+n[lo].down));}
 else if(f==LIMIT_FORM){a.w=maximum(20*scale,n[a.last].w)+10+b.w;a.down=maximum(b.down,8+n[a.last].up+n[a.last].down);}
 else if(f==CHOOSE_FORM){a.w=maximum(b.w,n[a.last].w)+18;a.up=b.up+b.down+3;a.down=n[a.last].up+n[a.last].down+3;}
 else if(f==IDENTITY_FORM){a.w=6*scale+b.w+2;a.up=7*scale;a.down=b.up+b.down;}
 else if(f==EXP_FORM){a.w=6*scale+2+b.w;a.up=7*scale+b.up+b.down-3;a.down=2*scale;}
 else if(f==INDEX_ROOT){a.w=maximum(12,n[a.last].w+2)+18+b.w;a.up=maximum(b.up+5,b.up+2+n[a.last].up+n[a.last].down);}
 else if(f==NTH_DIFF){Node &v=n[child(id,1)],&o=n[a.last];int col=maximum(30,6*scale+v.w+o.w+8);a.w=col+18+b.w;a.up=maximum(b.up,7*scale+o.up+o.down+7);a.down=maximum(b.down,7*scale+9);}
 else if(f==POLAR_FORM||f==DOT_FORM||f==CROSS_FORM){a.w+=n[a.last].w+12*scale;a.up=maximum(a.up,n[a.last].up);a.down=maximum(a.down,n[a.last].down);}
 else if(f==ABS_FORM||f==NORM_FORM){a.w+=f==NORM_FORM?20:12;a.up+=2;a.down+=2;}
 else if(f==CONJ_FORM){a.w+=4;a.up+=4;}
 else {a.w+=f==INV_FORM?18:12;a.up+=8;}
 return true;
}
bool Editor::positionNotation(int id,int x,int y){
 Node &a=n[id];int f=notation(id);if(!f)return false;Node &b=n[a.first];int s=a.scale;
 if(f==SIGMA||f==PRODUCT){int l=child(id,1),lo=child(id,2),hi=a.last,col=a.w-12-b.w;
   position(a.first,x+col+12,y);position(hi,x+(col-n[hi].w)/2,y-18-n[hi].down);
   int lower=n[l].w+8+n[lo].w;position(l,x+(col-lower)/2,y+14+n[l].up);position(lo,n[l].x+n[l].w+8,y+14+n[lo].up);}
 else if(f==LIMIT_FORM){position(a.first,x+a.w-b.w,y);position(a.last,x,y+8+n[a.last].up);}
 else if(f==CHOOSE_FORM){position(a.first,x+(a.w-b.w)/2,y-3-b.down);position(a.last,x+(a.w-n[a.last].w)/2,y+3+n[a.last].up);}
 else if(f==IDENTITY_FORM)position(a.first,x+6*s+2,y+b.up);
 else if(f==EXP_FORM)position(a.first,x+6*s+2,y-7*s-b.down+3);
 else if(f==INDEX_ROOT){position(a.first,x+a.w-b.w,y);position(a.last,x,y-b.up-2-n[a.last].down);}
 else if(f==NTH_DIFF){int col=a.w-b.w-18;position(a.first,x+col+10,y);position(child(id,1),x+6*s+4,y+7+7*s);position(a.last,x+col/2+3*s,y-9-7*s);}
 else if(f==POLAR_FORM||f==DOT_FORM||f==CROSS_FORM){position(a.first,x,y);position(a.last,x+b.w+12*s,y);}
 else position(a.first,x+(f==NORM_FORM?10:f==ABS_FORM?6:f==CONJ_FORM?2:0),y);
 return true;
}
bool Editor::drawNotation(int id,Painter &p,int x,int y){
 Node &a=n[id];int f=notation(id);if(!f)return false;Node &b=n[a.first];int s=a.scale;
 if(f==SIGMA||f==PRODUCT){int col=a.w-12-b.w,xx=x+col/2-11,t=y-15,bot=y+11;
   p.line(xx,t,xx+22,t,INK);p.line(xx+1,t+1,xx+22,t+1,INK);
   if(f==SIGMA){p.line(xx,t,xx+12,y-2,INK);p.line(xx+12,y-2,xx,bot,INK);p.line(xx,bot,xx+22,bot,INK);p.line(xx+1,bot-1,xx+22,bot-1,INK);}
   else {p.rect(xx+3,t,2,27,INK);p.rect(xx+18,t,2,27,INK);}
   Node &v=n[child(id,1)];p.text(x+(v.x-a.x)+v.w+1,y+14,"=",1,INK);
 }
 else if(f==LIMIT_FORM)p.text(x,y-7*s,"lim",s,INK);
 else if(f==CHOOSE_FORM){int t=y-a.up,bot=y+a.down;for(int side=0;side<2;++side){int xx=x+(side?a.w-2:1),dir=side?-1:1;p.line(xx+3*dir,t,xx,t+4,INK);p.line(xx,t+4,xx,bot-4,INK);p.line(xx,bot-4,xx+3*dir,bot,INK);}}
 else if(f==IDENTITY_FORM)p.text(x,y-7*s,"I",s,INK);
 else if(f==EXP_FORM)p.text(x,y-7*s,"e",s,INK);
 else if(f==INDEX_ROOT){int xx=x+maximum(12,n[a.last].w+2);p.line(xx,y-2,xx+4,y-4,INK);p.line(xx+4,y-4,xx+8,y+3,INK);p.line(xx+8,y+3,xx+14,y-b.up-4,INK);p.line(xx+14,y-b.up-4,x+a.w,y-b.up-4,INK);}
 else if(f==NTH_DIFF){int col=a.w-b.w-18;p.text(x+col/2-3*s,y-6-7*s,"d",s,INK);p.line(x,y,x+col-2,y,INK);p.text(x+3,y+7,"d",s,INK);
   Node &v=n[child(id,1)],&o=n[a.last];draw(a.last,p,x+3+6*s+v.w-o.x,y+9-o.y,false);
   p.text(x+col+1,y-7*s,"(",s,INK);p.text(x+a.w-7,y-7*s,")",s,INK);}
 else if(f==POLAR_FORM)p.text(x+b.w+3*s,y-7*s,"\x1b",s,INK);
 else if(f==DOT_FORM)p.rect(x+b.w+5*s,y-4*s,2*s,2*s,INK);
 else if(f==CROSS_FORM)p.text(x+b.w+3*s,y-7*s,"*",s,INK);
 else if(f==ABS_FORM||f==NORM_FORM){p.line(x+1,y-a.up,x+1,y+a.down,INK);p.line(x+a.w-2,y-a.up,x+a.w-2,y+a.down,INK);
   if(f==NORM_FORM){p.line(x+5,y-a.up,x+5,y+a.down,INK);p.line(x+a.w-6,y-a.up,x+a.w-6,y+a.down,INK);}}
 else if(f==CONJ_FORM)p.line(x+1,y-a.up+1,x+a.w-1,y-a.up+1,INK);
 else p.text(x+b.w+2,y-a.up,f==INV_FORM?"-1":"T",1,INK);
 return true;
}
