'use strict';
(() => {
 const $=id=>document.getElementById(id),format=n=>n.toLocaleString('en-GB');
 const rendered=new Map(),widths=new WeakMap();
 function draw(canvas,graph,colour){
  const display=canvas.getBoundingClientRect().width||260,scale=Math.min(window.devicePixelRatio||1,3);
  canvas.width=Math.round(display*scale);canvas.height=canvas.width;rendered.set(canvas,{graph,colour});widths.set(canvas,display);
  const size=canvas.width,ratio=size/display,ctx=canvas.getContext('2d'),N=graph.order,radius=size*.425;
  const pts=Array.from({length:N},(_,i)=>({x:size/2+radius*Math.cos(2*Math.PI*i/N-Math.PI/2),y:size/2+radius*Math.sin(2*Math.PI*i/N-Math.PI/2)}));
  ctx.beginPath();ctx.arc(size/2,size/2,radius*1.055,0,Math.PI*2);ctx.strokeStyle='#dedfd4';ctx.lineWidth=ratio*.5;ctx.stroke();
  ctx.strokeStyle=colour==='blue'?'#2e5e86':'#992d43';ctx.lineWidth=ratio*(N<=17?1.05:N<=35?.72:.45);ctx.beginPath();let edges=0;
  for(let a=0;a<N;a++)for(let b=a+1;b<N;b++)if((graph.distances[RamseyGraph.distance(a,b,N)-1]===0)===(colour==='blue')){ctx.moveTo(pts[a].x,pts[a].y);ctx.lineTo(pts[b].x,pts[b].y);edges++;}
  ctx.stroke();ctx.fillStyle='#3b5360';for(const p of pts){ctx.beginPath();ctx.arc(p.x,p.y,ratio*(N<=17?1.9:1.2),0,Math.PI*2);ctx.fill();}return edges;
 }
 const sub=n=>String(n).replace(/\d/g,d=>'₀₁₂₃₄₅₆₇₈₉'[Number(d)]);
 RAMSEY_DATA.gallery.filter(graph=>graph.order<=51).forEach(graph=>{
  const card=document.createElement('article');card.className='gallery-card';
  const top=document.createElement('div');top.className='gallery-card-top';const count=document.createElement('strong');count.textContent=graph.order;const label=document.createElement('span');label.textContent=`R(${graph.m},${graph.n})`;top.append(count,label);
  const pair=document.createElement('div');pair.className='gallery-pair';const pending=[];
  for(const colour of ['blue','red']){
   const link=document.createElement('a');link.className=`gallery-colour ${colour}`;link.href=`witness.html?m=${graph.m}&n=${graph.n}&colour=${colour}`;link.setAttribute('aria-label',`View the ${colour} graph on ${graph.order} vertices for R(${graph.m},${graph.n})`);
   const caption=document.createElement('span');caption.className='gallery-colour-label';caption.textContent=`${colour==='blue'?'Blue':'Red'} · no K${sub(colour==='blue'?graph.m:graph.n)}`;
   const canvas=document.createElement('canvas');canvas.setAttribute('aria-hidden','true');canvas.dataset.colour=colour;
   const meta=document.createElement('span');meta.className='gallery-edge-count';link.append(caption,canvas,meta);pair.append(link);pending.push({canvas,colour,meta});
  }
  const proof=document.createElement('p');proof.className='gallery-card-proof';proof.textContent=`Together they prove R(${graph.m},${graph.n}) ≥ ${graph.order+1}.`;
  const bottom=document.createElement('a');bottom.className='gallery-card-bottom gallery-open';bottom.href=`witness.html?m=${graph.m}&n=${graph.n}`;bottom.textContent='Open the graph and explore ↗';
  card.append(top,pair,proof,bottom);$('graph-gallery').append(card);
  for(const {canvas,colour,meta} of pending)meta.textContent=`${format(draw(canvas,graph,colour))} edges`;
 });
 const observer=new ResizeObserver(entries=>{for(const entry of entries){const item=rendered.get(entry.target);if(item&&entry.contentRect.width>0&&Math.abs(entry.contentRect.width-widths.get(entry.target))>.5)draw(entry.target,item.graph,item.colour);}});
 rendered.forEach((_,canvas)=>observer.observe(canvas));
})();
