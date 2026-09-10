// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NATURALCAS_CATALOG_H
#define NATURALCAS_CATALOG_H
namespace natural {
struct Operation {const char *category,*title,*command;int count;const char *defaults[6];const char *fields[6];bool radians;};
extern const Operation operations[];
extern const int operationCount;
}
#endif
