// SPDX-License-Identifier: GPL-3.0-or-later
// Download only public, SHA-256-pinned build inputs into the chosen build directory.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),cp=require('child_process');
const root=path.resolve(__dirname,'..');
const where=process.argv.indexOf('--workspace');
const workspace=path.resolve(where>=0?process.argv[where+1]:path.join(root,'.build'));
fs.mkdirSync(workspace,{recursive:true});
const lock=JSON.parse(fs.readFileSync(path.join(root,'dependencies.lock.json')));
function sha(file){return crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');}
function run(command,args){const r=cp.spawnSync(command,args,{cwd:workspace,encoding:'utf8'});if(r.status)throw Error(r.stderr||r.stdout||command+' failed');}
async function get(name){
 const d=lock.dependencies.find(x=>x.file===name);if(!d)throw Error('Unpinned dependency '+name);
 const dest=path.join(workspace,name),bundled=path.join(root,'third_party/source',name);
 if(!fs.existsSync(dest)&&fs.existsSync(bundled))fs.copyFileSync(bundled,dest);
 if(!fs.existsSync(dest)){console.log('Download '+name);const response=await fetch(d.url);if(!response.ok)throw Error(response.status+' '+d.url);fs.writeFileSync(dest,Buffer.from(await response.arrayBuffer()));}
 if(sha(dest)!==d.sha256)throw Error('Checksum mismatch for '+name+'. Do not build from a silently changed archive.');
 return dest;
}
async function main(){
 await get('giacbf.tgz');await get('sdk.zip');
 if(!fs.existsSync(path.join(workspace,'giacbf/Makefile'))){
   run('tar',['-xzf','giacbf.tgz','--exclude=giacbf/archive','--exclude=giacbf/iostream','--exclude=giacbf/publish','--exclude=giacbf/khicasioen.html','--exclude=giacbf/khicasio.html']);
 }
 const support=path.join(workspace,'opt/sh3eb-elf/libfxcg');
 const bundledSupport=path.join(root,'third_party/support');
 if(!fs.existsSync(support)){
   if(fs.existsSync(bundledSupport))fs.cpSync(bundledSupport,support,{recursive:true});
   else {await get('casiolocal.tgz');run('tar',['-xzf','casiolocal.tgz','--exclude=opt/sh3eb-elf/libfxcg/include/fxcg/rtc.h','--exclude=opt/sh3eb-elf/libfxcg/include/keyboard.h','opt/sh3eb-elf/libfxcg']);}
 }
 if(!fs.existsSync(path.join(workspace,'sdk/PrizmSDK-win-0.6/bin/sh3eb-elf-g++.exe'))){
   fs.mkdirSync(path.join(workspace,'sdk'),{recursive:true});run('tar',['-xf','sdk.zip','-C','sdk']);
 }
 fs.copyFileSync(path.join(workspace,'giacbf/iostream.new'),path.join(workspace,'giacbf/iostream'));
 fs.copyFileSync(path.join(support,'include/rtc.h'),path.join(support,'include/fxcg/rtc.h'));
 const config=path.join(support,'include/ustl/config.h');let text=fs.readFileSync(config,'utf8');
 fs.writeFileSync(config,text.replace('typedef size_t ssize_t;','typedef signed ssize_t;'));
 console.log('Prepared '+workspace);
}
main().catch(e=>{console.error(e.message);process.exitCode=1;});
