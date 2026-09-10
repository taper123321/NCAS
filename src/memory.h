// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NCAS_MEMORY_H
#define NCAS_MEMORY_H
#include "worksheet.h"
namespace natural {
const unsigned MAX_SAVED_RAW=524288,MAX_SAVED_PACKED=49152;
struct Memory {
 unsigned char *data;unsigned capacity,size,hash;bool reading,ok;
 Memory(unsigned char *p,unsigned cap,bool read=false):data(p),capacity(cap),size(0),hash(2166136261u),reading(read),ok(true){}
 bool bytes(void *p,unsigned n);
 unsigned number(unsigned v=0);
 bool text(char *p,unsigned cap);
};
bool worksheetMemory(Memory &s,Worksheet &w);
unsigned compressMemory(const unsigned char *in,unsigned length,unsigned char *out,unsigned capacity);
bool decompressMemory(const unsigned char *in,unsigned length,unsigned char *out,unsigned capacity);
}
#endif
