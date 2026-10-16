'use strict';
(() => {
  const button=document.querySelector('.menu-toggle');
  const navigation=document.getElementById('navigation');
  function close(){navigation.classList.remove('open');button.setAttribute('aria-expanded','false');}
  button.addEventListener('click',()=>{button.setAttribute('aria-expanded',String(navigation.classList.toggle('open')));});
  navigation.querySelectorAll('a').forEach(a=>a.addEventListener('click',close));
  document.addEventListener('keydown',e=>{if(e.key==='Escape')close();});
  button.hidden=false;
})();
