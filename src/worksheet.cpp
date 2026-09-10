// SPDX-License-Identifier: GPL-3.0-or-later
#include "worksheet.h"
#include <string.h>
#include <stdio.h>
namespace natural {
static int maxv(int a,int b){return a>b?a:b;}
static int minv(int a,int b){return a<b?a:b;}
Entry::Entry():formSize(0),inputUp(14),inputDown(4),inputWidth(20),resultUp(14),resultDown(4),resultWidth(20),state(PENDING){source[0]=result[0]=0;}
Worksheet::Worksheet():deleted(false),evicted(false),count(0),selected(0),scrollY(0),scrollX(0),scale(2),bodyBottom(192),totalHeight(0),suspendedInput(-1),editing(true),resultFocus(false),panMode(false),followCursor(true),undoReady(false){message[0]=0;}
void Worksheet::snapshot(){deleted=false;undo=input;undoReady=true;followCursor=true;}
void Worksheet::undoEdit(){if(undoReady){input.swap(undo);followCursor=true;}}
void Worksheet::clear(){deleted=evicted=false;count=selected=scrollX=scrollY=0;suspendedInput=-1;editing=true;resultFocus=panMode=false;undoReady=false;input.clear();message[0]=0;followCursor=true;}
void Worksheet::suspendInput(){
  if(!editing)return;
  if(input.n[input.root].first){
    unsigned char form[MAX_FORM];int size=input.pack(form,sizeof(form));
    if(selected==count||size!=entries[selected].formSize||memcmp(form,entries[selected].form,size))suspendedInput=selected;
  }
  editing=false;followCursor=false;
}
bool Worksheet::commit(){
  evicted=false;
  // Validate before changing the committed expression or invalidating any answer.
  char source[MAX_SOURCE];if(!input.serialize(source,sizeof(source)))return false;
  unsigned char form[MAX_FORM];int size=input.pack(form,sizeof(form));
  if(!size){strcpy(message,"Expression layout is too large to save.");return false;}
  deleted=false;
  if(selected==count&&count==MAX_ENTRIES){for(int i=0;i<MAX_ENTRIES-1;++i)entries[i]=entries[i+1];--count;--selected;evicted=true;}
  Entry &e=entries[selected];strcpy(e.source,source);memcpy(e.form,form,size);e.formSize=size;
  if(selected==count)++count;
  for(int i=selected;i<count;++i){entries[i].state=PENDING;entries[i].result[0]=0;}
  suspendedInput=-1;editing=false;undoReady=false;resultFocus=false;scrollX=0;updateEntry(selected);measure();return true;
}
bool Worksheet::select(int i,bool result){
  if(i<0||i>count)return false;
  suspendInput();
  selected=i;editing=i==count&&(suspendedInput<0||suspendedInput==i);resultFocus=result&&i<count;panMode=false;scrollX=0;
  if(suspendedInput<0){undoReady=false;if(i<count)input.unpack(entries[i].form,entries[i].formSize);else input.clear();}
  else if(editing)suspendedInput=-1;
  followCursor=editing;measure();focus(resultFocus);return true;
}
void Worksheet::edit(bool fromRight){
  if(suspendedInput>=0){if(suspendedInput!=selected){strcpy(message,"Return to the unfinished input; EXE or AC.");return;}suspendedInput=-1;editing=true;resultFocus=false;panMode=false;followCursor=true;measure();cursorVisible();return;}
  if(selected>=count){editing=true;return;}
  input.unpack(entries[selected].form,entries[selected].formSize);editing=true;resultFocus=false;panMode=false;undoReady=false;
  input.before=fromRight?0:input.n[input.root].first;followCursor=true;measure();cursorVisible();
}
bool Worksheet::newLine(){
  if(suspendedInput>=0){strcpy(message,"Return to the unfinished input; EXE or AC.");return false;}
  selected=count;input.clear();editing=true;resultFocus=panMode=false;undoReady=false;scrollX=0;followCursor=true;measure();focus(false);return true;
}
int Worksheet::inputHeight(int i) const {if((i==selected&&editing)||i==suspendedInput)return input.n[input.root].up+input.n[input.root].down+6;if(i>=count)return 9*scale+6;return entries[i].inputUp+entries[i].inputDown+6;}
int Worksheet::resultHeight(int i) const {return i<count?entries[i].resultUp+entries[i].resultDown+6:0;}
int Worksheet::entryTop(int i) const {int y=0;for(int k=0;k<i;++k)y+=inputHeight(k)+resultHeight(k)+6;return y;}
void Worksheet::measure(){
  input.layout(scale);
  // Committed dimensions are cached; loading/laying out offscreen history on every key
  // would make a long sheet unnecessarily expensive on the SuperH CPU.
  totalHeight=entryTop(count)+(selected==count||suspendedInput==count?inputHeight(count)+6:0);
  int view=bodyBottom-24;
  if(count&&selected!=count&&inputHeight(count-1)+resultHeight(count-1)>view)
    totalHeight+=maxv(0,view-resultHeight(count-1));
  clampScroll();
}
void Worksheet::updateEntry(int i){
  Entry &e=entries[i];scratch.unpack(e.form,e.formSize);scratch.layout(scale);
  e.inputUp=scratch.n[scratch.root].up;e.inputDown=scratch.n[scratch.root].down;e.inputWidth=scratch.n[scratch.root].w;
  if(e.state==VALID){scratch.load(e.result);scratch.layout(scale);e.resultUp=scratch.n[scratch.root].up;e.resultDown=scratch.n[scratch.root].down;e.resultWidth=scratch.n[scratch.root].w;}
  else {e.resultUp=14;e.resultDown=4;e.resultWidth=200;}
}
void Worksheet::reflow(int textScale){scale=textScale;for(int i=0;i<count;++i)updateEntry(i);measure();if(editing)cursorVisible();else focus(resultFocus);}
void Worksheet::clampScroll(){int h=bodyBottom-24;scrollY=maxv(0,minv(scrollY,maxv(0,totalHeight-h)));scrollX=maxv(0,scrollX);}
void Worksheet::focus(bool result){
  resultFocus=result&&selected<count;
  int top=entryTop(selected)+(resultFocus?inputHeight(selected):0);
  int height=resultFocus?resultHeight(selected):inputHeight(selected),view=bodyBottom-24;
  if(height>view||(resultFocus&&inputHeight(selected)+height>view))scrollY=top;
  else if(top<scrollY)scrollY=top;else if(top+height>scrollY+view)scrollY=top+height-view;
  clampScroll();
}
void Worksheet::cursorVisible(){
  if(!editing||!followCursor)return;
  // Reveal the entire active input when it fits, otherwise follow its field.
  int top=entryTop(selected),height=inputHeight(selected),view=bodyBottom-24;
  if(height+8<=view){if(top<scrollY)scrollY=top;else if(top+height+5>scrollY+view)scrollY=top+height+5-view;}
  int baseline=entryTop(selected)+3+input.n[input.root].up;
  int y=baseline+input.cursorY(),up=7*input.n[input.row].scale,down=2*input.n[input.row].scale;
  if(y-up<scrollY+5)scrollY=y-up-5;
  if(y+down>scrollY+bodyBottom-29)scrollY=y+down-(bodyBottom-29);
  int cx=input.cursorX();if(cx-scrollX>340)scrollX=cx-340;if(cx-scrollX<4)scrollX=maxv(0,cx-4);clampScroll();
}
void Worksheet::navigate(int direction){
  measure();int view=bodyBottom-24;
  if(editing){
    int r=input.row,b=input.before;input.move(direction<0?UP:DOWN);
    if(r!=input.row||b!=input.before){followCursor=true;cursorVisible();return;}
    if((direction<0&&selected==0)||(direction>0&&selected==count))return;
    // At a field boundary browse the sheet without discarding an unfinished edit.
    suspendInput();
  }
  int top=entryTop(selected)+(resultFocus?inputHeight(selected):0);
  int height=resultFocus?resultHeight(selected):inputHeight(selected);
  // First traverse the selected input/result if it is taller than the viewport.
  if(direction<0&&scrollY>top){pan(0,-24);return;}
  if(direction>0&&top+height>scrollY+view){pan(0,24);return;}
  if(direction<0){
    if(resultFocus){resultFocus=false;scrollX=0;focus(false);return;}
    if(selected>0){select(selected-1,true);int end=entryTop(selected)+inputHeight(selected)+resultHeight(selected);scrollY=maxv(0,end-view);clampScroll();}
  }else{
    if(selected<count&&!resultFocus){resultFocus=true;scrollX=0;focus(true);return;}
    if(selected<count)select(selected+1,false);
  }
}
void Worksheet::pan(int dx,int dy){
  if(editing){
    int x=input.cursorX()+dx,y=input.cursorY()+dy;
    input.seekCursor(x,y);followCursor=true;cursorVisible();return;
  }
  followCursor=false;int width=selected<count?(resultFocus?entries[selected].resultWidth:entries[selected].inputWidth):input.n[input.root].w;
  if(!resultFocus&&selected==suspendedInput)width=input.n[input.root].w;
  scrollX=maxv(0,minv(scrollX+dx,maxv(0,width-344)));
  int top=entryTop(selected)+(resultFocus?inputHeight(selected):0),height=resultFocus?resultHeight(selected):inputHeight(selected);
  if(dy)scrollY=maxv(top,minv(scrollY+dy,maxv(top,top+height-(bodyBottom-24))));clampScroll();
}
bool Worksheet::deleteLast(){
  if(!count)return false;deletedEntry=entries[count-1];deleted=true;--count;
  suspendedInput=-1;input.clear();editing=false;undoReady=false;selected=count?count-1:0;resultFocus=count>0;scrollX=0;followCursor=false;
  if(count){measure();focus(true);int view=bodyBottom-24;if(inputHeight(selected)+resultHeight(selected)<=view){scrollY=maxv(0,entryTop(selected)+inputHeight(selected)+resultHeight(selected)-view);clampScroll();}}
  else newLine();return true;
}
bool Worksheet::restoreLast(){
  if(!deleted||count>=MAX_ENTRIES)return false;entries[count++]=deletedEntry;deleted=false;editing=false;selected=count-1;resultFocus=true;scrollX=0;measure();focus(true);return true;
}
void Worksheet::clearInput(){
  if(suspendedInput>=0){selected=suspendedInput;suspendedInput=-1;editing=true;}
  if(!editing){newLine();if(!editing)return;}
  snapshot();input.clear();message[0]=0;scrollX=0;resultFocus=false;panMode=false;followCursor=true;measure();focus(false);
}
int batteryBars(int v,int previous){
  // Approximate supply-voltage bands, not calibrated percentages of remaining charge.
  static const int thresholds[]={380,440,500,560};
  if(v<=0||v>=1000)return -1;
  int level=0;while(level<4&&v>=thresholds[level])++level;
  if(previous<0||previous>4)return level;
  level=previous;
  while(level<4&&v>=thresholds[level]+3)++level;
  while(level>0&&v<thresholds[level-1]-3)--level;
  return level;
}
void modifierIndicators(Surface &s,unsigned mode){
  s.clip(0,0,384,216);s.rect(28,3,23,17,INK);
  if(mode&1){s.rect(28,4,10,15,0xff08);s.text(30,8,"S",1,INK);}
  if(mode&12){s.rect(40,4,10,15,0xe986);s.text(42,8,"A",1,PAPER);if(mode&128)s.line(41,17,48,17,PAPER);}
}
void statusBar(Surface &s,const Status &st){
  s.clip(0,0,384,216);s.rect(0,0,384,22,INK);
  int level=st.batteryLevel>=0&&st.batteryLevel<=4?st.batteryLevel:batteryBars(st.battery);
  unsigned short batteryColor=level<=1&&level>=0?0xfb2c:level==2?0xff08:0x5f16;
  s.rect(4,5,20,12,level==0?batteryColor:0xc6fb);s.rect(5,6,18,10,INK);s.rect(24,8,2,6,0xc6fb);
  for(int i=0;i<level;++i)s.rect(7+i*4,8,3,6,batteryColor);
  if(level<0)s.text(11,8,"?",1,0xc6fb);
  if(level==0)s.line(8,13,19,8,batteryColor);
  modifierIndicators(s,st.keyMode);
  const int boxes[][2]={{53,23},{78,36},{116,29},{147,24}};
  for(int i=0;i<4;++i){s.rect(boxes[i][0],4,boxes[i][1],15,0x94f4);s.rect(boxes[i][0]+1,5,boxes[i][1]-2,13,INK);}
  s.text(56,8,st.radians?"RAD":"DEG",1,PAPER);
  s.text(81,8,st.decimal?"DEC":"EXACT",1,0xc6fb);
  s.text(119,8,st.complex==2?"r\x1b\x1c":st.complex?"A+Bi":"REAL",1,PAPER);
  char digits[8];sprintf(digits,"%dD",st.digits);s.text(150,8,digits,1,0xc6fb);
  if(st.section&&st.section[0]){char title[15];strncpy(title,st.section,14);title[14]=0;
    if(strlen(st.section)>14){title[12]='.';title[13]='.';}s.text(174,8,title,1,0xc6fb);}
  if(st.time)s.text(262,8,st.time,1,PAPER);
  s.line(332,4,332,18,0xc6fb);
  s.rect(338,4,43,15,ACCENT);s.label(340,6,39,11,"NCAS",PAPER);
}
void sheetScreen(Surface &s,Worksheet &w,const Status &st,int noticeOffset){
  s.clip(0,0,384,216);s.rect(0,0,384,216,PAPER);statusBar(s,st);
  const char *notice=w.message[0]?w.message:w.input.error;
  w.measure();w.cursorVisible();s.clip(0,24,376,w.bodyBottom-24);
  for(int i=0;i<w.count+(w.selected==w.count||w.suspendedInput==w.count?1:0);++i){
    int y=24+w.entryTop(i)-w.scrollY,ih=w.inputHeight(i),rh=w.resultHeight(i);
    if(y+ih+rh<24||y>w.bodyBottom)continue;
    bool active=i==w.selected;
    if(active){int yy=y+(w.resultFocus&&!w.editing?ih:0),hh=w.resultFocus&&!w.editing?rh:ih;
      s.rect(2,yy+3,3,maxv(3,hh-6),ACCENT);
      if(w.panMode)s.text(7,yy+4,"+",1,ACCENT);
    }
    Editor *e=&w.scratch;
    if((active&&w.editing)||i==w.suspendedInput)e=&w.input;
    else if(i<w.count)e->unpack(w.entries[i].form,w.entries[i].formSize);
    else e->clear();
    if(y+ih>=24&&y<w.bodyBottom){e->layout(w.scale);e->paint(s,14-(active?w.scrollX:0),y+3+e->n[e->root].up,active&&w.editing);}
    if(i<w.count&&y+ih+rh>=24&&y+ih<w.bodyBottom){
      Entry &entry=w.entries[i];
      if(entry.state==PENDING)s.text(338,y+ih+3,"...",1,MUTED);
      else if(entry.state==FAILED)s.text(324,y+ih+3,"Error",1,ERROR);
      else {w.scratch.load(entry.result);w.scratch.layout(w.scale);int width=w.scratch.n[w.scratch.root].w;
        int x=width<342?360-width:16;w.scratch.paint(s,x-(active?w.scrollX:0),y+ih+3+w.scratch.n[w.scratch.root].up,false);}
    }
    s.line(12,y+ih+rh+4,366,y+ih+rh+4,0xef7d);
  }
  s.clip(0,0,384,216);
  int view=w.bodyBottom-24;
  if(w.totalHeight>view){int thumb=maxv(10,view*view/w.totalHeight);int y=24+(view-thumb)*w.scrollY/maxv(1,w.totalHeight-view);
    s.rect(379,24,3,view,0xef7d);s.rect(379,y,3,thumb,ACCENT);}
  if(w.scrollX)s.text(368,28,"<",1,ACCENT);
  int width=w.editing||(!w.resultFocus&&w.selected==w.suspendedInput)?w.input.n[w.input.root].w:w.selected<w.count?(w.resultFocus?w.entries[w.selected].resultWidth:w.entries[w.selected].inputWidth):0;
  if(width>344){int thumb=maxv(12,344*344/width);int x=14+(344-thumb)*minv(w.scrollX,width-344)/(width-344);
    s.rect(14,w.bodyBottom-3,344,2,0xef7d);s.rect(x,w.bodyBottom-3,thumb,2,ACCENT);}
  if(notice[0]){int length=strlen(notice),width=minv(330,maxv(144,minv(length,52)*6+16));
    int columns=(width-16)/6,height=length>columns?32:22,x=375-width+noticeOffset;
    s.clip(0,24,376,w.bodyBottom-24);s.rect(x+2,28,width,height,0xe71c);s.rect(x,26,width,height,ERROR);
    char line[54];for(int i=0;i<2;++i){int offset=i*columns;if(length<=offset)break;strncpy(line,notice+offset,columns);line[columns]=0;s.text(x+8,33+i*11,line,1,PAPER);}}
}
void splash(Surface &s){
  s.clip(0,0,384,216);s.rect(0,0,384,216,PAPER);
  s.text(30,36,"NCAS",4,INK);s.rect(30,70,66,2,ACCENT);
  s.text(31,89,"Made by",1,MUTED);
  s.label(30,105,210,19,"Nazar Heldyiev",INK);s.label(31,105,210,19,"Nazar Heldyiev",INK);
  int x=265,y=48;
  s.rect(x,y,4,90,INK);s.rect(x,y,16,4,INK);s.rect(x,y+86,16,4,INK);
  s.rect(x+82,y,4,90,INK);s.rect(x+70,y,16,4,INK);s.rect(x+70,y+86,16,4,INK);
  for(int k=-1;k<=1;++k){s.line(x+43+k,y+22,x+43+k,y+45,ACCENT);s.line(x+20,y+45+k,x+43,y+45+k,ACCENT);s.line(x+43+k,y+45,x+66+k,y+68,ACCENT);}
  for(int r=0;r<3;++r)for(int c=0;c<3;++c){bool accent=(r==0&&c==1)||(r==1&&c<2)||(r==2&&c==2);s.rect(x+15+23*c,y+17+23*r,10,10,accent?ACCENT:INK);}
  s.text(31,148,"Powered by KhiCAS / Giac",1,INK);
  s.text(31,161,"Includes MicroPython",1,MUTED);
}
bool replay(Worksheet &w,ReplayEngine &engine,int changedFrom,bool rebuild){
  // Restore the state immediately before the changed row; never run its prefix.
  for(int i=changedFrom;i<w.count;++i){w.entries[i].state=PENDING;w.entries[i].result[0]=0;w.updateEntry(i);}
  if(rebuild&&!engine.restoreBefore(changedFrom)){strcpy(w.message,"State unavailable. Restore the preceding calculation first.");return false;}w.editing=false;w.panMode=false;w.scrollX=0;
  for(int i=changedFrom;i<w.count;++i){
    Entry &e=w.entries[i];w.selected=i;w.resultFocus=false;w.measure();w.focus(false);engine.progress(w,i,false);
    bool ok=engine.evaluate(i,e.source,e.result,sizeof(e.result));e.result[sizeof(e.result)-1]=0;
    e.state=ok?VALID:FAILED;w.updateEntry(i);w.measure();w.focus(true);engine.progress(w,i,true);
    if(!ok){for(int j=i+1;j<w.count;++j){w.entries[j].state=PENDING;w.entries[j].result[0]=0;w.updateEntry(j);}
      strcpy(w.message,"Calculation stopped. Edit this line to continue.");return false;}
  }
  return true;
}
}
