// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NCAS_WORKSHEET_H
#define NCAS_WORKSHEET_H
#include "screen.h"
namespace natural {
const int MAX_ENTRIES=32, MAX_FORM=4096;
enum ResultState { PENDING, VALID, FAILED };
struct Entry {
  char source[MAX_SOURCE],result[MAX_SOURCE];
  unsigned char form[MAX_FORM];
  int formSize,inputUp,inputDown,inputWidth,resultUp,resultDown,resultWidth;
  ResultState state;
  Entry();
};
struct Worksheet {
  Entry entries[MAX_ENTRIES],deletedEntry;
  bool deleted,evicted;
  // selected==count is the draft. Inputs and results have independent vertical space.
  int count,selected,scrollY,scrollX,scale,bodyBottom,totalHeight;
  int suspendedInput;
  bool editing,resultFocus,panMode,followCursor;
  Editor input,scratch,undo;
  bool undoReady;
  char message[96];
  Worksheet();
  void snapshot();
  bool deleteLast();
  bool restoreLast();
  void undoEdit();
  bool commit();
  bool select(int index,bool result=false);
  void edit(bool fromRight=true);
  bool newLine();
  void navigate(int direction);
  void pan(int dx,int dy);
  void measure();
  void updateEntry(int index);
  void reflow(int textScale);
  int entryTop(int index) const;
  int inputHeight(int index) const;
  int resultHeight(int index) const;
  void focus(bool result);
  void cursorVisible();
  void clampScroll();
  void clear();
  void clearInput();
  void suspendInput();
};
struct Status {
  bool radians,decimal;
  int complex;
  int digits,battery,batteryLevel;
  unsigned keyMode;
  const char *time,*section;
  Status():radians(false),complex(false),decimal(false),digits(10),battery(-1),batteryLevel(-1),keyMode(0),time(0),section(0){}
};
int batteryBars(int centivolts,int previous=-1);
void statusBar(Surface &s,const Status &status);
void modifierIndicators(Surface &s,unsigned keyMode);
void sheetScreen(Surface &s,Worksheet &sheet,const Status &status,int noticeOffset=0);
void splash(Surface &s);
struct ReplayEngine {
  virtual void reset()=0;
  virtual bool restoreBefore(int index)=0;
  virtual bool evaluate(int index,const char *source,char *result,int capacity)=0;
  virtual void progress(Worksheet &sheet,int index,bool finished)=0;
  virtual ~ReplayEngine(){}
};
bool replay(Worksheet &sheet,ReplayEngine &engine,int changedFrom,bool rebuild=true);
}
#endif
