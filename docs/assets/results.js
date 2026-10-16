'use strict';
(() => {
  const $ = id => document.getElementById(id);
  const format = n => n.toLocaleString('en-GB');
  const rows=RAMSEY_DATA.results;
  for(const row of rows){
    const option=document.createElement('option');option.value=row.n;option.textContent=`R(3,${row.n}) · ${row.order} vertices`;
    $('result-select').append(option);
  }
  $('result-select').value='37';
  function drawResult(){
    const row=rows.find(x=>x.n===Number($('result-select').value));
    $('result-formula').textContent=`R(3,${row.n}) ≥ ${row.bound}`;
    $('result-explanation').textContent=`A colouring on ${row.order} vertices with no blue triangle and no red clique of order ${row.n}. Its existence proves that R(3,${row.n}) is at least ${row.bound}.`;
    $('result-order').textContent=format(row.order);
    $('result-edges').textContent=format(row.order*(row.order-1)/2);
    $('result-distances').textContent=format(row.distances.length);
    $('certificate-link').href=`certificates/${row.certificate}`;
    $('certificate-link').setAttribute('download',row.certificate);
    $('red-target').textContent=row.n;
    $('distance-total').textContent=`${row.distances.length} DISTANCES`;
    $('distance-info').textContent='Each cell is a circular distance. Select one to inspect its role.';
    const grid=$('result-grid');grid.replaceChildren();
    row.distances.forEach((colour,i)=>{
      const d=i+1,cell=document.createElement('button');cell.type='button';cell.className=`distance-cell ${colour?'red':'blue'}`;
      cell.textContent=d;cell.title=`Distance ${d}: ${colour?'red':'blue'}`;
      cell.setAttribute('aria-label',cell.title);cell.setAttribute('aria-pressed','false');
      cell.addEventListener('click',()=>{
        grid.querySelectorAll('[aria-pressed="true"]').forEach(el=>el.setAttribute('aria-pressed','false'));
        cell.setAttribute('aria-pressed','true');
        const edges=d===row.order/2?row.order/2:row.order;
        $('distance-info').textContent=`Distance ${d}: ${colour?'red':'blue'}. This class represents ${format(edges)} edges, all ${colour?'red':'blue'}, in the graph on ${row.order} vertices.`;
      });grid.append(cell);
    });
  }
  $('result-select').addEventListener('change',drawResult);drawResult();
  const tableRows=[...$('results-table').querySelectorAll('tr')];
  tableRows.forEach((tr,i)=>{tr.hidden=i>=6;});
  $('expand-results').hidden=false;
  $('result-explorer').hidden=false;
  function setExpanded(expanded){
    tableRows.forEach((tr,i)=>{tr.hidden=!expanded&&i>=6;});
    $('expand-results').setAttribute('aria-expanded',String(expanded));
    $('expand-results').textContent=expanded?'Show fewer ↑':'Show all 25 results ↓';
  }
  $('browse-results').addEventListener('click',()=>setExpanded(true));
  if(location.hash==='#paper-results')setExpanded(true);
  $('expand-results').addEventListener('click',()=>{
    const expanded=$('expand-results').getAttribute('aria-expanded')==='true';
    setExpanded(!expanded);
  });
})();
