import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';
import createJCDPModule from '../lib/jcdp.js';

// Resolve assets using import.meta.url. Bundlers (Vite/Webpack) will see these,
// bundle the files, and replace these variables with the final public URLs.
const wasmUrl = new URL('../lib/jcdp.wasm', import.meta.url).href;
const jsUrlObj = new URL('../lib/jcdp.js', import.meta.url);
const jsUrl = jsUrlObj.href;

const isNode =
  typeof process !== 'undefined' && process.versions != null && process.versions.node != null;

let moduleInstance: any = null;

async function getModule() {
  if (!moduleInstance) {
    let mainScript = jsUrl;
    if (isNode && jsUrlObj.protocol === 'file:') {
      // In Node.js, we need to provide the mainScript as a file path
      const { fileURLToPath } = await import('url');
      mainScript = fileURLToPath(jsUrlObj);
    }

    moduleInstance = await createJCDPModule({
      locateFile: (path: string) => {
        if (path.endsWith('.wasm')) {
          return wasmUrl;
        }
        if (path.endsWith('.js')) {
          return jsUrl;
        }
        return path;
      },
      mainScriptUrlOrBlob: mainScript,
    });
  }
  return moduleInstance;
}

// Wrapper to call JCDP from JavaScript/TypeScript. C signature:
// int32_t jcdp_run_from_json(
//   const char* json_str, const char* sequence_str, const char* optimizer, const char* scheduler,
//   uint32_t threads, uint32_t available_threads, uint32_t memory, uint32_t time_to_solve,
//   uint32_t matrix_free, int32_t* handle)
// Returns 0 on success, or error code (<0) on failure.
export async function jcdpSync(
  graph: JCDPGraph | string,
  partial_sequence: SequenceStep[] | string = [],
  options: JCDPOptions = {}
): Promise<SequenceStep[]> {
  const mod = await getModule();

  // Prepare input parameters for C function
  const graph_str = typeof graph === 'string' ? graph : JSON.stringify(graph);
  const sequence_str =
    typeof partial_sequence === 'string' ? partial_sequence : JSON.stringify(partial_sequence);
  const optimizer = options.optimizer || 'dp';
  const scheduler = options.scheduler || 'list';
  const omp_threads = options.OpenMPThreads || 1;
  const available_threads = options.availableThreads || 1;
  const available_memory = options.availableMemory || 0;
  const time_to_solve = options.timeToSolve || 60;
  const matrix_free = options.matrixFree ? 1 : 0;

  // Allocate pointer for handle (4 bytes for int32)
  const handlePtr = mod._malloc(4);

  try {
    console.log(`Running JCDP with ${omp_threads} OpenMP threads`);
    const startTime = performance.now();
    const ret = mod.ccall(
      'jcdp_run_from_json',
      'number',
      [
        'string',
        'string',
        'string',
        'string',
        'number',
        'number',
        'number',
        'number',
        'number',
        'number',
      ],
      [
        graph_str,
        sequence_str,
        optimizer,
        scheduler,
        omp_threads,
        available_threads,
        available_memory,
        time_to_solve,
        matrix_free,
        handlePtr,
      ]
    );
    const endTime = performance.now();
    console.log(`JCDP execution took ${(endTime - startTime).toFixed(2)} ms`);

    if (ret !== 0) {
      throw new Error(`JCDP execution failed with code ${ret}`);
    }

    // Retrieve the handle from the buffer
    const handle = mod.getValue(handlePtr, 'i32');

    try {
      // Retrieve the result string using the handle
      const resultPtr = mod.ccall('jcdp_get_result', 'number', ['number'], [handle]);

      if (resultPtr === 0) {
        return [];
      }

      let resultJson: string = '';
      if (mod.UTF8ToString) {
        resultJson = mod.UTF8ToString(resultPtr);
      }

      return JSON.parse(resultJson) as SequenceStep[];
    } finally {
      // Free the result in C++ map
      mod.ccall('jcdp_free_result', 'void', ['number'], [handle]);
    }
  } catch (e) {
    throw e;
  } finally {
    mod._free(handlePtr);
  }
}
