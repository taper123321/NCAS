const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..');const flag=process.argv.indexOf('--workspace');const workspace=path.resolve(flag>=0?process.argv[flag+1]:path.join(root,'.build'));const up=path.join(workspace,'giacbf'),src=path.join(root,'src');
const sdk=path.join(workspace,'sdk/PrizmSDK-win-0.6/bin'),inc=path.join(workspace,'opt/sh3eb-elf/libfxcg/include'),lib=path.join(workspace,'opt/sh3eb-elf/libfxcg/lib');
function run(tool,args){let r=cp.spawnSync(path.join(sdk,tool+'.exe'),args,{cwd:up,encoding:'utf8'});if(r.stdout)process.stdout.write(r.stdout);if(r.status){process.stderr.write(r.stderr||'');throw Error(tool+' failed '+r.status);}}
const flags=['-Os','-mb','-m4a-nofpu','-mhitachi','-std=c++11','-fpermissive','-w','-U_WIN32','-UWIN32','-D__STDC_LIMIT_MACROS','-D__STDC_CONSTANT_MACROS','-fno-use-cxa-atexit','-fno-threadsafe-statics','-fno-strict-aliasing','-fno-exceptions','-DHAVE_CONFIG_H','-DTIMEOUT','-DRELEASE','-DFILEICON','-DTURTLETAB','-DGINT_MALLOC','-DKMALLOC','-DCONFIG_BIGNUM','-DMICROPY_LIB','-I.','-I'+src,'-I'+path.join(inc,'ustl'),'-I'+inc];
let main=fs.readFileSync(path.join(up,'main.cc'),'utf8');
main=main.replace('void quit_handler(){','bool naturalcas_save();\nvoid quit_handler(){').replace(/(void quit_handler\(\)\{[\s\S]*?)save_session\(\);/, '$1if(!naturalcas_save())save_session();');
main=main.replace('int main1(){','void naturalcas_run();\nint main1(){');
main=main.replace('if((expr=Console_GetLine())','naturalcas_run();\n    if((expr=Console_GetLine())');
main=main.replace('if (1 ||\n      calculator==1','if (0 &&\n      calculator==1');
// Separate the new application's automatic session from an existing KhiCAS install.
main=main.replaceAll('"session"','"natcas"').replaceAll('session.xw','natcas.xw').replaceAll('session.py','natcas.py');
main=main.replace('if (confirm("Syntax?","F1: Xcas, F6: Python",0)==KEY_CTRL_F6)','if (false)');
fs.writeFileSync(path.join(up,'main-natural.cc'),main);
let catalog=fs.readFileSync(path.join(up,'catalogen.cpp'),'utf8').replaceAll('khicas50.8c2','natcas.8c2');fs.writeFileSync(path.join(up,'catalog-natural.cpp'),catalog);
let consoleSource=fs.readFileSync(path.join(up,'console.cc'),'utf8').replaceAll('"session"','"natcas"').replaceAll('session.xw','natcas.xw').replaceAll('session.py','natcas.py');fs.writeFileSync(path.join(up,'console-natural.cc'),consoleSource);
const inputs=[['main-natural.cc','main-natural.o'],['console-natural.cc','console-natural.o'],['catalog-natural.cpp','catalog-natural.o'],['app.cpp','znatural-app.o'],['editor.cpp','znatural-editor.o'],['screen.cpp','znatural-screen.o'],['catalog.cpp','znatural-catalog.o'],['worksheet.cpp','znatural-worksheet.o'],['toolbar.cpp','znatural-toolbar.o'],['settings.cpp','znatural-settings.o'],['symbols.cpp','znatural-symbols.o'],['memory.cpp','znatural-memory.o']];
for(const [f,o] of inputs){console.log('Compile '+f);run('sh3eb-elf-g++',[...flags,'-c',f.startsWith('main-')||f.startsWith('console-')||f.startsWith('catalog-')?f:path.join(src,f),'-o',o]);}
const make=fs.readFileSync(path.join(up,'Makefile'),'utf8');
let objects=make.match(/^CAS_OBJS = (.*)/m)[1].split('#')[0].trim().split(/\s+/);
objects.push(...make.match(/^GUI_OBJS = (fileGUI.*)/m)[1].trim().split(/\s+/).filter(x=>x!=='main.o'&&x!=='console.o'));
objects.push('main-natural.o','console-natural.o','catalog-natural.o','helpen.o','khelpen.o',...inputs.slice(3).map(x=>x[1]));
console.log('Link full KhiCAS engine + NCAS');
run('sh3eb-elf-g++',['-mb','-m4a-nofpu','-mhitachi','-static','-nostdlib','-Tprizm.ld','-Wl,--gc-sections,--print-memory-usage,-Map=naturalcas.map',...objects,'-L.','-L'+lib,'-Wl,--start-group','-lsupc++','-lmicropy','-ltommath','-lustl','-lm','-lc','-lgcc','-Wl,--end-group','-o','naturalcas.elf']);
const out=path.join(root,'calculator');fs.mkdirSync(out,{recursive:true});
run('sh3eb-elf-objcopy',['-O','binary','-R','.bss','-R','.gint_bss','-R','.rominram','naturalcas.elf','naturalcas.bin']);
run('sh3eb-elf-objcopy',['-O','binary','-j','.rominram','naturalcas.elf',path.join(out,'natcas.ac2')]);
run('mkg3a',['-n','basic:NCAS','-n','internal:NATCAS','-V','0.4.2','-i','uns:'+path.join(root,'assets/icon.png'),'-i','sel:'+path.join(root,'assets/icon-selected.png'),'naturalcas.bin',path.join(out,'NCAS.g3a')]);
console.log('Built '+out);
