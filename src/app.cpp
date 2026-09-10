// NCAS - Copyright (C) 2026. SPDX-License-Identifier: GPL-3.0-or-later
// Upstream rtc.h omits C linkage; declare the syscall before that header is seen.
extern "C" void RTC_GetTime(unsigned int *,unsigned int *,unsigned int *,unsigned int *);
extern "C" void RTC_SetDateTime(unsigned char *);
extern "C" int EnableDisplayHeader(int,int);
#include "giacPCH.h"
#include "main.h"
#include "console.h"
#include "catalogGUI.hpp"
#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/file.h>
#include <fxcg/system.h>
#include "worksheet.h"
#include "toolbar.h"
#include "settings.h"
#include "memory.h"
#include "sym2poly.h"
#undef printf
namespace natural {
struct Preferences {
  unsigned char version,radians,decimal,digits,complex,compact,reserved,check;
  Preferences():version(1),radians(0),decimal(0),digits(10),complex(0),compact(0),reserved(0),check(11){}
};
struct App {
  Worksheet sheet;Toolbar toolbar;Preferences settings;
  giac::context *baseline,*after[MAX_ENTRIES];
  unsigned savedHash,savedSize;
  giac::gen exact[MAX_ENTRIES],parsed[MAX_ENTRIES];
  int operation,operationNode,computedCount,battery,batteryStamp,batteryLevel;
  bool inSync,animate;
  bool decimalShown[MAX_ENTRIES];
  App():baseline(0),savedHash(0),savedSize(0),operation(-1),operationNode(0),computedCount(0),battery(-1),batteryStamp(-100),batteryLevel(-1),inSync(true),animate(false){memset(decimalShown,0,sizeof(decimalShown));memset(after,0,sizeof(after));}
};
static App *state=0;
static int originalHeader=-1;
static void nativeHeader(bool original){
  if(originalHeader<0)originalHeader=EnableDisplayHeader(1,0);
  EnableDisplayHeader(original?2:0,original?originalHeader:0);
  // 0 enables the OS status area; 3 disables it (not a boolean API).
  EnableStatusArea(original?0:3);
  if(!original)Cursor_SetFlashOff();
}
static void ncasGetKey(Surface &s,int *key){
  for(;;){
    nativeHeader(false);GetKey(key);nativeHeader(false);
    modifierIndicators(s,GetSetupSetting(0x14));Bdisp_PutDisp_DD();
    if(*key!=KEY_CTRL_SHIFT&&*key!=KEY_CTRL_ALPHA)return;
  }
}
static void applySettings(App &a){
  giac::angle_radian(a.settings.radians!=0,contextptr);giac::decimal_digits(a.settings.digits,contextptr);
  giac::complex_mode(a.settings.complex!=0,contextptr);giac::approx_mode(false,contextptr);
  giac::python_compat(0,contextptr);xcas_python_eval=0;
}
static unsigned checksum(const Preferences &p){return p.version^p.radians^p.decimal^p.digits^p.complex^p.compact^p.reserved;}
static void settingsPath(unsigned short *p){const char *name="\\\\fls0\\natcas.cfg";Bfile_StrToName_ncpy(p,(const unsigned char *)name,strlen(name)+1);}
static void loadSettings(App &a){
  unsigned short path[40];settingsPath(path);int h=Bfile_OpenFile_OS(path,READ);if(h<0)return;
  Preferences p;int count=Bfile_ReadFile_OS(h,&p,sizeof(p),0);Bfile_CloseFile_OS(h);
  if(count==sizeof(p)&&p.version==1&&p.radians<=1&&p.decimal<=1&&p.complex<=2&&p.compact<=1&&p.reserved<=1&&(p.digits==6||p.digits==10||p.digits==14)&&p.check==checksum(p))a.settings=p;
}
static void captureBaseline(App &a){
  if(a.baseline)delete a.baseline;a.baseline=giac::clone_context(contextptr);
  *a.baseline->history_in_ptr=*contextptr->history_in_ptr;*a.baseline->history_out_ptr=*contextptr->history_out_ptr;
  *a.baseline->rootofs=*contextptr->rootofs;*a.baseline->quoted_global_vars=*contextptr->quoted_global_vars;
}
static giac::context *snapshotContext(const giac::context *src){
  giac::context *c=giac::clone_context(src);*c->history_in_ptr=*src->history_in_ptr;*c->history_out_ptr=*src->history_out_ptr;
  *c->rootofs=*src->rootofs;*c->quoted_global_vars=*src->quoted_global_vars;return c;
}
static void restoreContext(const giac::context *c){
  *contextptr->globalptr=*c->globalptr;*contextptr->tabptr=*c->tabptr;*contextptr->rootofs=*c->rootofs;
  *contextptr->quoted_global_vars=*c->quoted_global_vars;*contextptr->history_in_ptr=*c->history_in_ptr;*contextptr->history_out_ptr=*c->history_out_ptr;contextptr->history_plot_ptr->clear();
}
static void trimHistory(giac::context *c){while(c->history_in_ptr->size()>MAX_ENTRIES+1)c->history_in_ptr->erase(c->history_in_ptr->begin());while(c->history_out_ptr->size()>MAX_ENTRIES+1)c->history_out_ptr->erase(c->history_out_ptr->begin());}
#include "main_memory.inc"
// SH7305 calendar registers: read a coherent snapshot without altering RTC flags.
// Calendar writes use the same OS syscall and seven-byte BCD format as KhiCAS.
static bool readClock(DateTime &d){
  volatile unsigned char *rtc=(volatile unsigned char *)0xA413FEC0;
  for(int attempt=0;attempt<8;++attempt){
    unsigned char second=rtc[2];unsigned short year=*(volatile unsigned short *)(rtc+14);
    unsigned char bytes[7]={(unsigned char)(year>>8),(unsigned char)year,rtc[12],rtc[10],rtc[6],rtc[4],second};
    if(second==rtc[2])return d.decode(bytes);
  }
  return false;
}
static Status status(App &a,char time[12]){
  Status s;s.radians=a.settings.radians;s.complex=a.settings.complex;s.decimal=a.settings.decimal;s.digits=a.settings.digits;
  s.keyMode=GetSetupSetting(0x14);
  DateTime d;bool valid=readClock(d);if(valid)d.format(time);else strcpy(time,"--/-- --:--");s.time=time;
  int now=valid?d.hour*3600+d.minute*60+d.second:0;
  if(a.battery<0||now<a.batteryStamp||now-a.batteryStamp>=5){a.battery=GetMainBatteryVoltage(1);a.batteryLevel=batteryBars(a.battery,a.batteryLevel);a.batteryStamp=now;}
  s.battery=a.battery;s.batteryLevel=a.batteryLevel;return s;
}
static void settingsMenu(App &a,Surface &s);
static bool clockSetup(App &a,Surface &s,bool first){
  DateTime d;readClock(d);ClockForm form(d);
  for(;;){char time[12];clockScreen(s,status(a,time),form,first);Bdisp_PutDisp_DD();int key=0;ncasGetKey(s,&key);
    if((key==KEY_CTRL_EXIT||key==KEY_CTRL_F1)&&!first)return false;
    if(key==KEY_CTRL_UP)form.move(-1);if(key==KEY_CTRL_DOWN)form.move(1);
    if(key==KEY_CTRL_LEFT)form.step(-1);if(key==KEY_CTRL_RIGHT)form.step(1);
    if(key>=KEY_CHAR_0&&key<=KEY_CHAR_9)form.digit(key-KEY_CHAR_0);
    if(key==KEY_CTRL_DEL)form.erase();
    if(key==KEY_CTRL_AC||key==KEY_PRGM_ACON){form.current()=0;form.typed=0;}
    if(key==KEY_CTRL_EXE&&form.selected<4){form.move(1);continue;}
    if(key==KEY_CTRL_F6||(key==KEY_CTRL_EXE&&form.selected==4)){
      unsigned char bytes[7];form.value.second=0;if(!form.value.encode(bytes)){form.error=true;continue;}
      RTC_SetDateTime(bytes);DateTime check;
      if(!readClock(check)||check.year!=form.value.year||check.month!=form.value.month||check.day!=form.value.day){form.error=true;continue;}
      a.settings.reserved=1;a.batteryStamp=-100;
      return true;
    }
  }
}
static const char *field(App &a,char *out){
  Editor &e=a.sheet.input;if(!a.sheet.editing||a.operation<0||!a.operationNode||e.n[a.operationNode].kind!=CALL)return 0;
  int r=e.row;while(r&&e.n[r].parent!=a.operationNode){int p=e.n[r].parent;r=p?e.n[p].parent:0;}
  if(!r)return 0;int index=0;for(int c=e.n[a.operationNode].first;c&&c!=r;c=e.n[c].next)++index;
  if(index>=operations[a.operation].count)return 0;
  strncpy(out,operations[a.operation].fields[index],63);out[63]=0;return out;
}
static void display(App &a,Surface &s,int noticeOffset=0){
  nativeHeader(false);
  char time[12];Status st=status(a,time);
  a.toolbar.build(st);a.sheet.bodyBottom=216-a.toolbar.height();
  char label[64];const char *hint=field(a,label);st.section=hint?hint:a.sheet.panMode?"PAN":a.sheet.suspendedInput==a.sheet.selected?"EDIT PENDING":a.toolbar.trail;
  sheetScreen(s,a.sheet,st,noticeOffset);a.toolbar.paint(s);
  Bdisp_PutDisp_DD();
}
static bool noticeKey(){
  int col=0,row=0;unsigned short code=0;
  if(GetKeyWait_OS(&col,&row,KEYWAIT_HALTOFF_TIMEROFF,0,1,&code)!=KEYREP_KEYEVENT)return false;
  // Return the event to normal GetKey handling, including MENU and modifiers.
  Keyboard_PutKeycode(col,row,code);return true;
}
static void showNotice(App &a,Surface &s){
  bool interrupted=false;
  for(int frame=6;frame>=0;--frame){display(a,s,frame*55);if(noticeKey()){interrupted=true;break;}OS_InnerWait_ms(18);}
  for(int wait=0;!interrupted&&wait<75;++wait){if(noticeKey()){interrupted=true;break;}OS_InnerWait_ms(20);}
  if(!interrupted)for(int frame=1;frame<=6;++frame){display(a,s,frame*55);if(noticeKey())break;OS_InnerWait_ms(18);}
  a.sheet.message[0]=a.sheet.input.error[0]=0;display(a,s);
}
static bool confirmWorkspace(App &a,Surface &s){
  bool selected=false;
  for(;;){display(a,s);workspacePromptScreen(s,selected);Bdisp_PutDisp_DD();int key=0;ncasGetKey(s,&key);
    if(key==KEY_CTRL_EXIT||key==KEY_CTRL_AC||key==KEY_PRGM_ACON||key==KEY_CTRL_F1)return false;
    if(key==KEY_CTRL_F6)return true;
    if(key==KEY_CTRL_LEFT)selected=false;if(key==KEY_CTRL_RIGHT)selected=true;
    if(key==KEY_CTRL_EXE)return selected;
  }
}
static bool isGraph(const giac::gen &g){if(g.is_symb_of_sommet(giac::at_pnt))return true;return g.type==giac::_VECT&&!g._VECTptr->empty()&&isGraph(g._VECTptr->back());}
static ustl::string naturalPrint(const giac::gen &g){
  if(g.type==giac::_VECT&&giac::ckmatrix(g)){ustl::string text("[");for(unsigned r=0;r<g._VECTptr->size();++r){if(r)text+=",";text+="[";const giac::vecteur &row=*(*g._VECTptr)[r]._VECTptr;for(unsigned c=0;c<row.size();++c){if(c)text+=",";text+=naturalPrint(row[c]);}text+="]";}return text+"]";}
  return g.print(contextptr);
}
static bool containsSurd(const giac::gen &g){
  if(g.type==giac::_FRAC)return containsSurd(g._FRACptr->num)||containsSurd(g._FRACptr->den);
  if(g.type==giac::_CPLX)return containsSurd(g._CPLXptr[0])||containsSurd(g._CPLXptr[1]);
  if(g.type==giac::_VECT){for(unsigned i=0;i<g._VECTptr->size();++i)if(containsSurd((*g._VECTptr)[i]))return true;return false;}
  if(g.type!=giac::_SYMB)return false;
  if(g.is_symb_of_sommet(giac::at_sqrt))return true;
  if(g.is_symb_of_sommet(giac::at_pow)&&g._SYMBptr->feuille.type==giac::_VECT&&g._SYMBptr->feuille._VECTptr->size()==2&&(*g._SYMBptr->feuille._VECTptr)[1].type==giac::_FRAC)return true;
  return containsSurd(g._SYMBptr->feuille);
}
static giac::gen rationalized(const giac::gen &g){
  if(g.type==giac::_VECT){giac::vecteur v=*g._VECTptr;for(unsigned i=0;i<v.size();++i)v[i]=rationalized(v[i]);return giac::gen(v,g.subtype);}
  if(g.type!=giac::_SYMB&&g.type!=giac::_FRAC&&g.type!=giac::_CPLX)return g;
  if(containsSurd(g))return giac::normal(g,contextptr);
  return g;
}
static bool resultText(App &a,int i,char *out,int capacity){
  if(isGraph(a.exact[i])){strcpy(out,"Graph (VIEW)");return true;}
  giac::gen value=a.decimalShown[i]?giac::evalf(a.exact[i],1,contextptr):rationalized(a.exact[i]);ustl::string text=naturalPrint(value);
  if(a.settings.complex==2&&value.type==giac::_CPLX){giac::gen radius=giac::abs(value,contextptr),angle=giac::arg(value,contextptr);text="ncas_polar("+naturalPrint(radius)+","+naturalPrint(angle)+")";}
  if(text.size()>=(unsigned)capacity||!a.sheet.scratch.load(text.c_str())){strcpy(out,"Open full result");return true;}
  strcpy(out,text.c_str());return true;
}
struct Engine:ReplayEngine {
  App &a;Surface &surface;Engine(App &app,Surface &s):a(app),surface(s){}
  void reset(){restoreBefore(0);}
  bool restoreBefore(int index){
    giac::context *c=index?a.after[index-1]:a.baseline;if(!c)return false;
    restoreContext(c);applySettings(a);
    for(int i=index;i<MAX_ENTRIES;++i){delete a.after[i];a.after[i]=0;a.exact[i]=0;a.parsed[i]=0;}return true;
  }
  bool evaluate(int i,const char *source,char *result,int capacity){
    applySettings(a);do_run(source,a.parsed[i],a.exact[i]);bool failed=giac::is_undef(a.exact[i]);
    if(a.exact[i].type==giac::_STRNG){const char *p=a.exact[i]._STRNGptr->c_str();if(strstr(p,"Error")||strstr(p,"error")||strstr(p,"Interrupted"))failed=true;}
    if(failed){strncpy(result,"Undefined or interrupted; check input and domain",capacity-1);result[capacity-1]=0;return false;}
    giac::history_in(contextptr).push_back(a.parsed[i]);giac::history_out(contextptr).push_back(a.exact[i]);
    a.decimalShown[i]=a.settings.decimal;trimHistory(contextptr);delete a.after[i];a.after[i]=snapshotContext(contextptr);
    return resultText(a,i,result,capacity);
  }
  void progress(Worksheet &w,int i,bool finished){display(a,surface);if(finished&&a.animate)OS_InnerWait_ms(55);}
};
static void run(App &a,Surface &s){
  Worksheet &w=a.sheet;int changed=w.selected;if(!w.commit()){w.followCursor=true;return;}
  if(w.evicted){delete a.baseline;a.baseline=a.after[0];for(int i=0;i<MAX_ENTRIES-1;++i){a.after[i]=a.after[i+1];a.exact[i]=a.exact[i+1];a.parsed[i]=a.parsed[i+1];a.decimalShown[i]=a.decimalShown[i+1];}a.after[MAX_ENTRIES-1]=0;--a.computedCount;changed=w.selected;}
  bool rebuild=!a.inSync||a.computedCount!=changed;a.animate=rebuild;Engine engine(a,s);
  a.inSync=replay(w,engine,changed,rebuild);
  a.computedCount=a.inSync?w.count:w.selected;
  a.operation = -1;
  if(a.inSync){int last=w.count-1;console_changed=1;
    Console_Clear_EditLine();Console_Input((const unsigned char *)w.entries[last].source);Console_NewLine(LINE_TYPE_INPUT,1);
    Console_Output((const unsigned char *)w.entries[last].result);Console_NewLine(LINE_TYPE_OUTPUT,1);
    check_do_graph(a.exact[last],a.parsed[last],6,contextptr);nativeHeader(false);
  }
}
static bool editable(App &a){if(!a.sheet.editing)a.sheet.edit(true);return a.sheet.editing;}
static void operation(App &a,int op){
  if(!editable(a))return;Worksheet &w=a.sheet;Editor &e=w.input;const Operation &o=operations[op];w.snapshot();
  bool filled=e.n[e.root].first!=0;if(!e.call(o.command,o.count,true))return;int fn=e.n[e.row].parent;
  for(int i=0;i<o.count;++i)if(!(i==0&&filled)&&o.defaults[i]&&o.defaults[i][0])e.setArgument(fn,i,o.defaults[i]);
  e.row=e.child(fn,0);e.before=0;a.operation=op;a.operationNode=fn;w.followCursor=true;
}
static bool matrixDimensions(App &a,Surface &s,int &rows,int &cols){
  int selected=0;bool typed=false;char error[64]="";
  for(;;){display(a,s);dimensionScreen(s,rows,cols,selected,error);Bdisp_PutDisp_DD();int key=0;ncasGetKey(s,&key);
    if(key==KEY_CTRL_EXIT||key==KEY_CTRL_F1||key==KEY_CTRL_AC)return false;
    if(key==KEY_CTRL_UP||key==KEY_CTRL_DOWN){selected=1-selected;typed=false;}
    int &value=selected?cols:rows;
    if(key>=KEY_CHAR_0&&key<=KEY_CHAR_9){value=typed?value*10+key-KEY_CHAR_0:key-KEY_CHAR_0;typed=true;if(value>99)value=key-KEY_CHAR_0;error[0]=0;}
    if(key==KEY_CTRL_DEL){value/=10;typed=false;}
    if(key==KEY_CTRL_LEFT||key==KEY_CTRL_RIGHT){value+=key==KEY_CTRL_LEFT?-1:1;if(value<1)value=6;if(value>6)value=1;typed=false;}
    if(key==KEY_CTRL_EXE&&selected==0){selected=1;typed=false;continue;}
    if(key==KEY_CTRL_F6||key==KEY_CTRL_EXE){if(rows>=1&&rows<=6&&cols>=1&&cols<=6)return true;strcpy(error,"Enter 1 to 6 rows and columns.");}
  }
}
static bool variableName(const ustl::string &v){if(v.empty()||!isalpha(v[0]))return false;for(unsigned i=1;i<v.size();++i)if(!isalnum(v[i])&&v[i]!='_')return false;return true;}
static bool action(App &a,Surface &s,int command){
  Worksheet &w=a.sheet;Editor &e=w.input;if(command==NOTHING)return false;
  if(command==BACK){a.toolbar.back();w.measure();w.focus(w.resultFocus);return false;}if(command==PAGE){++a.toolbar.page;return false;}
  if(command>=OPERATION){operation(a,command-OPERATION);return false;}
  if(command==CONFIRM_CLEAR){
    for(;;){display(a,s);s.clip(0,0,384,216);s.rect(20,48,344,122,INK);s.rect(22,50,340,118,PAPER);s.text(36,65,"Clear worksheet?",2,INK);s.text(36,94,"Delete every calculation in this sheet?",1,INK);s.text(36,108,"This cannot be undone.",1,MUTED);s.rect(34,137,118,22,INK);s.text(43,145,"F1 KEEP",1,PAPER);s.text(266,145,"F6 CLEAR",1,ERROR);Bdisp_PutDisp_DD();int key=0;ncasGetKey(s,&key);
      if(key==KEY_CTRL_EXIT||key==KEY_CTRL_AC||key==KEY_CTRL_F1||key==KEY_CTRL_EXE)return false;
      if(key==KEY_CTRL_F6)return action(a,s,CLEAR_SHEET);
    }
  }
  if(command==SETTINGS){settingsMenu(a,s);return false;}
  if(command>=0){a.toolbar.open(command);w.bodyBottom=216-a.toolbar.height();w.measure();w.focus(w.resultFocus);return false;}
  if(command==ANGLE||command==DOMAIN||command==OUTPUT||command==DIGITS||command==FONT){
    if(command==ANGLE)a.settings.radians=!a.settings.radians;if(command==DOMAIN)a.settings.complex=(a.settings.complex+1)%3;
    if(command==OUTPUT)a.settings.decimal=!a.settings.decimal;
    if(command==DIGITS)a.settings.digits=a.settings.digits==6?10:a.settings.digits==10?14:6;
    if(command==FONT)a.settings.compact=!a.settings.compact;applySettings(a);if(command==ANGLE||command==DOMAIN)a.inSync=false;
    if(command==OUTPUT||command==DIGITS||command==DOMAIN)for(int i=0;i<w.count;++i)if(w.entries[i].state==VALID){if(command==OUTPUT)a.decimalShown[i]=a.settings.decimal;resultText(a,i,w.entries[i].result,MAX_SOURCE);}
    w.reflow(a.settings.compact?1:2);return false;
  }
  if(command==FIRST||command==LAST){w.select(command==FIRST?0:w.count-1,command==LAST);return false;}
  if(command==NEW_LINE){if(w.editing&&e.n[e.root].first){strcpy(w.message,"Finish with EXE, or clear the input with AC.");return false;}w.newLine();a.operation=-1;return false;}
  if(command==PAN){w.panMode=!w.panMode;w.followCursor=false;a.toolbar.home();return false;}
  if(command==WORKSPACE){if(!confirmWorkspace(a,s))return false;a.inSync=false;return true;}
  if(command==FULL_RESULT){if(w.selected<w.count&&w.entries[w.selected].state==VALID){if(isGraph(a.exact[w.selected]))check_do_graph(a.exact[w.selected],a.parsed[w.selected],6,contextptr);else eqw(a.exact[w.selected],false);nativeHeader(false);}return false;}
  if(command==DECIMAL_TOGGLE){int i=w.selected;if(i<w.count&&w.entries[i].state==VALID){a.decimalShown[i]=!a.decimalShown[i];resultText(a,i,w.entries[i].result,MAX_SOURCE);w.updateEntry(i);w.measure();w.focus(true);}return false;}
  if(command==CLEAR_SHEET){w.clear();for(int i=0;i<MAX_ENTRIES;++i){a.exact[i]=0;a.parsed[i]=0;delete a.after[i];a.after[i]=0;}captureBaseline(a);a.computedCount=0;a.inSync=true;a.operation=-1;a.toolbar.home();return false;}
  if(command==DELETE_LINE){
    if(w.deleteLast()){int end=w.count;while(end&&!a.after[end-1])--end;restoreContext(end?a.after[end-1]:a.baseline);applySettings(a);a.computedCount=end;a.inSync=end==w.count;a.operation=-1;}return false;
  }
  if(command==UNDO){if(w.restoreLast()){int end=w.count;while(end&&!a.after[end-1])--end;restoreContext(end?a.after[end-1]:a.baseline);applySettings(a);a.computedCount=end;a.inSync=end==w.count;}else if(editable(a))w.undoEdit();a.operation=-1;return false;}
  if(command==USE_ANSWER){int i=w.selected;if(i<w.count&&w.entries[i].state==VALID){ustl::string value=a.exact[i].print(contextptr);if(w.newLine())w.input.load(value.c_str());a.operation=-1;}return false;}
  if(command==STORE){int i=w.selected;if(i<w.count&&w.entries[i].state==VALID){ustl::string variable("A");if(inputline("Store answer","Variable:",variable,false)==KEY_CTRL_EXE&&variableName(variable)){
    ustl::string value="sto("+a.exact[i].print(contextptr)+","+variable+")";if(w.newLine()&&w.input.load(value.c_str()))run(a,s);}}return false;}
  if(command==MATRIX_LOAD){ustl::string v("A");if(inputline("Edit matrix inline","Variable:",v,false)==KEY_CTRL_EXE&&variableName(v)){
    giac::gen value=giac::eval(giac::gen(v,contextptr),1,contextptr);if(value.type==giac::_VECT&&giac::ckmatrix(value)){if(w.newLine())w.input.load(value.print(contextptr).c_str());a.operation=-1;}else strcpy(w.message,"This variable is not a matrix.");}return false;}
  if(command==CATALOGUE){char buf[512]={0};if(showCatalog(buf,0,0)&&buf[0]&&editable(a)){w.snapshot();e.insert(buf);}return false;}
  if(!editable(a))return false;w.snapshot();
  if(command==CLEAR_FIELD)e.clearField();if(command==INSERT_MATRIX)e.matrix(2,2);
  if(command==MATRIX_33)e.matrix(3,3);if(command==MATRIX_31)e.matrix(3,1);if(command==MATRIX_21)e.matrix(2,1);
  if(command==MATRIX_CUSTOM){int rows=2,cols=2;if(matrixDimensions(a,s,rows,cols))e.matrix(rows,cols);}
  if(command==ROW_ADD)e.resizeMatrix(1,0);if(command==ROW_DEL)e.resizeMatrix(-1,0);if(command==COL_ADD)e.resizeMatrix(0,1);if(command==COL_DEL)e.resizeMatrix(0,-1);
  if(command==FRACTION_KEY)e.fraction();if(command==POWER_KEY)e.power();if(command==ROOT_KEY)e.radical();
  if(command==PI_KEY)e.insert("pi");if(command==I_KEY)e.insert("i");if(command==ANS_KEY)e.insert("ans()");if(command==EQUAL_KEY)e.insert("=");if(command==LIST_KEY)e.insert("[");return false;
}
static void settingsMenu(App &a,Surface &s){
  int selected=0;const int commands[]={ANGLE,DOMAIN,OUTPUT,DIGITS,FONT};
  for(;;){char time[12];settingsScreen(s,status(a,time),selected,a.settings.compact,a.sheet.message);Bdisp_PutDisp_DD();int key=0;ncasGetKey(s,&key);
    if(key==KEY_CTRL_EXIT||key==KEY_CTRL_F1||key==KEY_CTRL_SETUP)return;
    a.sheet.message[0]=0;
    if(key==KEY_CTRL_UP)selected=(selected+5)%6;if(key==KEY_CTRL_DOWN)selected=(selected+1)%6;
    if(key==KEY_CTRL_LEFT||key==KEY_CTRL_RIGHT||key==KEY_CTRL_EXE||key==KEY_CTRL_F5||key==KEY_CTRL_F6){
      if(selected==5){clockSetup(a,s,false);continue;}
      // Reverse the three-state precision cycle for LEFT.
      if(selected==3&&(key==KEY_CTRL_LEFT||key==KEY_CTRL_F5))a.settings.digits=a.settings.digits==6?10:a.settings.digits==10?14:6;
      action(a,s,commands[selected]);
    }
  }
}
static bool keyInput(Editor &e,int key){
  if(key==KEY_CHAR_DIV)return e.insert("/");if(key==KEY_CHAR_FRAC)return e.fraction();
  if(key==KEY_CHAR_POWROOT)return e.radical(true);if(key==KEY_CHAR_CUBEROOT){if(!e.radical(true))return false;e.setArgument(e.n[e.row].parent,1,"3");return true;}
  if(key==KEY_CHAR_POW)return e.power();if(key==KEY_CHAR_SQUARE)return e.square();if(key==KEY_CHAR_ROOT)return e.radical();
  if(key==KEY_CHAR_LPAR)return e.insert("(");if(key==KEY_CHAR_RPAR)return e.insert(")");
  if(key==KEY_CHAR_COMMA)return e.nextArgument();if(key==KEY_CTRL_XTT)return e.insert("x");
  if(key==KEY_CHAR_PI)return e.insert("pi");if(key==KEY_CHAR_IMGNRY)return e.insert("i");if(key==KEY_CHAR_ANS)return e.insert("ans()");
  if(key==KEY_CHAR_PLUS)return e.insert("+");if(key==KEY_CHAR_MINUS||key==KEY_CHAR_PMINUS)return e.insert("-");if(key==KEY_CHAR_MULT)return e.insert("*");
  if(key==KEY_CHAR_SIN)return e.insert("sin(");if(key==KEY_CHAR_COS)return e.insert("cos(");if(key==KEY_CHAR_TAN)return e.insert("tan(");
  if(key==KEY_CHAR_ASIN)return e.insert("asin(");if(key==KEY_CHAR_ACOS)return e.insert("acos(");if(key==KEY_CHAR_ATAN)return e.insert("atan(");
  if(key==KEY_CHAR_LN)return e.insert("ln(");if(key==KEY_CHAR_LOG)return e.insert("log10(");
  if(key==KEY_CHAR_EXP){e.insert("*10");return e.power();}if(key==KEY_CHAR_EXPN)return e.exponential();if(key==KEY_CHAR_EXPN10)return e.exponential("10");
  if(key==KEY_CHAR_RECIP){if(!e.power())return false;e.insert("-1");e.leave();return true;}
  if(key>=32&&key<=126){char text[2]={(char)key,0};return e.insert(text);}return false;
}
}
void naturalcas_run(){
  using namespace natural;Surface s((unsigned short *)GetVRAMAddress());nativeHeader(false);
  if(!state){splash(s);Bdisp_PutDisp_DD();state=new App;if(!state)return;loadSettings(*state);applySettings(*state);captureBaseline(*state);loadMainMemory(*state);state->sheet.reflow(state->settings.compact?1:2);OS_InnerWait_ms(1200);}
  App &a=*state;Worksheet &w=a.sheet;applySettings(a);DateTime clock;
  if(!a.settings.reserved||!readClock(clock))clockSetup(a,s,true);
  for(;;){
    if(w.message[0]||w.input.error[0])showNotice(a,s);else display(a,s);
    int key=0;ncasGetKey(s,&key);w.message[0]=0;w.input.error[0]=0;
    if(key==KEY_CTRL_SETUP){settingsMenu(a,s);continue;}
    if(key==KEY_CTRL_AC||key==KEY_PRGM_ACON){w.clearInput();a.operation=-1;continue;}
    const int soft[]={KEY_CTRL_F1,KEY_CTRL_F2,KEY_CTRL_F3,KEY_CTRL_F4,KEY_CTRL_F5,KEY_CTRL_F6};bool handled=false;
    for(int i=0;i<6;++i)if(key==soft[i]){handled=true;if(action(a,s,a.toolbar.actions[i])){nativeHeader(true);Console_Disp();return;}break;}if(handled)continue;
    if(key==KEY_CTRL_SHIFT||key==KEY_CTRL_ALPHA)continue;
    if(key==KEY_CTRL_EXIT&&a.toolbar.menu==HOME){w.clearInput();a.operation=-1;continue;}
    if(w.panMode){if(key==KEY_CTRL_LEFT)w.pan(-28,0);if(key==KEY_CTRL_RIGHT)w.pan(28,0);if(key==KEY_CTRL_UP)w.pan(0,-24);if(key==KEY_CTRL_DOWN)w.pan(0,24);if(key==KEY_CTRL_EXIT||key==KEY_CTRL_EXE){w.panMode=false;w.followCursor=w.editing;}continue;}
    if(key==KEY_CTRL_UP||key==KEY_CTRL_DOWN){w.navigate(key==KEY_CTRL_UP?-1:1);continue;}
    if(key==KEY_CTRL_LEFT||key==KEY_CTRL_RIGHT){if(!w.editing){w.edit(key==KEY_CTRL_LEFT);a.operation=-1;}else w.input.move(key==KEY_CTRL_LEFT?LEFT:RIGHT);w.followCursor=true;continue;}
    if(key==KEY_CTRL_EXIT){
      if(a.toolbar.menu!=HOME){a.toolbar.back();continue;}
      w.clearInput();a.operation=-1;continue;
    }
    if(key==KEY_CTRL_EXE){if(!w.editing&&w.suspendedInput==w.selected)w.edit();if(w.editing)run(a,s);else w.newLine();continue;}
    if(key==KEY_CTRL_FD||key==KEY_CTRL_SD){action(a,s,DECIMAL_TOGGLE);continue;}
    if(key==KEY_CTRL_UNDO){action(a,s,UNDO);continue;}
    if(!w.editing){w.newLine();a.operation=-1;if(!w.editing)continue;if(key==KEY_CHAR_PLUS||key==KEY_CHAR_MINUS||key==KEY_CHAR_MULT||key==KEY_CHAR_DIV)w.input.insert("ans()");}
    w.snapshot();if(key==KEY_CTRL_DEL){w.input.backspace();continue;}
    if(key==KEY_CHAR_MAT){w.input.matrix(2,2);a.toolbar.open(MAT);continue;}keyInput(w.input,key);
  }
}


bool naturalcas_save(){if(!natural::state)return false;if(!natural::saveMainMemory(*natural::state)){natural::Surface surface((unsigned short *)GetVRAMAddress());natural::display(*natural::state,surface);OS_InnerWait_ms(1800);}return true;}
