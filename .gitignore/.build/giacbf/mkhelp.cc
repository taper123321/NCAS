// Example:
// ./mkhelp 0 > static_helpfr.h
// ./mkhelp 1 > static_helpen.h

#include <iostream>
#include <fstream>
#include <string.h>

const int HELP_LANGUAGES=5;

struct static_help_t {
  const char * cmd_name;
  const char * cmd_howto[HELP_LANGUAGES];
  const char * cmd_syntax;
  const char * cmd_related;
  const char * cmd_examples;
};

const static_help_t static_help[]={
#include "static_help.h"
};

const int static_help_size=sizeof(static_help)/sizeof(static_help_t);
using namespace std;

void output(ostream & of,const char * s){
  if (!s) return;
  int l=strlen(s);
  for (int i=0;i<l;++i){
    if (s[i]=='"'){
      of << "\\\"";
      continue;
    }
    of << s[i];
  }
}

int main(int argc,char **argv){
  ofstream fr("static_helpfr.h");
  ofstream en("static_helpen.h");
  ofstream es("static_helpes.h");
  ofstream el("static_helpel.h");
  ofstream de("static_helpde.h");
  ofstream * ptrtab[]={&fr,&en,&es,&el,&de};
  for (int l=0;l<sizeof(ptrtab)/sizeof(ofstream *);++l){
    ofstream * ptr=ptrtab[l];
    for (int i=0;i<static_help_size;++i){
      const static_help_t & h=static_help[i];
      (*ptr) << "{\"" ;
      output(*ptr,h.cmd_name);
      (*ptr) << "\",\"";
      output(*ptr,h.cmd_howto[l]);
      (*ptr) << "\",\"";
      output(*ptr,h.cmd_syntax);
      (*ptr) << "\",\"";
      output(*ptr,h.cmd_related);
      (*ptr) << "\",\"";
      output(*ptr,h.cmd_examples);
      (*ptr) << "\"},\n";
    }
  }
}
