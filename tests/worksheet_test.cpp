// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/worksheet.h"
#include "../src/toolbar.h"
#include "../src/settings.h"
#include "../src/symbols.h"
#include "../src/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace natural;
static int checks=0;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void image(const char *name,unsigned short *pixels){FILE *f=fopen(name,"wb");CHECK(f);fprintf(f,"P6\n384 216\n255\n");for(int i=0;i<384*216;++i){unsigned c=pixels[i];unsigned char rgb[3]={(unsigned char)(((c>>11)&31)*255/31),(unsigned char)(((c>>5)&63)*255/63),(unsigned char)((c&31)*255/31)};fwrite(rgb,1,3,f);}fclose(f);}
static void example(Worksheet &w,const char *in,const char *out,bool literal=false){w.newLine();if(literal){if(!strncmp(in,"ans()",5)){w.input.insert("ans()");w.input.insert(in+5);}else w.input.insert(in);}else w.input.load(in);CHECK(w.commit());Entry &e=w.entries[w.selected];strcpy(e.result,out);e.state=VALID;w.updateEntry(w.selected);w.measure();w.focus(true);}
struct MockEngine:ReplayEngine {
  int savedA[32],savedAns[32];
  int a,ans,resetCount,evalCount,events[100],eventCount;bool fail;
  MockEngine():a(0),ans(0),resetCount(0),evalCount(0),eventCount(0),fail(false){}
  void reset(){a=ans=0;++resetCount;eventCount=0;}
  bool restoreBefore(int i){if(!i)reset();else {a=savedA[i-1];ans=savedAns[i-1];eventCount=0;}return true;}
  bool evaluate(int i,const char *s,char *out,int cap){++evalCount;
    if(fail&&i==1){strcpy(out,"Undefined");return false;}
    if(!strncmp(s,"a:=",3))ans=a=atoi(s+3);else if(!strcmp(s,"a*3"))ans=a*3;else if(!strcmp(s,"ans()+a"))ans+=a;else ans=atoi(s);
    savedA[i]=a;savedAns[i]=ans;sprintf(out,"%d",ans);return true;}
  void progress(Worksheet &w,int i,bool done){if(done){events[eventCount++]=i;CHECK(w.selected==i);CHECK(w.resultFocus);CHECK(w.scrollY>=0);}}
};
int main(int argc,char **argv){
  Worksheet *ptr=new Worksheet;Worksheet &w=*ptr;Editor restored;unsigned char form[MAX_FORM];char src[MAX_SOURCE];
  w.input.insert("(5/5");CHECK(!w.input.serialize(src,sizeof(src)));CHECK(strstr(w.input.error,"Close"));
  int size=w.input.pack(form,sizeof(form));CHECK(size>0);CHECK(restored.unpack(form,size));CHECK(restored.serialize(src,sizeof(src),false));CHECK(!strcmp(src,"(5/5"));
  w.input.insert(")");CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"(5/5)"));
  CHECK(w.commit());w.edit(true);CHECK(w.input.n[w.input.n[w.input.root].first].kind==TEXT);CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"(5/5)"));
  // A full editor row (>255 children) and structural forms round-trip losslessly.
  restored.clear();for(int i=0;i<400;++i)restored.insert("1");size=restored.pack(form,sizeof(form));CHECK(size>0);CHECK(w.scratch.unpack(form,size));CHECK(w.scratch.childCount(w.scratch.root)==400);
  w.input.clear();w.input.fraction();w.input.insert("1");w.input.move(DOWN);w.input.insert("2");size=w.input.pack(form,sizeof(form));CHECK(restored.unpack(form,size));CHECK(restored.n[restored.n[restored.root].first].kind==FRACTION);
  CHECK(!restored.unpack(form,size-1));CHECK(restored.validate());
  // Editing line zero recomputes assignments and dependent Ans in chronological order.
  w.clear();example(w,"a:=2","2",true);example(w,"a*3","6",true);example(w,"ans()+a","8",true);
  CHECK(w.select(0));w.edit();w.input.clear();w.input.insert("a:=4");CHECK(w.commit());
  MockEngine engine;CHECK(replay(w,engine,0));CHECK(engine.resetCount==1);CHECK(engine.evalCount==3);CHECK(engine.eventCount==3);
  CHECK(!strcmp(w.entries[0].result,"4"));CHECK(!strcmp(w.entries[1].result,"12"));CHECK(!strcmp(w.entries[2].result,"16"));CHECK(w.selected==2);
  for(int i=0;i<3;++i)CHECK(engine.events[i]==i);
  // Append evaluates only the new input; editing a previous input restores the baseline.
  w.newLine();w.input.insert("ans()+a");CHECK(w.commit());CHECK(replay(w,engine,3,false));CHECK(engine.resetCount==1);CHECK(engine.evalCount==4);CHECK(!strcmp(w.entries[3].result,"20"));
  engine.fail=true;CHECK(!replay(w,engine,0));CHECK(w.entries[1].state==FAILED);CHECK(w.entries[2].state==PENDING);CHECK(!w.entries[2].result[0]);CHECK(w.selected==1);engine.fail=false;
  // A middle edit never evaluates its earlier prefix, including random/stateful rows.
  CHECK(replay(w,engine,0));int beforeCount=engine.evalCount;
  CHECK(w.select(2));w.edit();w.input.clear();w.input.insert("ans()+a");CHECK(w.commit());CHECK(replay(w,engine,2));
  CHECK(engine.evalCount==beforeCount+2);CHECK(engine.events[0]==2);CHECK(engine.events[1]==3);
  // Browse without a separate history view, then enter editing from either edge.
  CHECK(replay(w,engine,0));w.navigate(-1);CHECK(w.selected==3&&!w.resultFocus);w.navigate(-1);CHECK(w.selected==2&&w.resultFocus);
  w.edit(false);CHECK(w.editing);CHECK(w.input.before==w.input.n[w.input.root].first);
  w.editing=false;w.edit(true);CHECK(!w.input.before);
  // Large matrices are never scaled to squeeze both input and output into one row.
  w.clear();w.input.matrix(6,6);int m=w.input.activeMatrix();
  for(int i=0;i<36;++i){w.input.row=w.input.child(m,i);w.input.before=0;w.input.insert("12345");w.input.fraction();w.input.insert("6789");}
  CHECK(w.commit());strcpy(w.entries[0].result,"[[1,2],[3,4]]");w.entries[0].state=VALID;w.updateEntry(0);w.measure();w.focus(true);
  CHECK(w.entries[0].inputUp+w.entries[0].inputDown>168);CHECK(w.entries[0].inputWidth>344);CHECK(w.scrollY>w.inputHeight(0)-168);
  int resultScroll=w.scrollY;w.navigate(-1);CHECK(!w.resultFocus);CHECK(w.scrollY<resultScroll);CHECK(w.scrollY==0);
  w.navigate(1);CHECK(w.scrollY>0);CHECK(!w.resultFocus);w.pan(5000,0);CHECK(w.scrollX==w.entries[0].inputWidth-344);
  w.edit();w.input.row=w.input.child(m,35);w.input.before=0;w.input.layout();w.followCursor=true;w.cursorVisible();CHECK(w.scrollX>0);CHECK(w.scrollY>0);
  // Toolbar navigation retains ancestors and exposes all 71 operations.
  Toolbar toolbar;Status st;toolbar.build(st);CHECK(toolbar.actions[3]==MATH);toolbar.open(MATH);toolbar.page=1;toolbar.open(CATEGORY);toolbar.back();CHECK(toolbar.menu==MATH&&toolbar.page==1);
  bool found[100]={0};for(int c=0;c<11;++c){toolbar.home();toolbar.open(CATEGORY+c);for(int page=0;page<5;++page){toolbar.page=page;toolbar.build(st);for(int k=0;k<6;++k)if(toolbar.actions[k]>=OPERATION)found[toolbar.actions[k]-OPERATION]=true;}}
  for(int i=0;i<operationCount;++i)CHECK(found[i]);
  // Root pagination never enters a branch; all toolbar heights are identical.
  toolbar.home();toolbar.build(st);CHECK(toolbar.height()==24);CHECK(toolbar.actions[5]==PAGE);
  toolbar.page++;toolbar.build(st);CHECK(toolbar.menu==HOME&&toolbar.depth==0&&!toolbar.trail[0]);CHECK(toolbar.actions[0]==SETTINGS);
  toolbar.page++;toolbar.build(st);CHECK(toolbar.page==0&&!toolbar.trail[0]);CHECK(toolbar.actions[0]==JUMP);
  for(int n=0;n<20;++n){int menus[]={JUMP,EDIT,MAT,MATH,CALC,RESIZE,SYMBOLS,CONFIRM_CLEAR,CONFIRM_DELETE};int c=n<9?menus[n]:CATEGORY+n-9;toolbar.home();toolbar.open(c);toolbar.build(st);CHECK(toolbar.height()==24);for(int k=0;k<6;++k){CHECK(strlen(toolbar.labels[k])<=7);CHECK(!strchr(toolbar.labels[k],'\n'));}}
  // AC clears all nested structures at once and undo restores the whole edit.
  w.clear();w.input.insert("7+");w.input.matrix(2,2);w.input.fraction();w.input.insert("9");
  size=w.input.pack(form,sizeof(form));w.clearInput();CHECK(!w.input.n[w.input.root].first);CHECK(w.input.row==w.input.root);CHECK(!w.scrollX&&!w.panMode);
  w.undoEdit();CHECK(w.input.activeMatrix()!=0);CHECK(w.input.pack(form,sizeof(form))==size);
  w.clear();example(w,"1+2","3",true);w.clearInput();CHECK(w.count==1&&w.selected==1&&w.editing);CHECK(!w.input.n[w.input.root].first);CHECK(!strcmp(w.entries[0].source,"1+2"));
  example(w,"4+5","9",true);w.select(0);w.edit();w.input.fraction();w.input.insert("3");w.clearInput();CHECK(w.selected==0);CHECK(!w.input.n[w.input.root].first);w.navigate(1);CHECK(!w.editing&&w.resultFocus);
  // Real calendar validity and BCD transport, including leap-day boundaries.
  DateTime d;d.year=2024;d.month=2;d.day=29;d.hour=0;d.minute=7;char time[12];unsigned char bcd[7];
  CHECK(d.valid());d.format(time);CHECK(!strcmp(time,"29/02 00:07"));CHECK(d.encode(bcd));CHECK(bcd[0]==0x20&&bcd[1]==0x24&&bcd[3]==0x29&&bcd[5]==0x07);
  DateTime decoded;CHECK(decoded.decode(bcd));CHECK(decoded.day==29&&decoded.year==2024);
  d.year=2025;CHECK(!d.valid());CHECK(!d.encode(bcd));d.format(time);CHECK(!strcmp(time,"--/-- --:--"));
  bcd[2]=0x1a;CHECK(!decoded.decode(bcd));
  d.year=2026;d.month=9;d.day=9;d.hour=16;d.minute=38;CHECK(d.valid());d.format(time);CHECK(!strcmp(time,"09/09 16:38"));
  d.month=4;d.day=31;CHECK(!d.valid());d.day=30;d.hour=24;CHECK(!d.valid());d.hour=23;d.minute=60;CHECK(!d.valid());d.minute=59;
  ClockForm f(d);f.selected=1;f.step(-1);CHECK(f.value.month==3);f.value.day=31;f.step(1);CHECK(f.value.day==30);
  f.selected=2;f.digit(2);f.digit(0);f.digit(2);f.digit(8);CHECK(f.value.year==2028);f.move(1);f.digit(0);f.digit(9);CHECK(f.value.hour==9);f.erase();CHECK(f.value.hour==0);f.step(-1);CHECK(f.value.hour==23);

  // Battery display uses measured voltage, bounded bands and hysteresis.
  CHECK(batteryBars(-1)==-1);CHECK(batteryBars(0)==-1);CHECK(batteryBars(1000)==-1);
  CHECK(batteryBars(365)==0);CHECK(batteryBars(410)==1);CHECK(batteryBars(470)==2);CHECK(batteryBars(530)==3);CHECK(batteryBars(620)==4);
  CHECK(batteryBars(561,3)==3);CHECK(batteryBars(563,3)==4);CHECK(batteryBars(558,4)==4);CHECK(batteryBars(556,4)==3);
  CHECK(batteryBars(370,4)==0);CHECK(batteryBars(620,0)==4);
  // The symbolic label retains the same command and distinct derivative/integral variants.
  CHECK(toolbarSymbol(FRACTION_KEY)==FRACTION_SYMBOL);CHECK(toolbarSymbol(POWER_KEY)==POWER_SYMBOL);CHECK(toolbarSymbol(ROOT_KEY)==ROOT_SYMBOL);
  CHECK(toolbarSymbol(OPERATION+14)==DERIVATIVE_SYMBOL);CHECK(toolbarSymbol(OPERATION+15)==NTH_DERIVATIVE_SYMBOL);
  CHECK(toolbarSymbol(OPERATION+16)==INTEGRAL_SYMBOL);CHECK(toolbarSymbol(OPERATION+17)==DEFINITE_INTEGRAL_SYMBOL);
  CHECK(toolbarSymbol(OPERATION)==NO_SYMBOL);CHECK(toolbarSymbol(OPERATION+operationCount)==NO_SYMBOL);
  // A draft at capacity keeps all 32 rows until the next successful commit.
  w.clear();for(int i=0;i<MAX_ENTRIES;++i)example(w,"1","1");w.newLine();CHECK(w.count==MAX_ENTRIES);CHECK(w.selected==MAX_ENTRIES&&w.editing);w.clearInput();CHECK(w.editing&&w.count==MAX_ENTRIES);CHECK(!w.input.n[w.input.root].first);
  // Continuous in-memory retention and undo deletion.
  w.clear();for(int i=0;i<40;++i){char expr[12];sprintf(expr,"%d",i);example(w,expr,expr,true);}
  CHECK(w.count==32);CHECK(!strcmp(w.entries[0].source,"8"));CHECK(!strcmp(w.entries[31].source,"39"));
  CHECK(w.deleteLast());CHECK(w.count==31);CHECK(w.restoreLast());CHECK(!strcmp(w.entries[31].source,"39"));
  unsigned char *saved=new unsigned char[MAX_SAVED_RAW],*compressed=new unsigned char[MAX_SAVED_RAW],*unpacked=new unsigned char[MAX_SAVED_RAW];
  Memory writer(saved,MAX_SAVED_RAW);CHECK(worksheetMemory(writer,w));unsigned compressedSize=compressMemory(saved,writer.size,compressed,MAX_SAVED_RAW);CHECK(compressedSize>0&&compressedSize<writer.size);CHECK(decompressMemory(compressed,compressedSize,unpacked,writer.size));CHECK(!memcmp(saved,unpacked,writer.size));CHECK(!decompressMemory(compressed,compressedSize-1,unpacked,writer.size));
  Worksheet *copy=new Worksheet;Memory reader(saved,writer.size,true);CHECK(worksheetMemory(reader,*copy));CHECK(reader.hash==writer.hash);CHECK(copy->count==32);CHECK(!strcmp(copy->entries[31].result,"39"));
  Memory truncated(saved,writer.size-1,true);CHECK(!worksheetMemory(truncated,*copy));CHECK(copy->count==0);
  // An incomplete edited fraction and its selected field survive a memory round trip.
  w.newLine();w.input.fraction();w.input.insert("5");w.input.move(DOWN);Memory draft(saved,MAX_SAVED_RAW);CHECK(worksheetMemory(draft,w));Memory draftRead(saved,draft.size,true);CHECK(worksheetMemory(draftRead,*copy));CHECK(copy->editing&&copy->selected==32);CHECK(copy->input.n[copy->input.n[copy->input.row].parent].kind==FRACTION);CHECK(!copy->input.n[copy->input.row].first);
  // Codec stress includes incompressible bytes, overlapping copies, tiny buffers and invalid references.
  unsigned random=193;for(int trial=0;trial<80;++trial){unsigned length=1+trial*997;for(unsigned i=0;i<length;++i){random=random*1664525+1013904223;saved[i]=trial%3?random>>24:i%17;}unsigned n=compressMemory(saved,length,compressed,MAX_SAVED_RAW);CHECK(n);CHECK(decompressMemory(compressed,n,unpacked,length));CHECK(!memcmp(saved,unpacked,length));CHECK(!compressMemory(saved,length,compressed,1));}
  compressed[0]=128;compressed[1]=0;CHECK(!decompressMemory(compressed,2,unpacked,3));delete[]saved;delete[]compressed;delete[]unpacked;delete copy;
  // New tall drafts follow the complete input if it fits, then the active cell.
  w.newLine();w.input.matrix(3,3);w.measure();w.cursorVisible();CHECK(w.entryTop(w.selected)>=w.scrollY);CHECK(w.entryTop(w.selected)+w.inputHeight(w.selected)<=w.scrollY+w.bodyBottom-24);
  // DOWN leaves a recalled input for its answer; repeated browsing never traps the selection.
  w.clear();w.navigate(-1);CHECK(w.editing&&w.selected==0);w.navigate(1);CHECK(w.editing&&w.selected==0);
  w.input.insert("2*x");w.navigate(-1);CHECK(w.editing&&w.suspendedInput<0);w.navigate(1);CHECK(w.editing&&w.suspendedInput<0);
  CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"2*x"));
  w.clear();example(w,"2*x","2*x");example(w,"5+5","10",true);
  for(int i=0;i<20;++i){w.select(1,true);w.navigate(-1);CHECK(!w.resultFocus);w.edit();CHECK(w.editing);w.navigate(1);CHECK(!w.editing&&w.resultFocus&&w.selected==1);w.navigate(-1);w.navigate(-1);CHECK(w.selected==0&&w.resultFocus);}
  // A changed input can be suspended to inspect its old answer, then resumed without losing edits.
  w.select(1);w.edit();w.input.insert("+1");w.navigate(1);CHECK(!w.editing&&w.resultFocus&&w.suspendedInput==1);CHECK(!strcmp(w.entries[1].result,"10"));
  w.select(0,true);CHECK(w.suspendedInput==1);w.edit();CHECK(!w.editing);w.select(1);w.edit();CHECK(w.editing&&w.suspendedInput<0);CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"5+5+1"));
  w.navigate(1);unsigned char *paused=new unsigned char[MAX_SAVED_RAW];Memory pausedWrite(paused,MAX_SAVED_RAW);CHECK(worksheetMemory(pausedWrite,w));Worksheet *again=new Worksheet;Memory pausedRead(paused,pausedWrite.size,true);CHECK(worksheetMemory(pausedRead,*again));CHECK(again->editing&&again->selected==1);CHECK(again->input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"5+5+1"));delete again;delete[]paused;
  CHECK(!w.newLine());CHECK(w.suspendedInput==1);CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"5+5+1"));
  w.clearInput();CHECK(w.editing&&w.selected==1&&!w.input.n[w.input.root].first);w.navigate(1);CHECK(w.resultFocus);
  // A pending new input can be left and returned to, even with 32 retained entries.
  w.clear();for(int i=0;i<32;++i)example(w,"1","1",true);w.newLine();w.input.insert("7+8");w.navigate(-1);CHECK(w.selected==31&&w.resultFocus&&w.suspendedInput==32);w.navigate(1);CHECK(w.selected==32&&w.editing&&w.suspendedInput<0);CHECK(w.input.serialize(src,sizeof(src)));CHECK(!strcmp(src,"7+8"));
  // Panning in the editor relocates the caret, keeping it inside the visible area.
  w.clear();w.input.matrix(6,6);m=w.input.activeMatrix();for(int i=0;i<36;++i){w.input.row=w.input.child(m,i);w.input.insert("1234567");}w.input.row=w.input.child(m,0);w.input.before=0;w.measure();w.cursorVisible();int oldCursor=w.input.cursorY();w.pan(0,40);CHECK(w.input.cursorY()>oldCursor);CHECK(w.followCursor);
  for(int i=0;i<20;++i){w.pan(40,24);int cy=24+w.entryTop(w.selected)+3+w.input.n[w.input.root].up+w.input.cursorY()-w.scrollY;CHECK(cy-7*w.input.n[w.input.row].scale>=24);CHECK(cy+2*w.input.n[w.input.row].scale<=w.bodyBottom);CHECK(w.input.cursorX()-w.scrollX<=344);}
  // Notification animation is a paint overlay: every pixel outside its rectangle is stable.
  unsigned short *plain=new unsigned short[384*216],*overlay=new unsigned short[384*216];Surface baselineScreen(plain),overlayScreen(overlay);w.clear();example(w,"2*x+3","2*x+3");sheetScreen(baselineScreen,w,st);int oldScroll=w.scrollY,oldHeight=w.totalHeight,oldBottom=w.bodyBottom;strcpy(w.message,"Check this expression.");
  for(int offset=0;offset<=330;offset+=55){sheetScreen(overlayScreen,w,st,offset);CHECK(w.scrollY==oldScroll&&w.totalHeight==oldHeight&&w.bodyBottom==oldBottom);for(int y=0;y<216;++y)for(int x=0;x<384;++x)if(x<45+offset||x>=376||y<26||y>=60)if(plain[y*384+x]!=overlay[y*384+x])CHECK(false);}
  sheetScreen(overlayScreen,w,st,330);for(unsigned mode=1;mode<=0x84;mode=mode==1?4:mode==4?0x84:0x85){modifierIndicators(overlayScreen,mode);CHECK(overlay[8*384+28]==(mode==1?0xff08:INK));CHECK(overlay[8*384+40]==(mode&12?0xe986:INK));}
  delete[]plain;delete[]overlay;
  printf("PASS: %d worksheet/replay/toolbar/memory checks; Worksheet %zu bytes\n",checks,sizeof(Worksheet));
  if(argc>1){unsigned short *pixels=new unsigned short[384*216];Surface s(pixels);st=Status();st.time="09/09 16:38";st.battery=560;
    #define SCREEN(name) do{w.bodyBottom=216-toolbar.height();toolbar.build(st);st.section=toolbar.trail;sheetScreen(s,w,st);toolbar.paint(s);image(name ".ppm",pixels);}while(0)
    toolbar.home();w.clear();example(w,"5/5","1",true);example(w,"ans()/5","1/5",true);example(w,"5+5","10",true);w.newLine();w.reflow(1);SCREEN("worksheet");toolbar.page=1;SCREEN("main-page2");toolbar.page=0;
    w.clear();w.reflow(2);example(w,"1/2+1/6","2/3");SCREEN("fractions");toolbar.open(MATH);SCREEN("functions");toolbar.open(CATEGORY);SCREEN("algebra-toolbar");
    toolbar.home();w.clear();w.input.matrix(2,2);m=w.input.activeMatrix();for(int i=0;i<4;++i){w.input.row=w.input.child(m,i);char c[2]={(char)('1'+i),0};w.input.insert(c);}w.input.row=w.input.child(m,2);w.input.before=0;toolbar.open(MAT);SCREEN("matrix");toolbar.open(RESIZE);SCREEN("matrix-resize");
    toolbar.home();w.clear();example(w,"diff(x^3-3*x,x)","3*x^2-3");st.radians=true;SCREEN("calculus");
    w.clear();example(w,"det([[1,2],[3,4]])","-2");SCREEN("determinant");
    w.clear();example(w,"sin(30)","1/2",true);st.radians=false;SCREEN("degrees");settingsScreen(s,st,0,false);image("settings.ppm",pixels);st.radians=st.complex=true;settingsScreen(s,st,1,false);image("settings-radians.ppm",pixels);DateTime clock;clock.year=2026;clock.month=9;clock.day=9;clock.hour=16;clock.minute=38;ClockForm cf(clock);clockScreen(s,st,cf,true);image("clock-setup.ppm",pixels);
    toolbar.home();w.clear();example(w,"integrate(x^2,x,0,3)","9");SCREEN("integral");
    w.clear();w.input.fraction();w.input.insert("1");w.input.move(DOWN);SCREEN("fraction-edit");
    w.clear();example(w,"5*5/2","25/2",true);SCREEN("operators");
    w.clear();w.input.insert("sin(30");SCREEN("manual-bracket");
    w.clear();example(w,"a:=4","4",true);example(w,"a*3","12",true);example(w,"ans()+a","16",true);SCREEN("replay");w.select(0);w.edit();SCREEN("edit-history");
    w.clear();w.input.matrix(6,6);m=w.input.activeMatrix();for(int i=0;i<36;++i){w.input.row=w.input.child(m,i);w.input.before=0;w.input.insert("1");w.input.fraction();w.input.insert("2");}CHECK(w.commit());strcpy(w.entries[0].result,"[[1/2,1/2,1/2,1/2,1/2,1/2],[1/2,1/2,1/2,1/2,1/2,1/2],[1/2,1/2,1/2,1/2,1/2,1/2],[1/2,1/2,1/2,1/2,1/2,1/2],[1/2,1/2,1/2,1/2,1/2,1/2],[1/2,1/2,1/2,1/2,1/2,1/2]]");w.entries[0].state=VALID;w.updateEntry(0);w.measure();w.focus(true);SCREEN("large-result");w.navigate(-1);SCREEN("large-input");
    toolbar.home();w.clear();example(w,"diff(x^3-3*x,x)","3*x^2-3");toolbar.open(CALC);toolbar.open(CATEGORY+2);SCREEN("calculus-symbols");toolbar.page=1;SCREEN("sum-symbols");
    toolbar.home();toolbar.open(MAT);toolbar.open(CATEGORY+3);SCREEN("matrix-symbols");
    toolbar.home();toolbar.open(MATH);toolbar.open(CATEGORY+5);SCREEN("complex-symbols");
    toolbar.home();w.clear();w.input.insert("1+2");SCREEN("header-full");workspacePromptScreen(s,false);image("khicas-confirm.ppm",pixels);
    toolbar.home();st.battery=410;SCREEN("battery-low");st.battery=470;SCREEN("battery-half");st.battery=620;SCREEN("battery-full");
    st.battery=365;SCREEN("battery-empty");st.battery=-1;SCREEN("battery-unknown");
    // Every symbol is also exported separately in an atlas for visual review.
    for(int group=0;group<5;++group){s.clip(0,0,384,216);s.rect(0,0,384,216,PAPER);
      for(int i=0;i<6;++i){int k=group*6+i+1;if(k>PLOT_SYMBOL)break;int x=i*64;s.rect(x+1,26,62,24,INK);paintSymbol(s,(MathSymbol)k,x+3,29,PAPER);char label[8];sprintf(label,"%d",k);s.text(x+25,12,label,1,INK);}
      char name[32];sprintf(name,"symbols-%d.ppm",group);image(name,pixels);
    }
    toolbar.home();st.battery=520;st.radians=false;st.complex=2;w.clear();example(w,"1+i","ncas_polar(sqrt(2),45)");SCREEN("polar-result");settingsScreen(s,st,1,false);image("settings-polar.ppm",pixels);st.complex=0;
    w.clear();w.input.exponential();SCREEN("exp-entry");w.input.insert("x+1");w.input.leave();SCREEN("exp-natural");
    w.clear();w.input.radical(true);w.input.insert("8");w.input.move(UP);SCREEN("indexed-root");
    toolbar.home();toolbar.open(MAT);w.clear();w.input.matrix(3,1);SCREEN("matrix-presets");toolbar.page=1;SCREEN("matrix-more");dimensionScreen(s,3,2,0,"");image("matrix-dimensions.ppm",pixels);
    toolbar.home();toolbar.open(CATEGORY+2);toolbar.page=1;w.clear();w.input.call("sum",4);int sum=w.input.n[w.input.row].parent;w.input.setArgument(sum,0,"k^2");w.input.setArgument(sum,1,"k");w.input.setArgument(sum,2,"1");w.input.setArgument(sum,3,"n");SCREEN("sigma-natural");
    w.clear();w.input.load("product(k,k,1,5)");SCREEN("product-natural");w.clear();w.input.load("limit(sin(x)/x,x=0)");SCREEN("limit-natural");w.clear();w.input.load("diff(x^4,x,3)");SCREEN("nth-diff-natural");
    toolbar.home();w.clear();example(w,"1/sqrt(2)","sqrt(2)/2");SCREEN("rationalised");w.newLine();w.input.insert("sin(30");strcpy(w.input.error,"Close the open bracket before calculating.");SCREEN("error-toast");
    toolbar.home();w.clear();example(w,"2*x+3*x^2","3*x^2+2*x");SCREEN("coefficients");st.keyMode=1;SCREEN("header-shift");st.keyMode=4;SCREEN("header-alpha");st.keyMode=0x84;SCREEN("header-alpha-lock");st.keyMode=0;strcpy(w.message,"Check this expression.");SCREEN("notice-overlay");sheetScreen(s,w,st,165);toolbar.paint(s);image("notice-slide.ppm",pixels);
    splash(s);image("startup.ppm",pixels);delete[] pixels;
  }
  delete ptr;
}
