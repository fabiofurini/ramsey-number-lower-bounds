// The figures and certificates are static. JavaScript adds the interactive viewer.
'use strict';
(() => {
  document.querySelectorAll('#graph-gallery a[data-witness]').forEach(link=>{
    link.href=link.dataset.witness;
    if(link.classList.contains('gallery-open'))link.textContent='Open the graph and explore ↗';
  });
})();
