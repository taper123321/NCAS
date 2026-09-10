// NaturalCAS - Copyright (C) 2026. SPDX-License-Identifier: GPL-3.0-or-later
#include "editor.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

namespace natural {
static int maximum(int a,int b){return a>b?a:b;}
static int minimum(int a,int b){return a<b?a:b;}
Editor::Editor(){clear();}
void Editor::clear(){memset(n,0,sizeof(n));error[0]=0;root=allocate(ROW);row=root;before=0;}
int Editor::available() const {int c=0;for(int i=1;i<MAX_NODES;++i)if(!n[i].kind)++c;return c;}
int Editor::allocate(Kind kind){
  for(int i=1;i<MAX_NODES;++i)if(!n[i].kind){memset(n+i,0,sizeof(Node));n[i].kind=kind;return i;}
  strcpy(error,"Expression is full. Delete a term or start a new line.");return 0;
}
int Editor::depth(int id) const {int d=0;while(id && d<MAX_NODES){++d;id=n[id].parent;}return d;}
int Editor::subtreeHeight(int id) const {int h=0;for(int c=n[id].first;c;c=n[c].next)h=maximum(h,subtreeHeight(c));return h+1;}
void Editor::unlink(int id){
  int p=n[id].parent;if(!p)return;
  if(n[id].prev)n[n[id].prev].next=n[id].next;else n[p].first=n[id].next;
  if(n[id].next)n[n[id].next].prev=n[id].prev;else n[p].last=n[id].prev;
  n[id].parent=n[id].prev=n[id].next=0;
}
void Editor::release(int id){if(!id)return;while(n[id].first)release(n[id].first);unlink(id);memset(n+id,0,sizeof(Node));}
void Editor::linkBefore(int p,int id,int b){
  int prev=b?n[b].prev:n[p].last;
  n[id].parent=p;n[id].prev=prev;n[id].next=b;
  if(prev)n[prev].next=id;else n[p].first=id;
  if(b)n[b].prev=id;else n[p].last=id;
}
void Editor::append(int p,int id){linkBefore(p,id,0);}
int Editor::makeRow(int p){int a=allocate(ROW);if(a)append(p,a);return a;}
int Editor::child(int id,int index) const {int c=n[id].first;while(c && index-->0)c=n[c].next;return c;}
int Editor::childCount(int id) const {int c=0;for(int a=n[id].first;a;a=n[a].next)++c;return c;}
bool Editor::implicitProduct(int id) const {
  if(!id||n[id].kind!=TEXT||strcmp(n[id].text,"*"))return false;
  bool quoted=false;for(int c=n[n[id].parent].first;c&&c!=id;c=n[c].next)if(n[c].kind==TEXT&&!strcmp(n[c].text,"\""))quoted=!quoted;if(quoted)return false;
  int left=n[id].prev,right=n[id].next;if(!left||!right)return false;
  // Only integer coefficients followed by a single-letter variable (or its power).
  int variable=right;
  if(n[right].kind==POWER){int base=n[right].first;if(childCount(base)!=1)return false;variable=n[base].first;}
  if(n[variable].kind!=TEXT||strlen(n[variable].text)!=1||!isalpha((unsigned char)n[variable].text[0]))return false;
  int next=n[right].next;
  if(next&&n[next].kind==TEXT&&(isalnum((unsigned char)n[next].text[0])||n[next].text[0]=='_'))return false;
  bool digit=false;
  while(left&&n[left].kind==TEXT&&strlen(n[left].text)==1&&isdigit((unsigned char)n[left].text[0])){digit=true;left=n[left].prev;}
  if(!digit)return false;
  if(left&&(n[left].kind!=TEXT||!strchr("+-=,([:",n[left].text[0])))return false;
  if(left&&n[left].prev&&n[n[left].prev].kind==TEXT&&strchr("eE",n[n[left].prev].text[0]))return false;
  return true;
}
bool Editor::insert(const char *text){
  if(!strcmp(text,"pi")||!strcmp(text,"ans()")){if(!available())return false;int a=allocate(TEXT);strcpy(n[a].text,text);linkBefore(row,a,before);error[0]=0;return true;}
  int length=(int)strlen(text);if(length>available()){strcpy(error,"Expression is full.");return false;}
  for(int i=0;i<length;++i){int a=allocate(TEXT);n[a].text[0]=text[i];linkBefore(row,a,before);}
  error[0]=0;return true;
}
static bool isOperator(char c){return c=='+'||c=='-'||c=='*'||c=='/'||c=='^'||c=='='||c==','||c=='<'||c=='>'||c==':';}
int Editor::operandStart(int p) const {
  if(!p)return 0;
  if(n[p].kind!=TEXT)return p;
  char c=n[p].text[0];if(isOperator(c)||c=='('||c=='[')return 0;
  if(c==')'||c==']'){
    int balance=1;char close=c,open=c==')'?'(':'[';int a=n[p].prev;
    while(a){if(n[a].kind==TEXT){if(n[a].text[0]==close)++balance;if(n[a].text[0]==open)--balance;}
      if(!balance){int start=a;
        // Capture an explicitly typed function name together with its arguments.
        while(n[start].prev&&n[n[start].prev].kind==TEXT&&(isalpha((unsigned char)n[n[start].prev].text[0])||n[n[start].prev].text[0]=='_'))start=n[start].prev;
        return start;
      }a=n[a].prev;}
    return p;
  }
  // Capture a number or a single variable; 2x^2 means 2*(x^2).
  if(isalpha((unsigned char)c))return p;
  int a=p;
  while(n[a].prev){int q=n[a].prev;if(n[q].kind!=TEXT)break;char t=n[q].text[0];if(!isdigit((unsigned char)t)&&t!='.')break;a=q;}
  return a;
}
bool Editor::structure(Kind kind,int count,const char *name,bool capture,bool all){
  if(depth(row)+3>MAX_DEPTH || available()<count+1){strcpy(error,"Too many nested templates. Finish this expression first.");return false;}
  int start=0,end=before;
  if(all){row=root;before=0;start=n[root].first;end=0;}
  else if(capture)start=operandStart(before?n[before].prev:n[row].last);
  // Wrapping an existing subtree adds two levels to all its descendants.
  // Checking only the cursor depth would let repeated wrapping exceed the bound.
  if(start)for(int c=start;c!=end;c=n[c].next)
    if(depth(row)+2+subtreeHeight(c)>MAX_DEPTH){strcpy(error,"Too many nested templates. Finish this expression first.");return false;}
  int a=allocate(kind);strncpy(n[a].text,name?name:"",MAX_TEXT-1);
  for(int i=0;i<count;++i)makeRow(a);
  linkBefore(row,a,start?start:before);
  if(start){int dst=child(a,0);for(int c=start;c!=end;){int next=n[c].next;unlink(c);append(dst,c);c=next;}}
  row=child(a,(capture && start && count>1)?1:0);before=0;
  error[0]=0;return true;
}
bool Editor::fraction(){return structure(FRACTION,2,"",true,false);}
bool Editor::power(){return structure(POWER,2,"",true,false);}
bool Editor::square(){if(!power())return false;if(!insert("2"))return false;leave();return true;}
bool Editor::radical(bool indexed){return structure(ROOT,indexed?2:1,"sqrt",false,false);}
bool Editor::exponential(const char *base){if(available()<5)return false;if(!insert(base))return false;return power();}
bool Editor::empty(int id) const {if(n[id].kind==TEXT)return !n[id].text[0];for(int c=n[id].first;c;c=n[c].next)if(!empty(c))return false;return true;}
bool Editor::group(){return structure(GROUP,1,"",false,false);}
bool Editor::matrix(int r,int c){
  if(r<1||r>6||c<1||c>6){strcpy(error,"Use 1 to 6 rows and columns in the inline grid.");return false;}
  if(!structure(MATRIX,r*c,"",false,false))return false;
  int m=n[row].parent;n[m].rows=r;n[m].cols=c;return true;
}
int Editor::activeMatrix() const {int a=row;while(a){if(n[a].kind==MATRIX)return a;a=n[a].parent;}return 0;}
bool Editor::resizeMatrix(int dr,int dc){
  int m=activeMatrix();if(!m){strcpy(error,"Move into a matrix cell first.");return false;}
  int nr=n[m].rows+dr,nc=n[m].cols+dc,orows=n[m].rows,oc=n[m].cols;
  if(nr<1||nr>6||nc<1||nc>6){strcpy(error,"Inline matrices support 1 to 6 rows and columns.");return false;}
  if(available()<maximum(0,nr*nc-orows*oc)){strcpy(error,"Expression is full.");return false;}
  // Refuse implicit data loss when shrinking a non-empty edge.
  for(int r=0;r<orows;++r)for(int c=0;c<oc;++c)
    if((r>=nr||c>=nc)&&n[child(m,r*oc+c)].first){strcpy(error,"Clear the last row or column before removing it.");return false;}
  int old[36],active=row;while(n[active].parent!=m)active=n[active].parent;
  for(int i=0;i<orows*oc;++i)old[i]=child(m,i);
  for(int i=0;i<orows*oc;++i)unlink(old[i]);
  bool kept=false;
  for(int r=0;r<nr;++r)for(int c=0;c<nc;++c){int a=(r<orows&&c<oc)?old[r*oc+c]:allocate(ROW);append(m,a);if(a==active)kept=true;}
  if(!kept){row=child(m,0);before=0;}
  for(int r=0;r<orows;++r)for(int c=0;c<oc;++c)if(r>=nr||c>=nc)release(old[r*oc+c]);
  n[m].rows=nr;n[m].cols=nc;error[0]=0;return true;
}
bool Editor::call(const char *name,int count,bool all){
  if(count<1||count>6||strlen(name)>=MAX_TEXT){strcpy(error,"Unsupported template.");return false;}
  return structure(CALL,count,name,all,all);
}
bool Editor::setArgument(int call,int argument,const char *text){
  int r=child(call,argument);if(!r)return false;
  int savedRow=row,savedBefore=before;row=r;before=0;
  bool ok=parsePart(r,text,0,(int)strlen(text),0);row=savedRow;before=savedBefore;return ok;
}
bool Editor::nextArgument(){
  int p=n[row].parent;
  if(p&&n[p].kind==CALL){
    if(n[row].next){row=n[row].next;before=n[row].first;return true;}
    if(available()<1||childCount(p)>=16){strcpy(error,"This template has reached its argument limit.");return false;}
    row=makeRow(p);before=0;return true;
  }
  if(p&&n[p].kind==MATRIX){move(NEXT);return true;}
  return insert(",");
}
void Editor::leave(){if(row==root)return;int p=n[row].parent;row=n[p].parent;before=n[p].next;}
void Editor::seekCursor(int x,int y){
  int best=0x7fffffff,bestRow=row,bestBefore=before;
  for(int r=1;r<MAX_NODES;++r)if(n[r].kind==ROW){
    for(int c=n[r].first;;c=c?n[c].next:0){
      if(!implicitProduct(c)){
        int xx=c?n[c].x:n[r].x+(n[r].first?n[r].w:3),dx=xx-x,dy=n[r].y-y;
        if(dx<0)dx=-dx;if(dy<0)dy=-dy;int score=dy*4+dx;
        if(score<best){best=score;bestRow=r;bestBefore=c;}
      }
      if(!c)break;
    }
  }
  row=bestRow;before=bestBefore;
}
void Editor::move(Direction d){
  if(d==NEXT||d==PREVIOUS){
    int p=n[row].parent;if(!p)return;int adjacent=d==NEXT?n[row].next:n[row].prev;
    if(adjacent){row=adjacent;before=d==NEXT?n[row].first:0;}else leave();return;
  }
  if(d==UP||d==DOWN){
    int cell=row,p=n[cell].parent;
    while(p){
      int idx=0;for(int c=n[p].first;c&&c!=cell;c=n[c].next)++idx;
      int target=-1,count=childCount(p);
      if(n[p].kind==MATRIX)target=idx+(d==UP?-n[p].cols:n[p].cols);
      else if(n[p].kind==FRACTION)target=d==UP?0:1;
      else if(n[p].kind==POWER||n[p].kind==ROOT)target=d==UP?1:0;
      else if(n[p].kind==CALL)target=idx+(d==UP?-1:1);
      if(target>=0&&target<count&&target!=idx){row=child(p,target);before=0;return;}
      cell=n[p].parent;p=cell?n[cell].parent:0;
    }
    return;
  }
  int a=d==LEFT?(before?n[before].prev:n[row].last):before;
  while(a&&implicitProduct(a))a=d==LEFT?n[a].prev:n[a].next;
  if(a){
    if(n[a].kind==TEXT||!n[a].first){before=d==LEFT?a:n[a].next;if(d==RIGHT&&implicitProduct(before))before=n[before].next;return;}
    row=d==LEFT?n[a].last:n[a].first;before=d==LEFT?0:n[row].first;return;
  }
  if(row==root)return;
  int p=n[row].parent,adjacent=d==LEFT?n[row].prev:n[row].next;
  if(adjacent){row=adjacent;before=d==LEFT?0:n[row].first;}
  else {row=n[p].parent;before=d==LEFT?p:n[p].next;}
}
bool Editor::backspace(){
  int a=before?n[before].prev:n[row].last;
  if(implicitProduct(a)){
    int coefficient=n[a].prev,previous=n[coefficient].prev;
    if(!previous||n[previous].kind!=TEXT||!isdigit((unsigned char)n[previous].text[0]))release(a);
    a=coefficient;
  }
  if(a){if(n[a].kind!=TEXT&&n[a].first&&!empty(a)){row=n[a].last;before=0;}else release(a);return true;}
  if(row==root)return false;
  int p=n[row].parent;
  if(empty(p)){row=n[p].parent;before=n[p].next;release(p);return true;}
  move(LEFT);return true;
}
bool Editor::removeStructure(){
  int p=n[row].parent;if(!p)return false;
  row=n[p].parent;before=n[p].next;release(p);return true;
}
void Editor::clearField(){while(n[row].first)release(n[row].first);before=0;error[0]=0;}
void Editor::swap(Editor &other){
  for(int i=0;i<MAX_NODES;++i){Node t=n[i];n[i]=other.n[i];other.n[i]=t;}
  int t=root;root=other.root;other.root=t;t=row;row=other.row;other.row=t;t=before;before=other.before;other.before=t;
  for(unsigned i=0;i<sizeof(error);++i){char c=error[i];error[i]=other.error[i];other.error[i]=c;}
}
bool Editor::write(char *out,int size,int &pos,const char *s){
  int l=(int)strlen(s);if(pos+l>=size){strcpy(error,"Expression is too long to send to the algebra engine.");return false;}
  memcpy(out+pos,s,l);pos+=l;out[pos]=0;return true;
}
bool Editor::writeNode(int id,char *out,int size,int &pos,bool complete,int level){
  if(level>MAX_DEPTH){strcpy(error,"Expression is nested too deeply.");return false;}
  const Node &a=n[id];
  if(a.kind==TEXT)return write(out,size,pos,a.text);
  if(a.kind==ROW){
    if(!a.first&&complete){row=id;before=0;strcpy(error,"Fill the highlighted box before calculating.");return false;}
    // Literal brackets belong to the user. Reject an unfinished pair; never add it.
    char brackets[MAX_NODES];int nb=0;bool quoted=false;
    if(complete)for(int c=a.first;c;c=n[c].next)if(n[c].kind==TEXT){
      char t=n[c].text[0];if(t=='"')quoted=!quoted;if(quoted)continue;
      if(t=='('||t=='[')brackets[nb++]=t;
      if(t==')'||t==']'){
        if(!nb||brackets[--nb]!=(t==')'?'(':'[')){row=id;before=c;strcpy(error,"Check this closing bracket.");return false;}
      }
    }
    if(complete&&nb){row=id;before=0;strcpy(error,"Close the open bracket before calculating.");return false;}
    for(int c=a.first;c;c=n[c].next){
      int p=n[c].prev;
      if(p){
        bool leftValue=n[p].kind!=TEXT || isalnum((unsigned char)n[p].text[0]) || n[p].text[0]==')' || n[p].text[0]==']';
        bool rightValue=n[c].kind!=TEXT || isalnum((unsigned char)n[c].text[0]);
        if(leftValue&&rightValue&&(n[p].kind!=TEXT||n[c].kind!=TEXT))if(!write(out,size,pos,"*"))return false;
      }
      if(!writeNode(c,out,size,pos,complete,level+1))return false;
    }
    return true;
  }
  const char *open="(",*close=")",*sep=",";
  if(a.kind==FRACTION){open="((";sep=")/(";close="))";}
  if(a.kind==POWER){open="((";sep=")^(";close="))";}
  if(a.kind==ROOT&&childCount(id)==2){
    if(!write(out,size,pos,"surd("))return false;
    if(!writeNode(a.first,out,size,pos,complete,level+1)||!write(out,size,pos,",")||!writeNode(a.last,out,size,pos,complete,level+1))return false;
    return write(out,size,pos,")");
  }
  if(a.kind==ROOT)open="sqrt(";
  if(a.kind==CALL&&!write(out,size,pos,a.text))return false;
  if(a.kind==MATRIX){
    if(!write(out,size,pos,"["))return false;
    for(int r=0;r<a.rows;++r){if(r&&!write(out,size,pos,","))return false;if(!write(out,size,pos,"["))return false;
      for(int c=0;c<a.cols;++c){if(c&&!write(out,size,pos,","))return false;if(!writeNode(child(id,r*a.cols+c),out,size,pos,complete,level+1))return false;}
      if(!write(out,size,pos,"]"))return false;}
    return write(out,size,pos,"]");
  }
  if(!write(out,size,pos,open))return false;
  for(int c=a.first;c;c=n[c].next){if(c!=a.first&&!write(out,size,pos,sep))return false;if(!writeNode(c,out,size,pos,complete,level+1))return false;}
  return write(out,size,pos,close);
}
bool Editor::serialize(char *out,int size,bool complete){
  if(size<1)return false;out[0]=0;error[0]=0;int pos=0;
  if(!writeNode(root,out,size,pos,complete,0)){out[0]=0;return false;}return true;
}
static int matching(const char *s,int b,int e){
  char open=s[b],close=open=='('?')':']';int count=0;
  for(int i=b;i<e;++i){if(s[i]==open)++count;if(s[i]==close&&!--count)return i;}return -1;
}
bool Editor::parsePart(int id,const char *s,int b,int e,int level){
  if(level>MAX_DEPTH-3)return false;
  while(b<e&&s[b]==' ')++b;while(e>b&&s[e-1]==' ')--e;
  if(b==e)return true;
  // Split at the weakest top-level operator. Preserve grouping explicitly.
  int nesting=0,cut=-1,priority=99;
  for(int i=b;i<e;++i){
    char c=s[i];if(c=='('||c=='[')++nesting;else if(c==')'||c==']')--nesting;
    if(nesting<0)return false;if(nesting)continue;
    int p=99;
    if(c=='='||c==',')p=0;
    if((c=='+'||c=='-')&&i>b&&!isOperator(s[i-1])&&s[i-1]!='('&&s[i-1]!='e'&&s[i-1]!='E')p=1;
    if(c=='*'||c=='/')p=2;
    if(c=='^')p=3;
    if(p<99&&(p<priority||(p==priority&&c!='^'))){cut=i;priority=p;}
  }
  if(nesting)return false;
  // Unary minus has lower precedence than powers: -x^2 = -(x^2).
  if(priority>=3&&(s[b]=='-'||s[b]=='+')){
    int a=allocate(TEXT);if(!a)return false;n[a].text[0]=s[b];append(id,a);return parsePart(id,s,b+1,e,level+1);
  }
  if(cut>=0){
    if(s[cut]=='/'||s[cut]=='^'){
      int a=allocate(s[cut]=='/'?FRACTION:POWER);if(!a)return false;append(id,a);
      int l=makeRow(a),r=makeRow(a);return l&&r&&parsePart(l,s,b,cut,level+1)&&parsePart(r,s,cut+1,e,level+1);
    }
    if(!parsePart(id,s,b,cut,level+1))return false;
    int a=allocate(TEXT);if(!a)return false;n[a].text[0]=s[cut];append(id,a);return parsePart(id,s,cut+1,e,level+1);
  }
  return parseRow(id,s,b,e,level+1);
}
bool Editor::parseRow(int id,const char *s,int b,int e,int level){
  if(level>MAX_DEPTH-2)return false;
  for(int i=b;i<e;){
    if(isspace((unsigned char)s[i])){++i;continue;}
    if(!strncmp(s+i,"matrix[",7)){i+=6;continue;}
    if(s[i]=='('){int end=matching(s,i,e);if(end<0)return false;int a=allocate(GROUP);if(!a)return false;append(id,a);int r=makeRow(a);if(!r||!parsePart(r,s,i+1,end,level+1))return false;i=end+1;continue;}
    int next=i+1;while(next<e&&isspace((unsigned char)s[next]))++next;
    if(s[i]=='['&&next<e&&s[next]=='['){
      int end=matching(s,i,e);if(end<0)return false;int a=allocate(MATRIX);if(!a)return false;append(id,a);int pos=next,rows=0,cols=0;
      while(pos<end){
        if(s[pos]!='[')return false;int re=matching(s,pos,end);if(re<0)return false;int start=pos+1,nest=0,cc=0;
        for(int k=start;k<=re;++k){
          if(k==re||(s[k]==','&&nest==0)){int r=makeRow(a);if(!r||!parsePart(r,s,start,k,level+1))return false;++cc;start=k+1;}
          else if(s[k]=='('||s[k]=='[')++nest;else if(s[k]==')'||s[k]==']')--nest;
        }
        if(rows&&cc!=cols)return false;cols=cc;++rows;if(rows>6||cols>6)return false;
        pos=re+1;while(pos<end&&isspace((unsigned char)s[pos]))++pos;if(pos<end){if(s[pos]!=',')return false;++pos;while(pos<end&&isspace((unsigned char)s[pos]))++pos;}
      }
      n[a].rows=rows;n[a].cols=cols;i=end+1;continue;
    }
    if(isalpha((unsigned char)s[i])||s[i]=='_'){
      int k=i;while(k<e&&(isalnum((unsigned char)s[k])||s[k]=='_'))++k;
      if(k-i==2&&s[i]=='p'&&s[i+1]=='i'){int a=allocate(TEXT);if(!a)return false;strcpy(n[a].text,"pi");append(id,a);i=k;continue;}
      if(k<e&&s[k]=='('&&k-i<MAX_TEXT){
        int end=matching(s,k,e);if(end<0)return false;int a=allocate(CALL);if(!a)return false;append(id,a);memcpy(n[a].text,s+i,k-i);
        if(!strcmp(n[a].text,"sqrt")||!strcmp(n[a].text,"surd"))n[a].kind=ROOT;
        if(k+1<end){int start=k+1,nest=0;
          for(int p=start;p<=end;++p){if(p==end||(s[p]==','&&!nest)){int r=makeRow(a);if(!r||!parsePart(r,s,start,p,level+1))return false;start=p+1;}
            else if(s[p]=='('||s[p]=='[')++nest;else if(s[p]==')'||s[p]==']')--nest;}}
        if(n[a].kind==ROOT&&!n[a].first)makeRow(a);
        i=end+1;continue;
      }
    }
    int a=allocate(TEXT);if(!a)return false;n[a].text[0]=s[i++];append(id,a);
  }
  return true;
}
bool Editor::load(const char *s){
  if(strlen(s)>=MAX_SOURCE){strcpy(error,"Expression is too long for the inline editor.");return false;}
  clear();
  // Programming, strings and large/unsupported objects retain literal text.
  bool literal=strchr(s,'"')||strchr(s,';')||strchr(s,':')||strchr(s,'\'');
  if(!literal && parsePart(root,s,0,(int)strlen(s),0)){row=root;before=0;return true;}
  clear();return insert(s);
}

bool Editor::packNode(int id,unsigned char *out,int size,int &p) const {
  const Node &a=n[id];int len=(int)strlen(a.text),count=childCount(id);
  if(p+6+len>size)return false;
  out[p++]=(unsigned char)a.kind;out[p++]=(unsigned char)(count>>8);out[p++]=(unsigned char)count;
  out[p++]=(unsigned char)a.rows;out[p++]=(unsigned char)a.cols;out[p++]=(unsigned char)len;
  memcpy(out+p,a.text,len);p+=len;
  for(int c=a.first;c;c=n[c].next)if(!packNode(c,out,size,p))return false;
  return true;
}
int Editor::pack(unsigned char *out,int size) const {int p=0;return packNode(root,out,size,p)?p:0;}
int Editor::unpackNode(int parent,const unsigned char *data,int size,int &p,int level){
  if(level>MAX_DEPTH||p+6>size)return 0;
  int kind=data[p++],count=data[p++]*256;count+=data[p++];
  int rows=data[p++],cols=data[p++],len=data[p++];
  if(kind<ROW||kind>GROUP||len>=MAX_TEXT||p+len>size)return 0;
  if((kind==TEXT&&count)||(kind==MATRIX&&(rows<1||rows>6||cols<1||cols>6||count!=rows*cols)))return 0;
  if((kind==FRACTION||kind==POWER)&&count!=2)return 0;
  if(kind==ROOT&&(count<1||count>2))return 0;
  if(kind==GROUP&&count!=1)return 0;
  if(!parent&&kind!=ROW)return 0;
  if(parent&&((n[parent].kind==ROW)==(kind==ROW)))return 0;
  int a=allocate((Kind)kind);if(!a)return 0;if(parent)append(parent,a);
  n[a].rows=rows;n[a].cols=cols;memcpy(n[a].text,data+p,len);p+=len;
  for(int c=0;c<count;++c)if(!unpackNode(a,data,size,p,level+1))return 0;
  return a;
}
bool Editor::unpack(const unsigned char *data,int size){
  memset(n,0,sizeof(n));error[0]=0;int p=0;root=unpackNode(0,data,size,p,0);row=root;before=0;
  if(root&&p==size&&validate())return true;
  clear();strcpy(error,"Cannot restore this expression.");return false;
}

void Editor::measure(int id,int scale,int level){
  Node &a=n[id];a.scale=scale;
  if(level>MAX_DEPTH){a.w=12;a.up=8;a.down=8;return;}
  if(a.kind==TEXT){a.w=implicitProduct(id)?0:6*scale*(!strcmp(a.text,"pi")?1:!strcmp(a.text,"ans()")?3:(int)strlen(a.text));a.up=7*scale;a.down=2*scale;return;}
  if(a.kind==CALL&&!a.first&&!strcmp(a.text,"ans")){a.w=18*scale;a.up=7*scale;a.down=2*scale;return;}
  if(a.kind==ROW){a.w=0;a.up=7*scale;a.down=2*scale;for(int c=a.first;c;c=n[c].next){measure(c,scale,level+1);a.w+=n[c].w;a.up=maximum(a.up,n[c].up);a.down=maximum(a.down,n[c].down);}if(!a.first)a.w=10*scale;return;}
  if(a.kind==FRACTION){for(int c=a.first;c;c=n[c].next)measure(c,scale,level+1);Node &t=n[a.first],&b=n[a.last];a.w=maximum(t.w,b.w)+8;a.up=t.up+t.down+5;a.down=b.up+b.down+5;return;}
  if(a.kind==POWER){measure(a.first,scale,level+1);measure(a.last,1,level+1);Node &b=n[a.first],&e=n[a.last];a.w=b.w+e.w+3;a.up=b.up+e.up+e.down-3;a.down=b.down;return;}
  if(a.kind==MATRIX){int widths[6]={0},ups[6]={0},downs[6]={0};for(int r=0;r<a.rows;++r)for(int c=0;c<a.cols;++c){int v=child(id,r*a.cols+c);measure(v,scale,level+1);widths[c]=maximum(widths[c],n[v].w);ups[r]=maximum(ups[r],n[v].up);downs[r]=maximum(downs[r],n[v].down);}
    a.w=12;int height=0;for(int c=0;c<a.cols;++c)a.w+=widths[c]+12;for(int r=0;r<a.rows;++r)height+=ups[r]+downs[r]+8;a.up=height/2+4;a.down=height-a.up+4;return;}
  if(measureNotation(id,scale,level))return;
  if(a.kind==CALL&&!strcmp(a.text,"diff")&&childCount(id)==2){
    measure(a.first,scale,level+1);measure(a.last,scale,level+1);a.w=maximum(30,n[a.last].w+6*scale+10)+n[a.first].w+20;a.up=maximum(7*scale+5,n[a.first].up);a.down=maximum(9*scale+5,n[a.first].down);return;
  }
  if(a.kind==CALL&&!strcmp(a.text,"integrate")&&(childCount(id)==2||childCount(id)==4)){
    for(int c=a.first;c;c=n[c].next)measure(c,c==a.first||c==child(id,1)?scale:1,level+1);
    a.w=36+n[a.first].w+10+6*scale+n[child(id,1)].w;a.up=maximum(scale==1?13:23,n[a.first].up+4);a.down=maximum(scale==1?13:23,n[a.first].down+4);
    if(childCount(id)==4){a.up+=n[a.last].up+n[a.last].down;a.down+=n[child(id,2)].up+n[child(id,2)].down;}
    return;
  }
  a.w=a.kind==CALL?((int)strlen(a.text)+1)*6*scale:12;
  a.up=7*scale;a.down=2*scale;
  for(int c=a.first;c;c=n[c].next){measure(c,scale,level+1);a.w+=n[c].w+(c==a.first?0:8*scale);a.up=maximum(a.up,n[c].up);a.down=maximum(a.down,n[c].down);}
  if(a.kind==ROOT){a.w+=8;a.up+=5;}else a.w+=6*scale;
}
void Editor::position(int id,int x,int y){
  Node &a=n[id];a.x=x;a.y=y;
  if(a.kind==TEXT)return;
  if(a.kind==ROW){for(int c=a.first;c;c=n[c].next){position(c,x,y);x+=n[c].w;}return;}
  if(a.kind==FRACTION){position(a.first,x+(a.w-n[a.first].w)/2,y-5-n[a.first].down);position(a.last,x+(a.w-n[a.last].w)/2,y+5+n[a.last].up);return;}
  if(a.kind==POWER){position(a.first,x,y);position(a.last,x+n[a.first].w+2,y-n[a.first].up-n[a.last].down+3);return;}
  if(a.kind==MATRIX){int widths[6]={0},ups[6]={0},downs[6]={0};for(int r=0;r<a.rows;++r)for(int c=0;c<a.cols;++c){Node &v=n[child(id,r*a.cols+c)];widths[c]=maximum(widths[c],v.w);ups[r]=maximum(ups[r],v.up);downs[r]=maximum(downs[r],v.down);}
    int yy=y-a.up+4;for(int r=0;r<a.rows;++r){int xx=x+12;yy+=ups[r];for(int c=0;c<a.cols;++c){int v=child(id,r*a.cols+c);position(v,xx+(widths[c]-n[v].w)/2,yy);xx+=widths[c]+12;}yy+=downs[r]+8;}return;}
  if(positionNotation(id,x,y))return;
  if(a.kind==CALL&&!strcmp(a.text,"diff")&&childCount(id)==2){int w=maximum(30,n[a.last].w+6*a.scale+10);position(a.first,x+w+10,y);position(a.last,x+6+6*a.scale,y+5+7*a.scale);return;}
  if(a.kind==CALL&&!strcmp(a.text,"integrate")&&(childCount(id)==2||childCount(id)==4)){
    position(a.first,x+34,y);position(child(id,1),x+34+n[a.first].w+10+6*a.scale,y);
    if(childCount(id)==4){position(child(id,2),x+12,y+a.down-3);position(a.last,x+20,y-a.up+n[a.last].up);}
    return;
  }
  x+=a.kind==CALL?((int)strlen(a.text)+1)*6*a.scale:(a.kind==ROOT?18:8);
  for(int c=a.first;c;c=n[c].next){position(c,x,y);x+=n[c].w+8*a.scale;}
}
#include "notation.inc"
void Editor::layout(int scale){measure(root,scale,0);position(root,0,0);}
int Editor::cursorX() const {return before?n[before].x:n[row].x+n[row].w-(n[row].first?0:n[row].w-3);}
int Editor::cursorY() const {return n[row].y;}
void Editor::draw(int id,Painter &p,int ox,int oy,bool cursor){
  Node &a=n[id];int x=a.x+ox,y=a.y+oy,s=a.scale;
  if(a.kind==CALL&&!a.first&&!strcmp(a.text,"ans")){p.text(x,y-7*s,"Ans",s,INK);return;}
  if(a.kind==TEXT){
    if(implicitProduct(id))return;
    // Both operators use the same bitmap alphabet and baseline as the digits.
    int parent=n[a.parent].parent;bool approach=parent&&notation(parent)==LIMIT_FORM&&a.parent==n[parent].last&&!strcmp(a.text,"=");
    p.text(x,y-7*s,approach?"\x1a":!strcmp(a.text,"/")?"\x1f":!strcmp(a.text,"pi")?"\x1e":!strcmp(a.text,"ans()")?"Ans":a.text,s,INK);return;
  }
  if(a.kind==ROW && !a.first){p.rect(x,y-7*s,10*s,9*s,(cursor&&row==id)?SELECT:BACKGROUND);p.line(x+2,y+2*s,x+10*s-2,y+2*s,RULE);}
  if(a.kind==FRACTION)p.line(x+1,y,x+a.w-2,y,INK);
  bool special=drawNotation(id,p,x,y);
  if(a.kind==ROOT&&!special){p.line(x+1,y-2,x+5,y-4,INK);p.line(x+5,y-4,x+9,y+3,INK);p.line(x+9,y+3,x+15,y-a.up+2,INK);p.line(x+15,y-a.up+2,x+a.w-2,y-a.up+2,INK);}
  if(a.kind==MATRIX){int t=y-a.up+1,b=y+a.down-1;p.line(x+6,t,x+1,t,INK);p.line(x+1,t,x+1,b,INK);p.line(x+1,b,x+6,b,INK);p.line(x+a.w-6,t,x+a.w-1,t,INK);p.line(x+a.w-1,t,x+a.w-1,b,INK);p.line(x+a.w-1,b,x+a.w-6,b,INK);}
  bool derivative=a.kind==CALL&&!strcmp(a.text,"diff")&&childCount(id)==2;
  bool integral=a.kind==CALL&&!strcmp(a.text,"integrate")&&(childCount(id)==2||childCount(id)==4);
  if(derivative){int w=maximum(30,n[a.last].w+6*s+10);p.text(x+w/2-3*s,y-5-7*s,"d",s,INK);p.line(x+1,y,x+w-3,y,INK);p.text(x+6,y+5,"d",s,INK);p.text(x+w+2,y-7*s,"(",s,INK);p.text(x+a.w-8,y-7*s,")",s,INK);}
  if(integral){int t=y-(s==1?10:20),b=y+(s==1?10:19);p.line(x+24,t,x+18,t-2,INK);p.line(x+18,t-2,x+14,t+3,INK);p.line(x+14,t+3,x+11,b-2,INK);p.line(x+11,b-2,x+6,b+2,INK);p.line(x+6,b+2,x+2,b,INK);p.text(x+34+n[a.first].w+8,y-7*s,"d",s,INK);}
  if(a.kind==GROUP||(a.kind==CALL&&!derivative&&!integral&&!special)){
    int xx=x;if(a.kind==CALL){p.text(x,y-7*s,!strcmp(a.text,"re")?"Re":!strcmp(a.text,"im")?"Im":a.text,s,INK);xx+=((int)strlen(a.text))*6*s;}
    p.line(xx+5,y-a.up+1,xx+2,y-a.up+5,INK);p.line(xx+2,y-a.up+5,xx+2,y+a.down-4,INK);p.line(xx+2,y+a.down-4,xx+5,y+a.down,INK);
    int rx=x+a.w-2;p.line(rx-3,y-a.up+1,rx,y-a.up+5,INK);p.line(rx,y-a.up+5,rx,y+a.down-4,INK);p.line(rx,y+a.down-4,rx-3,y+a.down,INK);
    if(a.kind==CALL)for(int c=a.first;c&&n[c].next;c=n[c].next)p.text(n[c].x+ox+n[c].w+2,y-7*s,",",s,MUTED);
  }
  for(int c=a.first;c;c=n[c].next)draw(c,p,ox,oy,cursor);
}
void Editor::paint(Painter &p,int x,int baseline,bool cursor){
  if(cursor&&activeMatrix()){Node &c=n[row];p.rect(c.x+x-2,c.y+baseline-c.up-2,c.w+4,c.up+c.down+4,SELECT);}
  draw(root,p,x,baseline,cursor);
  if(cursor){int xx=cursorX()+x,yy=cursorY()+baseline;p.rect(xx,yy-7*n[row].scale,2,9*n[row].scale,ACCENT);}
}
const char *Editor::hint(){
  if(error[0])return error;
  int p=n[row].parent;
  if(!p)return "EXE calculate   / fraction   Arrows move";
  if(n[p].kind==FRACTION)return row==n[p].first?"Numerator | DOWN denominator | EXIT out":"Denominator | UP numerator | EXIT out";
  if(n[p].kind==POWER)return row==n[p].last?"Power | DOWN base | EXIT out":"Base | UP power | EXIT out";
  if(activeMatrix())return "Matrix cell | Arrows move | F3 resize";
  return "RIGHT next box | EXIT leave template";
}
bool Editor::validate() const {
  if(root<=0||root>=MAX_NODES||n[root].kind!=ROW||n[root].parent||row<=0||row>=MAX_NODES||n[row].kind!=ROW)return false;
  if(before && n[before].parent!=row)return false;
  for(int i=1;i<MAX_NODES;++i)if(n[i].kind){
    if(depth(i)>MAX_DEPTH)return false;
    if(i!=root && (!n[i].parent||n[i].parent>=MAX_NODES))return false;
    int prev=0,count=0;for(int c=n[i].first;c;c=n[c].next){if(c<=0||c>=MAX_NODES||++count>=MAX_NODES||n[c].parent!=i||n[c].prev!=prev)return false;prev=c;}
    if(prev!=n[i].last)return false;
    if(n[i].kind==MATRIX&&(n[i].rows<1||n[i].rows>6||n[i].cols<1||n[i].cols>6||count!=n[i].rows*n[i].cols))return false;
  }
  return true;
}
}
