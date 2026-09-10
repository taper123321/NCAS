// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings.h"
#include <stdio.h>
#include <string.h>
namespace natural {
int DateTime::days() const {
  static const int lengths[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if(month<1||month>12)return 0;
  return lengths[month-1]+(month==2&&year%4==0&&(year%100!=0||year%400==0));
}
bool DateTime::valid() const {return year>=2000&&year<=2099&&month>=1&&month<=12&&day>=1&&day<=days()&&hour>=0&&hour<24&&minute>=0&&minute<60&&second>=0&&second<60;}
static void pair(char *out,int v){out[0]='0'+v/10;out[1]='0'+v%10;}
void DateTime::format(char out[12]) const {
  strcpy(out,"--/-- --:--");if(!valid())return;
  pair(out,day);pair(out+3,month);pair(out+6,hour);pair(out+9,minute);
}
bool DateTime::encode(unsigned char out[7]) const {
  if(!valid())return false;int fields[]={year/100,year%100,month,day,hour,minute,second};
  for(int i=0;i<7;++i)out[i]=(fields[i]/10)*16+fields[i]%10;return true;
}
bool DateTime::decode(const unsigned char in[7]){
  int v[7];for(int i=0;i<7;++i){if((in[i]&15)>9||(in[i]>>4)>9)return false;v[i]=(in[i]>>4)*10+(in[i]&15);}
  DateTime d;d.year=v[0]*100+v[1];d.month=v[2];d.day=v[3];d.hour=v[4];d.minute=v[5];d.second=v[6];
  if(!d.valid())return false;*this=d;return true;
}
int &ClockForm::current(){switch(selected){case 0:return value.day;case 1:return value.month;case 2:return value.year;case 3:return value.hour;default:return value.minute;}}
void ClockForm::digit(int n){if(n<0||n>9)return;int limit=selected==2?4:2;if(typed>=limit)typed=0;int &v=current();v=typed?v*10+n:n;++typed;error=false;}
void ClockForm::erase(){current()/=10;if(typed>0)--typed;error=false;}
void ClockForm::move(int dir){selected=(selected+dir+5)%5;typed=0;error=false;}
void ClockForm::step(int dir){
  int lo=selected<3?1:0,hi=selected==0?value.days():selected==1?12:selected==2?2099:selected==3?23:59;
  if(selected==2)lo=2000;if(hi<lo)hi=31;
  int &v=current();v+=dir;if(v<lo)v=hi;if(v>hi)v=lo;
  if((selected==1||selected==2)&&value.day>value.days())value.day=value.days();typed=0;error=false;
}
static void soft(Surface &s,int index,const char *label){int x=index*64+1;s.rect(x,193,62,22,INK);s.label(x+2,195,58,18,label,PAPER);}
void settingsScreen(Surface &s,Status st,int selected,bool compact,const char *message){
  s.clip(0,0,384,216);s.rect(0,0,384,216,PAPER);st.section="Settings";statusBar(s,st);
  const char *names[]={"Angle","Domain","Output","Digits","Text size","Date/time"};
  char digits[8];sprintf(digits,"%d",st.digits);
  const char *values[]={st.radians?"RAD":"DEG",st.complex==2?"r\x1b\x1c":st.complex?"A+Bi":"REAL",st.decimal?"DEC":"EXACT",digits,compact?"Compact":"Normal","Set..."};
  for(int i=0;i<6;++i){int y=30+i*24;bool active=i==selected;if(active)s.rect(5,y-3,371,22,INK);
    s.text(12,y,names[i],2,active?PAPER:INK);s.text(181,y,":",2,active?PAPER:MUTED);s.text(210,y,values[i],2,active?PAPER:INK);}
  if(message&&*message)s.text(8,178,message,1,ERROR);
  soft(s,0,"BACK");soft(s,4,"<");soft(s,5,">");
}
void clockScreen(Surface &s,Status st,const ClockForm &f,bool first){
  s.clip(0,0,384,216);s.rect(0,0,384,216,PAPER);st.section=first?"Clock setup":"Date/time";statusBar(s,st);
  const char *names[]={"Day","Month","Year","Hour (24h)","Minute"};
  int values[]={f.value.day,f.value.month,f.value.year,f.value.hour,f.value.minute};
  for(int i=0;i<5;++i){int y=33+i*26;bool active=i==f.selected;if(active)s.rect(5,y-4,371,23,INK);
    char value[8];sprintf(value,"%d",values[i]);s.text(13,y,names[i],2,active?PAPER:INK);s.text(218,y,value,2,active?PAPER:INK);}
  s.text(9,171,f.error?"Check date, year 2000-2099 and 24-hour time.":"Type digits; UP/DOWN select; LEFT/RIGHT adjust.",1,f.error?ERROR:MUTED);
  if(!first)soft(s,0,"BACK");soft(s,5,"SAVE");
}
void dimensionScreen(Surface &s,int rows,int cols,int selected,const char *error){
 s.clip(0,0,384,216);s.rect(28,38,332,139,MUTED);s.rect(24,34,332,139,INK);s.rect(26,36,328,135,PAPER);
 s.text(40,47,"Matrix size",2,INK);
 for(int i=0;i<2;++i){int y=79+26*i;if(selected==i)s.rect(36,y-4,308,24,SELECT);char value[12];sprintf(value,"%d",i?cols:rows);s.text(43,y,i?"Columns":"Rows",2,INK);s.text(270,y,value,2,INK);}
 if(error&&*error)s.text(40,132,error,1,ERROR);else s.text(40,132,"Type dimensions, 1 to 6 each.",1,MUTED);
 s.text(40,154,"F1 CANCEL",1,INK);s.text(272,154,"F6 INSERT",1,ACCENT);
}
void workspacePromptScreen(Surface &s,bool switchSelected){
  s.clip(0,0,384,216);s.rect(22,42,342,139,MUTED);s.rect(18,38,344,139,INK);s.rect(20,40,340,135,PAPER);
  s.text(35,53,"Switch to KhiCAS?",2,INK);
  s.text(35,80,"This opens the original interface",1,INK);
  s.text(35,91,"with different menus and controls.",1,INK);
  s.text(35,107,"Your NCAS worksheet stays in memory.",1,MUTED);
  s.text(35,119,"Return: EXIT to prompt, then 0 EXE.",1,MUTED);
  s.rect(35,141,134,23,switchSelected?PAPER:INK);s.rect(209,141,134,23,switchSelected?INK:PAPER);
  s.text(45,149,"F1 CANCEL",1,switchSelected?INK:PAPER);s.text(219,149,"F6 SWITCH",1,switchSelected?PAPER:INK);
}
}
