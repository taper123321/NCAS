// Usage: node tests/reference.cjs C:/prepared/build-directory
// Run editor-test screenshots in that directory first; requires giac.js from the lock.
const fs=require('fs'),path=require('path');const workspace=path.resolve(process.argv[2]||path.join(__dirname,'../.build'));const project=path.resolve(__dirname,'..');
const code=fs.readFileSync(workspace+'/giac.js','utf8');
const make=new Function('require','__dirname','module','process','var svgwidth=6;var UI={add_evalf:0,disable3d:1,js_bigint:0};var Module={worker:true,noInitialRun:true,print:function(){},printErr:function(){}};'+code+';return Module;');
const m=make(require,__dirname,{exports:{}},process),calc=m.cwrap('caseval','string',['string']);
let results=[];
function check(input,expected){let got=calc(input),pass=got===expected;results.push({input,expected,actual:got,pass});if(!pass)console.error(JSON.stringify(results.at(-1)));}
check('1/2+1/6','2/3');calc('angle_radian(0)');check('sin(30)','1/2');
check('det([[1,2],[3,4]])','-2');check('inv([[1,2],[3,4]])','[[-2,1],[3/2,-1/2]]');
check('[[1,2],[3,4]]*[[2,0],[1,2]]','[[4,4],[10,8]]');
check('binomial(10,1/2,3)','15/128');check('binomial_cdf(10,1/2,3)','11/64');
check('dot([1,2,3],[4,5,6])','32');check('cross([1,2,3],[4,5,6])','[-3,6,-3]');
calc('angle_radian(1)');check('diff(x^3-3*x,x)','3*x^2-3');check('integrate(x^2,x,0,3)','9');
check('solve(x^2-5*x+6=0,x)','list[2,3]');check('coeff(x^3+2*x^2,x,2)','2');
check('charpoly([[1,2],[3,4]],x)','x^2-5*x-2');check('limit(sin(x)/x,x=0)','1');
// Algebraic meaning of the editor's parsed/serialised examples.
for(const line of fs.readFileSync(workspace+'/giacbf/roundtrips.tsv','utf8').trim().split('\n')){
 const [original,serialised]=line.split('\t');if(original==='ans()'||original==='x=2')continue;
 let expected=calc(original),actual=calc(serialised),pass=expected===actual;
 results.push({original,serialised,expected,actual,pass});if(!pass)console.error(JSON.stringify(results.at(-1)));
}
const operations=JSON.parse(fs.readFileSync(project+'/tests/operations.json'));
// Actual Giac assignment and Ans semantics used by the worksheet replay adapter.
calc('restart()');check('ncas_a:=2','2');check('ncas_a*3','6');check('ans()+ncas_a','8');
calc('restart()');check('ncas_a:=4','4');check('ncas_a*3','12');check('ans()+ncas_a','16');
check('5*5/2','25/2');check('(5/5)','1');calc('purge(ncas_a)');
check('normal(1/sqrt(2))','√2/2');check('normal(1/(1+sqrt(2)))','√2-1');check('normal(surd(8,3))','2');check('simplify(e^x-exp(x))','0');
calc('angle_radian(0)');check('arg(1+i)','45.0');calc('angle_radian(1)');check('arg(1+i)','pi/4');
const graphCases=['plot(x^2,x,-10,10)','plotparam([cos(t),sin(t)],t=0..2*pi)','plotpolar(1+cos(t),t=0..2*pi)','plotimplicit(x^2+y^2=4,x=-3..3,y=-3..3)'];
let graphs=graphCases.map(input=>{try{let output=calc(input),pass=output.includes('<svg')&&(output.includes('<polyline')||output.includes('<path')||output.includes('<circle'));return {input,pass,bytes:output.length};}catch(e){return {input,pass:false,error:String(e)};}});fs.writeFileSync(project+'/tests/graph-reference-results.json',JSON.stringify(graphs,null,2));console.log(JSON.stringify(graphs));
let catalogue=[];for(const o of operations){let args=o.defaults.map((d,i)=>d||((o.category==='Matrices')?'[[1,2],[3,4]]':(['Statistics'].includes(o.category))?'[1,2,3]':o.command==='isprime'||o.command==='ifactor'?'12':o.command==='solve'||o.command==='csolve'||o.command==='fsolve'?'x^2-1=0':i===0?'x^2-1':'2'));
 if(o.category==='Graphs')continue;let input=o.command+'('+args.join(',')+')',result=calc(input);catalogue.push({title:o.title,input,result});}
fs.writeFileSync(project+'/tests/reference-results.json',JSON.stringify({engine:'Upstream Giac JavaScript reference; not the CG50 binary',results,catalogue},null,2));
console.log(results.filter(x=>x.pass).length+'/'+results.length+' exact/reference checks passed; '+catalogue.length+' non-graph catalogue probes recorded.');
process.exitCode=results.every(x=>x.pass)&&graphs.every(x=>x.pass)?0:1;
