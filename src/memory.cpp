// SPDX-License-Identifier: GPL-3.0-or-later
#include "memory.h"
#include <string.h>
#include <stdlib.h>
namespace natural {
bool Memory::bytes(void *p,unsigned n){if(!ok||n>capacity-size){ok=false;return false;}if(data){if(reading)memcpy(p,data+size,n);else memcpy(data+size,p,n);}const unsigned char *b=(const unsigned char *)p;for(unsigned i=0;i<n;++i){hash^=b[i];hash*=16777619u;}size+=n;return true;}
unsigned Memory::number(unsigned v){unsigned char b[4]={(unsigned char)(v>>24),(unsigned char)(v>>16),(unsigned char)(v>>8),(unsigned char)v};if(!bytes(b,4))return 0;return (unsigned(b[0])<<24)|(unsigned(b[1])<<16)|(unsigned(b[2])<<8)|b[3];}
bool Memory::text(char *p,unsigned cap){unsigned n=number(reading?0:strlen(p));if(!ok||n>=cap){ok=false;return false;}if(!bytes(p,n))return false;if(reading)p[n]=0;return true;}
bool worksheetMemory(Memory &s,Worksheet &w){
 unsigned count=s.number(w.count);if(!s.ok||count>MAX_ENTRIES){s.ok=false;return false;}if(s.reading)w.clear();
 for(unsigned i=0;i<count&&s.ok;++i){Entry &e=w.entries[i];s.text(e.source,sizeof(e.source));s.text(e.result,sizeof(e.result));unsigned state=s.number(e.state),size=s.number(e.formSize);
  if(!s.ok||state>FAILED||size>MAX_FORM||!size){s.ok=false;break;}s.bytes(e.form,size);
  if(s.reading){e.state=(ResultState)state;e.formSize=size;if(!w.scratch.unpack(e.form,size)){s.ok=false;break;}++w.count;w.updateEntry(i);}
 }
 // Save a suspended edit as the active input, preserving the existing record format.
 unsigned selected=s.number(w.suspendedInput>=0?w.suspendedInput:w.selected),editing=s.number(w.editing||w.suspendedInput>=0),result=s.number(w.suspendedInput>=0?0:w.resultFocus);
 if(!s.ok||selected>count||editing>1||result>1){s.ok=false;if(s.reading)w.clear();return false;}
 unsigned char form[MAX_FORM];unsigned size=s.reading?0:w.input.pack(form,sizeof(form));size=s.number(size);
 if(!size||size>MAX_FORM){s.ok=false;return false;}s.bytes(form,size);
 int path[MAX_DEPTH],depth=0,before=0;
 if(!s.reading){for(int id=w.input.row;id!=w.input.root;id=w.input.n[id].parent){int parent=w.input.n[id].parent,index=0;for(int c=w.input.n[parent].first;c&&c!=id;c=w.input.n[c].next)++index;path[depth++]=index;}
  for(int c=w.input.n[w.input.row].first;c&&c!=w.input.before;c=w.input.n[c].next)++before;
 }
 unsigned levels=s.number(depth);if(levels>=MAX_DEPTH){s.ok=false;return false;}
 for(unsigned i=0;i<levels;++i)path[i]=s.number(s.reading?0:path[i]);before=s.number(before);
 if(s.reading){if(!s.ok){w.clear();return false;}w.editing=false;w.select(selected,result);if(editing){
   if(!w.input.unpack(form,size)){s.ok=false;w.clear();return false;}int row=w.input.root;for(int i=levels-1;i>=0&&row;--i)row=w.input.child(row,path[i]);
   if(!row||w.input.n[row].kind!=ROW||before>w.input.childCount(row)){s.ok=false;w.clear();return false;}w.input.row=row;w.input.before=w.input.child(row,before);w.editing=true;w.resultFocus=false;w.followCursor=true;w.measure();w.cursorVisible();
  }}return s.ok;
}
// Linear-time small dictionary codec: literal runs and 4 KB back-references.
unsigned compressMemory(const unsigned char *in,unsigned length,unsigned char *out,unsigned cap){
 int *table=(int *)malloc(4096*sizeof(int));if(!table)return 0;for(int i=0;i<4096;++i)table[i]=-1;
 unsigned pos=0,w=0,start=0;bool ok=true;
 while(pos<length&&ok){unsigned match=0,distance=0;
  if(pos+2<length){unsigned key=((unsigned(in[pos])*251+in[pos+1])*251+in[pos+2])&4095;int candidate=table[key];table[key]=pos;
   if(candidate>=0&&pos-unsigned(candidate)<=4096){while(match<10&&pos+match<length&&in[candidate+match]==in[pos+match])++match;if(match>=3)distance=pos-candidate;else match=0;}}
  if(match||pos-start==128){unsigned n=pos-start;if(n){if(w+1+n>cap){ok=false;break;}out[w++]=n-1;memcpy(out+w,in+start,n);w+=n;}start=pos;}
  if(match){if(w+2>cap){ok=false;break;}unsigned token=0x8000|((match-3)<<12)|(distance-1);out[w++]=token>>8;out[w++]=token;pos+=match;start=pos;}
  else ++pos;
 }
 if(ok&&pos>start){unsigned n=pos-start;if(w+1+n>cap)ok=false;else{out[w++]=n-1;memcpy(out+w,in+start,n);w+=n;}}
 free(table);return ok?w:0;
}
bool decompressMemory(const unsigned char *in,unsigned length,unsigned char *out,unsigned cap){
 unsigned p=0,w=0;while(p<length){unsigned token=in[p++];if(token&128){if(p==length)return false;token=(token<<8)|in[p++];unsigned n=((token>>12)&7)+3,d=(token&4095)+1;if(d>w||n>cap-w)return false;for(unsigned i=0;i<n;++i){out[w]=out[w-d];++w;}}
  else {unsigned n=token+1;if(n>length-p||n>cap-w)return false;memcpy(out+w,in+p,n);p+=n;w+=n;}}
 return w==cap;
}
}
