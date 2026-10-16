/* Exact counting is checked against independent subset enumeration on small
 * circulant graphs, then on the certificate presets and complete graphs. */
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const vm=require('node:vm');
const root=path.resolve(__dirname,'../..');
global.window={};
vm.runInThisContext(fs.readFileSync(path.join(root,'docs/assets/data.js'),'utf8'));
vm.runInThisContext(fs.readFileSync(path.join(root,'docs/assets/graph.js'),'utf8'));
function enumerate(n,k,red,colour){
  let count=0;const chosen=[];
  function visit(start){
    if(chosen.length===k){
      for(let i=0;i<k;i++)for(let j=i+1;j<k;j++){
        const delta=Math.abs(chosen[i]-chosen[j]);
        if(red.has(Math.min(delta,n-delta))!==colour)return;
      }
      count++;return;
    }
    for(let v=start;v<=n-(k-chosen.length);v++){chosen.push(v);visit(v+1);chosen.pop();}
  }
  visit(0);return count;
}
let checks=0;
for(let n=5;n<=12;n++)for(let trial=0;trial<30;trial++){
  const mask=(trial*1543+n*71)%(1<<Math.floor(n/2));
  const red=new Set(Array.from({length:Math.floor(n/2)},(_,i)=>i+1).filter(d=>mask&(1<<(d-1))));
  const blueTarget=3+trial%3,redTarget=3+(trial+1)%4;
  const result=RamseyGraph.analyse(n,blueTarget,redTarget,red,3);
  assert.equal(result.blue.count,enumerate(n,blueTarget,red,false));
  assert.equal(result.red.count,enumerate(n,redTarget,red,true));
  for(const [colour,target] of [['blue',blueTarget],['red',redTarget]]){
    assert.ok(result[colour].witnesses.length<=3);
    for(const witness of result[colour].witnesses){
      assert.equal(new Set(witness).size,target);
      for(let i=0;i<target;i++)for(let j=i+1;j<target;j++)assert.equal(red.has(RamseyGraph.distance(witness[i],witness[j],n)),colour==='red');
    }
  }
  checks++;
}
for(const e of window.RAMSEY_DATA.examples){
  const a=RamseyGraph.analyse(e.order,e.m,e.n,new Set(e.redDistances));
  if(e.order===6)assert.ok(a.blue.count+a.red.count>0);
  else assert.equal(a.blue.count+a.red.count,0,`Invalid preset ${e.order}`);
}
function choose(n,k){let v=1;for(let i=1;i<=k;i++)v=v*(n-i+1)/i;return Math.round(v);}
const allRed=new Set(Array.from({length:28},(_,i)=>i+1));
let a=RamseyGraph.analyse(56,5,6,allRed,4);
assert.equal(a.red.count,choose(56,6));assert.equal(a.blue.count,0);assert.equal(a.red.witnesses.length,4);
a=RamseyGraph.analyse(56,5,6,new Set(),4);
assert.equal(a.blue.count,choose(56,5));assert.equal(a.red.count,0);
console.log(`PASS ${checks} asymmetric counting comparisons, ${window.RAMSEY_DATA.examples.length} presets, witness checks and large complete-colour counts.`);
