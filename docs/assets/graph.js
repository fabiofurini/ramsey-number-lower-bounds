/* Small, dependency-free graph primitives. Vertices are zero-indexed internally. */
(() => {
  'use strict';
  const NS = 'http://www.w3.org/2000/svg';
  function svg(tag, attrs = {}, text) {
    const el = document.createElementNS(NS, tag);
    for (const [key, value] of Object.entries(attrs)) el.setAttribute(key, value);
    if (text !== undefined) el.textContent = text;
    return el;
  }
  function distance(a, b, n) { return Math.min(Math.abs(a - b), n - Math.abs(a - b)); }
  function points(n, size, radius) {
    return Array.from({length:n}, (_,i) => ({x:size/2+radius*Math.cos(2*Math.PI*i/n-Math.PI/2),y:size/2+radius*Math.sin(2*Math.PI*i/n-Math.PI/2)}));
  }
  function cliques(n, k, redDistances) {
    const result = {red:[],blue:[]};
    function visit(group, start) {
      if (group.length === k) {
        let colour = null;
        for (let i=0;i<k;i++) for(let j=i+1;j<k;j++) {
          const c = redDistances.has(distance(group[i],group[j],n)) ? 'red' : 'blue';
          if (colour !== null && colour !== c) return;
          colour = c;
        }
        result[colour].push(group.slice());
        return;
      }
      for(let i=start;i<=n-(k-group.length);i++){group.push(i);visit(group,i+1);group.pop();}
    }
    visit([],0);return result;
  }
  // Count exactly without storing every witness: large monochromatic examples
  // may contain millions of target cliques. Only a small sample is needed to
  // highlight them. Growing each clique through common neighbours prunes the
  // incompatible vertex subsets before enumerating their completions.
  function analyse(n, blueTarget, redTarget, redDistances, witnessLimit=24) {
    const adjacency=Array.from({length:n},()=>Array(n).fill(false));
    for(let a=0;a<n;a++)for(let b=a+1;b<n;b++)adjacency[a][b]=adjacency[b][a]=redDistances.has(distance(a,b,n));
    function colourCliques(k,isRed){
      let count=0;
      const witnesses=[],prefix=[];
      const adjacent=(a,b)=>adjacency[a][b]===isRed;
      function combinationsCount(size,choose){let value=1;for(let i=1;i<=choose;i++)value=value*(size-i+1)/i;return Math.round(value);}
      function saveCompletions(vertices,need,start=0){
        if(witnesses.length>=witnessLimit)return;
        if(need===0){witnesses.push(prefix.slice());return;}
        for(let i=start;i<=vertices.length-need&&witnesses.length<witnessLimit;i++){prefix.push(vertices[i]);saveCompletions(vertices,need-1,i+1);prefix.pop();}
      }
      function visit(vertices,need){
        if(vertices.length<need)return;
        if(need===0){count++;if(witnesses.length<witnessLimit)witnesses.push(prefix.slice());return;}
        if(need===1){count+=vertices.length;saveCompletions(vertices,1);return;}
        let complete=true;
        for(let i=0;i<vertices.length&&complete;i++)for(let j=i+1;j<vertices.length;j++)if(!adjacent(vertices[i],vertices[j])){complete=false;break;}
        if(complete){count+=combinationsCount(vertices.length,need);saveCompletions(vertices,need);return;}
        for(let i=0;i<=vertices.length-need;i++){
          const v=vertices[i],next=[];
          for(let j=i+1;j<vertices.length;j++)if(adjacent(v,vertices[j]))next.push(vertices[j]);
          if(next.length<need-1)continue;
          prefix.push(v);visit(next,need-1);prefix.pop();
        }
      }
      visit(Array.from({length:n},(_,i)=>i),k);
      return {count,witnesses};
    }
    return {blue:colourCliques(blueTarget,false),red:colourCliques(redTarget,true)};
  }
  function render(target, {n, redDistances, size=560, interactive=false, witness=[], onDistance=null, hero=false, display='both'}) {
    const pts = points(n,size,hero?size*.39:size*.385);
    target.replaceChildren();
    if(hero){for(const r of [.425,.465])target.append(svg('circle',{cx:size/2,cy:size/2,r:size*r,fill:'none',stroke:'#dddcd1','stroke-width':.7,'stroke-dasharray':r===.465?'2 7':'none'}));}
    if(witness.length)target.append(svg('polygon',{points:witness.map(v=>`${pts[v].x},${pts[v].y}`).join(' '),fill:'#c5a45c','fill-opacity':.12,stroke:'#b3934d','stroke-width':1,'stroke-dasharray':'4 6'}));
    for(let a=0;a<n;a++)for(let b=a+1;b<n;b++){
      const d=distance(a,b,n),red=redDistances.has(d),highlight=witness.includes(a)&&witness.includes(b);
      if(display==='blue'&&red||display==='red'&&!red)continue;
      const attrs={x1:pts[a].x,y1:pts[a].y,x2:pts[b].x,y2:pts[b].y};
      const line=svg('line',{...attrs,stroke:red?'#992d43':'#416b91','stroke-width':highlight?3.6:hero?1.2:n<=6?2.2:1.35,opacity:witness.length?(highlight?1:.13):hero?.62:.65,class:'graph-edge'});
      if(!red&&!hero)line.setAttribute('stroke-dasharray','5 4');
      if(interactive){
        const group=svg('g',{class:'edge-button','data-edge':`${a}-${b}`,tabindex:'0',role:'button','aria-label':`Edge ${a+1}–${b+1}, distance ${d}, ${red?'red':'blue'}. Switch this distance.`});
        group.append(line,svg('line',{...attrs,stroke:'transparent','stroke-width':n<=6?20:9,class:'edge-hit'}));
        group.addEventListener('click',()=>onDistance(d));
        group.addEventListener('keydown',e=>{if(e.key==='Enter'||e.key===' '){e.preventDefault();onDistance(d);}});
        target.append(group);
      }else target.append(line);
    }
    pts.forEach((p,i)=>{
      target.append(svg('circle',{cx:p.x,cy:p.y,r:hero?4.1:n<=6?7:4.5,class:'vertex-dot'}));
      if(!hero){const l=points(n,size,size*.445)[i];target.append(svg('text',{x:l.x,y:l.y+4,'text-anchor':'middle',class:'vertex-label'},i+1));}
    });
  }
  globalThis.RamseyGraph={distance,cliques,analyse,render};
})();
