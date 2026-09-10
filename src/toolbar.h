// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NCAS_TOOLBAR_H
#define NCAS_TOOLBAR_H
#include "worksheet.h"
#include "catalog.h"
namespace natural {
enum Menu { HOME, MORE, JUMP, EDIT, MAT, MATH, CALC, SETTINGS, RESIZE, SYMBOLS, CONFIRM_CLEAR, CONFIRM_DELETE, CATEGORY=100 };
enum Action { NOTHING=-1, BACK=-2, PAGE=-3, FIRST=-4, LAST=-5, NEW_LINE=-6, PAN=-7,
 UNDO=-8, DELETE_LINE=-9, CLEAR_FIELD=-10, CLEAR_SHEET=-11, INSERT_MATRIX=-12,
 ROW_ADD=-13, ROW_DEL=-14, COL_ADD=-15, COL_DEL=-16, FRACTION_KEY=-17, POWER_KEY=-18,
 ROOT_KEY=-19, PI_KEY=-20, I_KEY=-21, ANS_KEY=-22, EQUAL_KEY=-23, LIST_KEY=-24,
 ANGLE=-25, DOMAIN=-26, OUTPUT=-27, DIGITS=-28, FONT=-29, CATALOGUE=-30, WORKSPACE=-31,
 FULL_RESULT=-32, USE_ANSWER=-33, STORE=-34, MATRIX_LOAD=-35, DECIMAL_TOGGLE=-36,
 MATRIX_33=-37, MATRIX_31=-38, MATRIX_21=-39, MATRIX_CUSTOM=-40,
 OPERATION=1000 };
struct Toolbar {
  int menu,page,depth,ancestors[8],pages[8];
  int actions[6];char labels[6][40],trail[64];
  Toolbar();
  void open(int next);
  void back();
  void home();
  void build(const Status &status);
  int height() const {return 24;}
  void paint(Surface &s,const char *field=0);
};
}
#endif
