'use strict';
(() => {
  const $ = id => document.getElementById(id);
  const format = n => n.toLocaleString('en-GB');
  const renderedGraphs=new Map();
  const renderedWidths=new WeakMap();
  function drawBlueGraph(canvas, graph) {
    const displayWidth=canvas.getBoundingClientRect().width||420;
    const pixelRatio=Math.min(window.devicePixelRatio||1,3);
    canvas.width=Math.round(displayWidth*pixelRatio);canvas.height=canvas.width;
    renderedGraphs.set(canvas,graph);renderedWidths.set(canvas,displayWidth);
    const width=canvas.width,scale=width/displayWidth,ctx=canvas.getContext('2d'),n=graph.order,radius=width*.425;
    const points=Array.from({length:n},(_,i)=>({x:width/2+radius*Math.cos(2*Math.PI*i/n-Math.PI/2),y:width/2+radius*Math.sin(2*Math.PI*i/n-Math.PI/2)}));
    ctx.clearRect(0,0,width,width);
    ctx.beginPath();ctx.arc(width/2,width/2,radius*1.055,0,2*Math.PI);ctx.strokeStyle='#dedfd4';ctx.lineWidth=width/800;ctx.stroke();
    let edges=0;
    ctx.strokeStyle='#2e5e86';
    ctx.lineWidth=scale*(n<=17?1.6:n<=35?1.25:1);
    ctx.beginPath();
    for(let a=0;a<n;a++)for(let b=a+1;b<n;b++){
      if(graph.distances[RamseyGraph.distance(a,b,n)-1]===0){ctx.moveTo(points[a].x,points[a].y);ctx.lineTo(points[b].x,points[b].y);edges++;}
    }
    ctx.stroke();
    ctx.fillStyle='#3b5360';
    for(const p of points){ctx.beginPath();ctx.arc(p.x,p.y,scale*(n<=17?2.7:n<=35?2:1.6),0,Math.PI*2);ctx.fill();}
    return edges;
  }
  RAMSEY_DATA.gallery.forEach(graph=>{
    const button=document.createElement('button');button.type='button';button.className='gallery-card';
    button.setAttribute('aria-label',`Enlarge the ${graph.order}-vertex construction for R(${graph.m},${graph.n})`);
    const top=document.createElement('div');top.className='gallery-card-top';
    const count=document.createElement('strong');count.textContent=graph.order;
    const label=document.createElement('span');label.textContent=`R(${graph.m},${graph.n})`;
    top.append(count,label);
    const canvas=document.createElement('canvas');canvas.width=420;canvas.height=420;canvas.setAttribute('aria-hidden','true');
    const bottom=document.createElement('div');bottom.className='gallery-card-bottom';
    const meta=document.createElement('span');
    const arrow=document.createElement('span');arrow.textContent='↗';arrow.setAttribute('aria-hidden','true');bottom.append(meta,arrow);
    const interpretation=document.createElement('p');interpretation.className='gallery-card-proof';interpretation.textContent=`No blue K${graph.m}, no red K${graph.n}. R(${graph.m},${graph.n}) ≥ ${graph.order+1}.`;
    button.append(top,canvas,interpretation,bottom);$('graph-gallery').append(button);
    const edgeCount=drawBlueGraph(canvas,graph);meta.textContent=`${format(edgeCount)} blue edges`;
    button.addEventListener('click',()=>{
      $('gallery-dialog-title').textContent=`${graph.order} vertices · R(${graph.m},${graph.n})`;
      $('gallery-dialog-description').textContent=`The blue graph on ${graph.order} vertices has no clique of size ${graph.m} and no independent set of size ${graph.n}. Its red complement therefore has no clique of size ${graph.n}. Together these properties give a counterexample proving R(${graph.m},${graph.n}) ≥ ${graph.order+1}. This drawing illustrates the certificate; it does not re-run the research checker.`;
      $('gallery-try-example').href=`index.html?example=${graph.order}#explore`;
      $('gallery-canvas').setAttribute('aria-label',`Blue colour class of the construction on ${graph.order} vertices`);
      $('gallery-certificate').href=graph.source;$('gallery-dialog').showModal();drawBlueGraph($('gallery-canvas'),graph);
    });
  });
  $('close-gallery').addEventListener('click',()=>$('gallery-dialog').close());
  $('gallery-dialog').addEventListener('click',e=>{if(e.target===$('gallery-dialog')){const rect=e.target.getBoundingClientRect();if(e.clientX<rect.left||e.clientX>rect.right||e.clientY<rect.top||e.clientY>rect.bottom)e.target.close();}});
  const resizeObserver=new ResizeObserver(entries=>{
    for(const entry of entries){const graph=renderedGraphs.get(entry.target);if(graph&&entry.contentRect.width>0&&Math.abs(entry.contentRect.width-renderedWidths.get(entry.target))>.5)drawBlueGraph(entry.target,graph);}
  });
  document.querySelectorAll('.gallery-card canvas,#gallery-canvas').forEach(canvas=>resizeObserver.observe(canvas));
})();
