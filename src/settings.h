// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NCAS_SETTINGS_H
#define NCAS_SETTINGS_H
#include "worksheet.h"
namespace natural {
struct DateTime {
  int year,month,day,hour,minute,second;
  DateTime():year(2026),month(1),day(1),hour(12),minute(0),second(0){}
  bool valid() const;
  int days() const;
  void format(char out[12]) const;
  bool encode(unsigned char out[7]) const;
  bool decode(const unsigned char in[7]);
};
struct ClockForm {
  DateTime value;int selected,typed;bool error;
  ClockForm(const DateTime &v):value(v),selected(0),typed(0),error(false){}
  int &current();
  void digit(int n);
  void step(int direction);
  void move(int direction);
  void erase();
};
void settingsScreen(Surface &s,Status st,int selected,bool compact,const char *message=0);
void clockScreen(Surface &s,Status st,const ClockForm &form,bool first);
void dimensionScreen(Surface &s,int rows,int cols,int selected,const char *error);
void workspacePromptScreen(Surface &s,bool switchSelected);
}
#endif
