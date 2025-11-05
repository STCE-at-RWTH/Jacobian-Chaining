// Demo that calls the JCDP-backed WASM functions via Emscripten runtime

(async () => {
  const out = document.getElementById('out');

   // Ensure auxiliary files (.wasm/.data) are found in the same folder
   const Module = await createJCDPModule({
      locateFile: (p) => p,
      print: (txt) => { out.textContent += txt + '\n'; console.log(txt); },
      printErr: (txt) => { out.textContent += '[err] ' + txt + '\n'; console.error(txt); },
      preRun: [(runtime) => { runtime.ENV.OMP_NUM_THREADS = "8"; }],
   });


   try {
      const runFromConfig = Module.cwrap('jcdp_run_from_config', 'number', ['string']);
      const rc = runFromConfig('/config.txt'); // preloaded from web/config.txt
      out.textContent += `jcdp_run_from_config("/config.txt") -> ${rc}\n`;
   } catch (e) {
      out.textContent += 'Error: ' + (e && e.message ? e.message : String(e)) + '\n';
      console.error(e);
   }
})();
