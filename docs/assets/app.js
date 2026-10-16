'use strict';
const linearDistance = document.body.dataset.colouring === 'linear';
const metric = linearDistance ? 'linear' : 'circular';
const classCount = n => linearDistance ? n-1 : Math.floor(n/2);
const liftDistances = (n,red) => Array.from({length:n-1},(_,i)=>i+1).filter(d=>red.includes(Math.min(d,n-d)));
const demoData = linearDistance ? {...RAMSEY_DATA, examples:RAMSEY_DATA.examples.map(e=>({...e,redDistances:liftDistances(e.order,e.redDistances),description:e.description+' Shown as a linear-distance colouring; distances d and N−d can now be edited independently.'}))} : RAMSEY_DATA;
const reference = new Set(linearDistance ? liftDistances(17,RAMSEY_DATA.small.redDistances) : RAMSEY_DATA.small.redDistances);
const drawGraph = (target,options) => RamseyGraph.render(target,{...options,layout:metric});
drawGraph(document.getElementById('hero-graph'),{n:17,redDistances:reference,size:500,hero:true});

(() => {
  const $ = id => document.getElementById(id);
  const format = n => n.toLocaleString('en-GB');
  const configurations=Object.fromEntries(demoData.examples.map(e=>[String(e.order),{n:e.order,blueTarget:e.m,redTarget:e.n,red:e.redDistances,description:e.description}]));
  const subscript=n=>String(n).replace(/\d/g,d=>'₀₁₂₃₄₅₆₇₈₉'[Number(d)]);
  demoData.examples.forEach(e=>{
    const option=document.createElement('option');option.value=String(e.order);option.textContent=`R(${e.m},${e.n}) · ${e.order} vertices`;$('example-select').append(option);
    const button=document.createElement('button');button.type='button';button.className='example-shortcut';button.dataset.order=String(e.order);button.setAttribute('aria-label',`Edit ${e.order} vertices for R(${e.m},${e.n})`);
    const count=document.createElement('strong');count.textContent=e.order;
    const target=document.createElement('span');target.textContent=`R(${e.m},${e.n})`;
    button.append(count,target);button.addEventListener('click',()=>selectExample(String(e.order)));$('example-shortcuts').append(button);
  });
  let key = new URLSearchParams(location.search).get('example') || '17';
  if(!configurations[key])key='17';
  $('edge-display').value=configurations[key].n>17?'blue':'both';
  $('example-select').value=key;
  let red = new Set(configurations[key].red);
  let analysis=null;
  let witness = [];
  let witnesses = [];
  let witnessIndex = -1;
  function drawExample() {
    const c = configurations[key];
    if(!analysis)analysis=RamseyGraph.analyse(c.n,c.blueTarget,c.redTarget,red,24,metric);
    const found=analysis;
    witnesses = [...found.blue.witnesses,...found.red.witnesses];
    const total=found.blue.count+found.red.count;
    $('example-description').textContent = c.description;
    $('circulant-summary').textContent = `${c.n} vertices · ${format(c.n*(c.n-1)/2)} edges · just ${classCount(c.n)} distance colours. Avoid blue K${subscript(c.blueTarget)} and red K${subscript(c.redTarget)} to prove R(${c.blueTarget},${c.redTarget}) ≥ ${c.n+1}.`;
    $('blue-count').textContent = format(found.blue.count);
    $('red-count').textContent = format(found.red.count);
    $('blue-count-label').textContent = `blue K${subscript(c.blueTarget)} cliques`;
    $('red-count-label').textContent = `red K${subscript(c.redTarget)} cliques`;
    $('explorer-status').classList.toggle('invalid',total>0);
    $('explorer-status').textContent = total===0
      ? `Valid (${c.blueTarget},${c.redTarget})-colouring: this construction proves R(${c.blueTarget},${c.redTarget}) ≥ ${c.n+1}.`
      : `Found ${format(total)} forbidden cliques: blue K${subscript(c.blueTarget)} or red K${subscript(c.redTarget)}. This colouring does not establish the desired lower bound.`;
    $('show-clique').disabled = total===0;
    $('graph-description').textContent = `${c.n} VERTICES · ${c.n*(c.n-1)/2} EDGES`;
    $('graph-summary').textContent = `${classCount(c.n)} ${linearDistance?'linear':'circular'} distance classes`;
    // Preserve the keyboard focus on a distance control when it is reconstructed.
    const active = document.activeElement;
    const focusDistance = active && active.dataset ? active.dataset.distance : null;
    const focusEdge=active&&active.dataset?active.dataset.edge:null;
    $('example-shortcuts').querySelectorAll('button').forEach(button=>button.setAttribute('aria-pressed',String(button.dataset.order===key)));
    const controls = $('distance-controls');
    controls.replaceChildren();
    for(let d=1;d<=classCount(c.n);d++){
      const button = document.createElement('button');
      button.type='button';
      button.className=`distance-button ${red.has(d)?'red':'blue'}`;
      button.textContent=`${d}`;
      button.dataset.distance=String(d);
      button.setAttribute('aria-label',`Distance ${d}: ${red.has(d)?'red':'blue'}. Switch colour.`);
      button.addEventListener('click',()=>flip(d));
      controls.append(button);
    }
    drawGraph($('interactive-graph'),{n:c.n,redDistances:red,interactive:true,witness,onDistance:flip,display:$('edge-display').value});
    if(focusDistance)controls.querySelector(`[data-distance="${focusDistance}"]`)?.focus({preventScroll:true});
    else if(focusEdge)$('interactive-graph').querySelector(`[data-edge="${focusEdge}"]`)?.focus({preventScroll:true});
  }
  function flip(d){red.has(d)?red.delete(d):red.add(d);analysis=null;witness=[];witnessIndex=-1;drawExample();}
  function selectExample(value){key=value;$('example-select').value=key;$('edge-display').value=configurations[key].n>17?'blue':'both';red=new Set(configurations[key].red);analysis=null;witness=[];witnessIndex=-1;drawExample();}
  $('example-select').addEventListener('change',e=>selectExample(e.target.value));
  $('edge-display').addEventListener('change',drawExample);
  $('reset-example').addEventListener('click',()=>{red=new Set(configurations[key].red);analysis=null;witness=[];witnessIndex=-1;drawExample();});
  $('show-clique').addEventListener('click',()=>{
    if(!witnesses.length)return;
    witnessIndex=(witnessIndex+1)%witnesses.length;
    witness=witnesses[witnessIndex];
    $('edge-display').value=red.has(linearDistance?Math.abs(witness[0]-witness[1]):RamseyGraph.distance(witness[0],witness[1],configurations[key].n))?'red':'blue';
    drawExample();
    const c=configurations[key];
    $('explorer-status').textContent=`Highlighted clique on vertices ${witness.map(v=>linearDistance?v:v+1).join(', ')}. All its edges have the same colour. Total forbidden cliques: ${format(analysis.blue.count+analysis.red.count)}.`;
  });
  drawExample();

  let builderOrder=17;
  const builderDistances=new Set(Array.from({length:classCount(17)},(_,i)=>i+1).filter(distance=>!reference.has(distance)));
  function drawBuilder(){
    const maximum=classCount(builderOrder);
    for(const distance of builderDistances)if(distance>maximum)builderDistances.delete(distance);
    $('builder-order-value').textContent=builderOrder;
    const active=document.activeElement?.dataset.builderDistance;
    const controls=$('builder-distances');controls.replaceChildren();
    for(let distance=1;distance<=maximum;distance++){
      const button=document.createElement('button');button.type='button';button.className='builder-distance';button.textContent=distance;button.dataset.builderDistance=distance;
      button.setAttribute('aria-pressed',String(builderDistances.has(distance)));
      button.setAttribute('aria-label',`Distance ${distance}: ${builderDistances.has(distance)?'included':'excluded'}. Toggle this class.`);
      button.addEventListener('click',()=>{builderDistances.has(distance)?builderDistances.delete(distance):builderDistances.add(distance);drawBuilder();});controls.append(button);
    }
    const distances=[...builderDistances].sort((a,b)=>a-b),edges=distances.reduce((sum,distance)=>sum+(linearDistance?builderOrder-distance:distance===builderOrder/2?builderOrder/2:builderOrder),0);
    $('builder-summary').textContent=`Selected distances: ${distances.join(', ')||'none'}. ${builderOrder} vertices and ${edges} edges, determined by just ${maximum} distance choices.`;
    $('builder-caption').textContent=`${builderOrder} vertices · ${edges} blue edges · ${distances.length} selected distance classes`;
    const absent=new Set(Array.from({length:maximum},(_,i)=>i+1).filter(distance=>!builderDistances.has(distance)));
    drawGraph($('builder-graph'),{n:builderOrder,redDistances:absent,display:'blue'});
    if(active)controls.querySelector(`[data-builder-distance="${active}"]`)?.focus({preventScroll:true});
  }
  $('builder-order').addEventListener('input',event=>{builderOrder=Number(event.target.value);drawBuilder();});
  $('builder-use').addEventListener('click',()=>{
    const absent=Array.from({length:classCount(builderOrder)},(_,i)=>i+1).filter(distance=>!builderDistances.has(distance));
    configurations.custom={n:builderOrder,blueTarget:4,redTarget:4,red:absent,description:`Your ${linearDistance?'linear-distance':'circulant'} graph on ${builderOrder} vertices is blue; its complement is red. Both colours are checked for cliques on four vertices.`};
    let option=$('example-select').querySelector('option[value="custom"]');
    if(!option){option=document.createElement('option');option.value='custom';$('example-select').append(option);}
    option.textContent=`Your graph · ${builderOrder} vertices · R(4,4)`;
    selectExample('custom');document.getElementById('colouring').scrollIntoView({behavior:'smooth',block:'start'});
  });
  drawBuilder();

})();
