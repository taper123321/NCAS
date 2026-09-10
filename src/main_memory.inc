// SPDX-License-Identifier: GPL-3.0-or-later
// Calculator main memory, written only by the MENU/app-exit hook.
static bool memoryString(Memory &s,ustl::string &str){
 unsigned n=s.number(str.size());if(!s.ok||n>65535){s.ok=false;return false;}
 if(s.reading){char *p=new char[n+1];if(!p){s.ok=false;return false;}s.bytes(p,n);p[n]=0;if(s.ok)str=p;delete[]p;}
 else s.bytes((void *)str.c_str(),n);return s.ok;
}
static bool memoryGen(Memory &s,giac::gen &g,int depth=0){
 if(depth>48){s.ok=false;return false;}unsigned type=s.number(g.type),sub=s.number((unsigned char)g.subtype);if(!s.ok)return false;
 if(type==giac::_VECT){unsigned count=s.number(s.reading?0:g._VECTptr->size());if(count>8192){s.ok=false;return false;}
  if(s.reading){giac::vecteur v;for(unsigned i=0;i<count&&s.ok;++i){giac::gen e;memoryGen(s,e,depth+1);v.push_back(e);}if(s.ok)g=giac::gen(v,(signed char)sub);}
  else for(unsigned i=0;i<count&&s.ok;++i)memoryGen(s,(*g._VECTptr)[i],depth+1);
 }else if(type==giac::_DOUBLE_){double v=s.reading?0:g.DOUBLE_val();s.bytes(&v,sizeof(v));if(s.reading)g=giac::gen(v);}
 else if(type==giac::_INT_){int v=s.number(s.reading?0:g.val);if(s.reading){g=giac::gen(v);g.subtype=(signed char)sub;}}
 else if(type==giac::_CPLX||type==giac::_FRAC){giac::gen a,b;if(!s.reading){a=type==giac::_CPLX?g._CPLXptr[0]:g._FRACptr->num;b=type==giac::_CPLX?g._CPLXptr[1]:g._FRACptr->den;}memoryGen(s,a,depth+1);memoryGen(s,b,depth+1);if(s.reading&&s.ok)g=type==giac::_CPLX?giac::gen(a,b):giac::gen(giac::fraction(a,b));}
 else {ustl::string str=s.reading?ustl::string():type==giac::_STRNG?*g._STRNGptr:g.print(contextptr);memoryString(s,str);if(s.reading&&s.ok){g=type==giac::_STRNG?giac::string2gen(str,false):giac::gen(str,contextptr);g.subtype=(signed char)sub;}}
 return s.ok;
}
static void contextMemory(Memory &s,giac::context *c,bool history){
 unsigned seed=s.number(giac::rand_seed(c));if(s.reading)giac::rand_seed(seed,c);unsigned count=s.number(c->tabptr->size());if(count>2048){s.ok=false;return;}
 if(s.reading){c->tabptr->clear();for(unsigned i=0;i<count&&s.ok;++i){ustl::string name;giac::gen value;memoryString(s,name);memoryGen(s,value);if(!s.ok)break;giac::gen id(name,c);if(id.type!=giac::_IDNT){s.ok=false;break;}giac::sto(value,id,c);}}
 else for(giac::sym_tab::const_iterator it=c->tabptr->begin();it!=c->tabptr->end()&&s.ok;++it){ustl::string name(it->first);giac::gen value=it->second;memoryString(s,name);memoryGen(s,value);}
 giac::gen roots(*c->rootofs),quoted(*c->quoted_global_vars);memoryGen(s,roots);memoryGen(s,quoted);
 if(s.reading&&s.ok){if(roots.type!=giac::_VECT||quoted.type!=giac::_VECT){s.ok=false;return;}*c->rootofs=*roots._VECTptr;*c->quoted_global_vars=*quoted._VECTptr;}
 if(history){giac::gen in(*c->history_in_ptr),out(*c->history_out_ptr);memoryGen(s,in);memoryGen(s,out);if(s.reading&&s.ok){if(in.type!=giac::_VECT||out.type!=giac::_VECT){s.ok=false;return;}*c->history_in_ptr=*in._VECTptr;*c->history_out_ptr=*out._VECTptr;}}
}
static bool appMemory(Memory &s,App &a){
 if(!worksheetMemory(s,a.sheet))return false;s.bytes(&a.settings,sizeof(a.settings));
 if(a.settings.version!=1||a.settings.complex>2||a.settings.radians>1||a.settings.decimal>1||a.settings.compact>1||a.settings.reserved>1||(a.settings.digits!=6&&a.settings.digits!=10&&a.settings.digits!=14)){s.ok=false;return false;}
 contextMemory(s,a.baseline,true);
 for(int i=0;i<a.sheet.count&&s.ok;++i){unsigned exists=s.number(a.after[i]?1:0);if(exists>1){s.ok=false;break;}memoryGen(s,a.parsed[i]);memoryGen(s,a.exact[i]);unsigned dec=s.number(a.decimalShown[i]);if(dec>1){s.ok=false;break;}a.decimalShown[i]=dec;
  if(exists){if(s.reading)a.after[i]=snapshotContext(i&&a.after[i-1]?a.after[i-1]:a.baseline);contextMemory(s,a.after[i],false);if(s.reading){a.after[i]->history_in_ptr->push_back(a.parsed[i]);a.after[i]->history_out_ptr->push_back(a.exact[i]);trimHistory(a.after[i]);}}
 }
 return s.ok;
}
static unsigned read32(const unsigned char *p){return (unsigned(p[0])<<24)|(unsigned(p[1])<<16)|(unsigned(p[2])<<8)|p[3];}
static void write32(unsigned char *p,unsigned v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
static unsigned char memoryDir[]="@NATCAS",memoryItem[]="WORKSHT";
static bool saveMainMemory(App &a){
 if(!a.baseline)return false;Memory count(0,MAX_SAVED_RAW);appMemory(count,a);if(!count.ok){strcpy(a.sheet.message,"Worksheet too large for main memory.");return false;}
 if(count.hash==a.savedHash&&count.size==a.savedSize)return true;
 unsigned char *raw=new unsigned char[count.size],*packed=new unsigned char[MAX_SAVED_PACKED];if(!raw||!packed){delete[]raw;delete[]packed;return false;}
 Memory writer(raw,count.size);bool ok=appMemory(writer,a)&&writer.size==count.size&&writer.hash==count.hash;unsigned n=ok?compressMemory(raw,count.size,packed+20,MAX_SAVED_PACKED-23):0;delete[]raw;
 if(n){memcpy(packed,"NCASMCS4",8);write32(packed+8,count.size);write32(packed+12,n);write32(packed+16,count.hash);unsigned bytes=(n+23)&~3u;memset(packed+20+n,0,bytes-20-n);
   // The OS owns the record and its backup. Do not delete the previous item first.
   MCS_CreateDirectory(memoryDir);ok=MCS_WriteItem(memoryDir,memoryItem,0,bytes,(int)packed)==0;
 }else ok=false;delete[]packed;
 if(ok){a.savedHash=count.hash;a.savedSize=count.size;}else strcpy(a.sheet.message,"Main memory full; worksheet was not saved.");return ok;
}
static void disposeContexts(App &a){delete a.baseline;a.baseline=0;for(int i=0;i<MAX_ENTRIES;++i){delete a.after[i];a.after[i]=0;}}
static bool loadMainMemory(App &a){
 int length=0;if(MCSGetDlen2(memoryDir,memoryItem,&length)!=0||length<20||length>(int)MAX_SAVED_PACKED)return false;
 unsigned char *packed=new unsigned char[length];if(!packed)return false;
 bool ok=MCSGetData1(0,length,packed)==0&&!memcmp(packed,"NCASMCS4",8);unsigned rawSize=ok?read32(packed+8):0,n=ok?read32(packed+12):0,hash=ok?read32(packed+16):0;
 ok=ok&&rawSize>0&&rawSize<=MAX_SAVED_RAW&&n>0&&n<=unsigned(length-20);unsigned char *raw=ok?new unsigned char[rawSize]:0;
 ok=ok&&raw&&decompressMemory(packed+20,n,raw,rawSize);delete[]packed;if(!ok){delete[]raw;return false;}
 Memory verify(raw,rawSize,true); // Check the complete uncompressed payload before parsing.
 unsigned actual=2166136261u;for(unsigned i=0;i<rawSize;++i){actual^=raw[i];actual*=16777619u;}if(actual!=hash){delete[]raw;return false;}
 App *loaded=new App;if(!loaded){delete[]raw;return false;}loaded->baseline=snapshotContext(a.baseline);
 ok=appMemory(verify,*loaded)&&verify.size==rawSize&&verify.hash==hash;delete[]raw;
 if(ok){disposeContexts(a);a.sheet=loaded->sheet;a.settings=loaded->settings;a.baseline=loaded->baseline;loaded->baseline=0;
  for(int i=0;i<MAX_ENTRIES;++i){a.after[i]=loaded->after[i];loaded->after[i]=0;a.exact[i]=loaded->exact[i];a.parsed[i]=loaded->parsed[i];a.decimalShown[i]=loaded->decimalShown[i];}
  a.computedCount=0;while(a.computedCount<a.sheet.count&&a.after[a.computedCount])++a.computedCount;restoreContext(a.computedCount?a.after[a.computedCount-1]:a.baseline);a.inSync=a.computedCount==a.sheet.count;applySettings(a);a.savedHash=hash;a.savedSize=rawSize;
 }
 disposeContexts(*loaded);delete loaded;return ok;
}
