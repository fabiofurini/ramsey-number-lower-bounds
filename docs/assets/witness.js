'use strict';
(() => {
 const $=id=>document.getElementById(id),params=new URLSearchParams(location.search);
 const graph=RAMSEY_WITNESSES.find(row=>String(row.m)===params.get('m')&&String(row.n)===params.get('n'));
 if(!graph){$('witness-content').hidden=true;$('witness-error').hidden=false;return;}
 const N=graph.order,red=new Set(graph.redDistances),total=N*(N-1)/2;
 const redEdges=graph.redDistances.reduce((sum,d)=>sum+(d===N/2?N/2:N),0);
 const sub=n=>String(n).replace(/\d/g,d=>'₀₁₂₃₄₅₆₇₈₉'[Number(d)]);
 let zoom=1,selected=null;
 if(['blue','red','both'].includes(params.get('colour')))$('witness-edges').value=params.get('colour');
 document.title=`${N}-vertex witness for R(${graph.m},${graph.n}) — Ramsey`;
 $('witness-title').textContent=`${N} vertices. A counterexample.`;
 $('witness-condition').textContent=`No blue K${sub(graph.m)} · No red K${sub(graph.n)}`;
 $('witness-bound').textContent=`R(${graph.m},${graph.n}) ≥ ${N+1}`;
 $('witness-explanation').textContent=`There is no blue clique on ${graph.m} vertices and no red clique on ${graph.n} vertices. This colouring on ${N} vertices therefore proves the lower bound R(${graph.m},${graph.n}) ≥ ${N+1}.`;
 $('witness-counts').textContent=`${N} vertices · ${total-redEdges} blue edges · ${redEdges} red edges`;
 $('witness-distance-count').textContent=`${Math.floor(N/2)} circular distances`;
 $('witness-download').href=`certificates/${graph.certificate}`;$('witness-download').download=graph.certificate.split('/').at(-1);
 $('witness-provenance').textContent=graph.source==='study'?'The drawing uses the exact distance classes of the study’s certificate.':'A classical illustrative representative reconstructed for the website and checked by exhaustive subset enumeration. This is not a new Ramsey result.';
 function setZoom(value){zoom=Math.max(1,Math.min(4,value));$('witness-graph').style.width=`${zoom*100}%`;$('witness-zoom').textContent=`${Math.round(zoom*100)}%`;$('witness-zoom-out').disabled=zoom===1;$('witness-zoom-in').disabled=zoom===4;}
 function draw(){
  const mode=$('witness-edges').value;
  RamseyGraph.render($('witness-graph'),{n:N,redDistances:red,size:900,display:mode});
  $('witness-graph').setAttribute('aria-label',`${mode==='blue'?'Blue graph':mode==='red'?'Red graph':'Both colour classes'} on ${N} vertices for R(${graph.m},${graph.n})`);
  $('witness-graph').classList.toggle('distance-highlight',selected!==null);
  $('witness-graph').querySelectorAll('.graph-edge').forEach(edge=>edge.classList.toggle('selected-distance',Number(edge.dataset.distance)===selected));
  $('witness-distances').querySelectorAll('button').forEach(button=>button.setAttribute('aria-pressed',String(Number(button.dataset.distance)===selected)));
  $('witness-distance-info').textContent=selected===null?'Choose a distance to highlight its edges.':`Distance ${selected}: ${selected===N/2?N/2:N} ${red.has(selected)?'red':'blue'} edges highlighted. Select it again to show the whole graph.`;
  $('witness-visible-edges').textContent=`${mode==='blue'?total-redEdges:mode==='red'?redEdges:total} edges shown`;
  setZoom(zoom);
 }
 for(let d=1;d<=Math.floor(N/2);d++){
  const button=document.createElement('button');button.type='button';button.className=`distance-button ${red.has(d)?'red':'blue'}`;button.dataset.distance=d;button.textContent=d;button.setAttribute('aria-label',`Highlight distance ${d}, ${red.has(d)?'red':'blue'}`);
  button.addEventListener('click',()=>{selected=selected===d?null:d;if(selected!==null)$('witness-edges').value=red.has(d)?'red':'blue';draw();});$('witness-distances').append(button);
 }
 $('witness-edges').addEventListener('change',()=>{selected=null;draw();});
 $('witness-zoom-in').addEventListener('click',()=>setZoom(zoom+.5));$('witness-zoom-out').addEventListener('click',()=>setZoom(zoom-.5));
 $('witness-reset-view').addEventListener('click',()=>{selected=null;$('witness-edges').value='blue';setZoom(1);draw();});
 function downloadImage(original,colour){
  const copy=original.cloneNode(true),elements=original.querySelectorAll('*'),clones=copy.querySelectorAll('*');
  elements.forEach((element,i)=>{const style=getComputedStyle(element);for(const property of ['fill','stroke','stroke-width','stroke-dasharray','opacity','font-size','font-family'])clones[i].style.setProperty(property,style.getPropertyValue(property));});
  copy.removeAttribute('id');copy.setAttribute('xmlns','http://www.w3.org/2000/svg');copy.setAttribute('width','900');copy.setAttribute('height','900');copy.style.width='900px';copy.style.background='#fffefa';const background=document.createElementNS('http://www.w3.org/2000/svg','rect');background.setAttribute('width','900');background.setAttribute('height','900');background.setAttribute('fill','#fffefa');copy.prepend(background);
  const url=URL.createObjectURL(new Blob([new XMLSerializer().serializeToString(copy)],{type:'image/svg+xml'}));
  const link=document.createElement('a');link.href=url;link.download=`R_${graph.m}_${graph.n}_${N}_vertices_${colour}.svg`;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
 }
 $('witness-image').addEventListener('click',()=>downloadImage($('witness-graph'),$('witness-edges').value));
 $('witness-red-image').addEventListener('click',()=>downloadImage($('witness-red-graph'),'red'));
 $('witness-red-condition').textContent=`No red clique on ${graph.n} vertices.`;
 $('witness-red-summary').textContent=`${N} vertices · ${redEdges} red edges. Every edge missing from the blue graph belongs to this red graph.`;
 RamseyGraph.render($('witness-red-graph'),{n:N,redDistances:red,size:900,display:'red'});
 $('witness-red-graph').setAttribute('aria-label',`Red complement on ${N} vertices, avoiding cliques of size ${graph.n}`);
 draw();
})();
