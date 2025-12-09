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

async function getJCDPModule() {
  if (!moduleInstance) {
    console.log('Loading JCDP Module...');
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

const statePtrMap = new Map<number, number>();

export async function jcdpGetStatePtr(handle: number): Promise<number> {
  const existing = statePtrMap.get(handle);
  if (existing) {
    return existing;
  }

  const mod = await getJCDPModule();
  const ptr = mod.ccall('jcdp_get_state_ptr', 'number', ['number'], [handle]);
  if (ptr !== 0) {
    statePtrMap.set(handle, ptr);
  }
  return ptr;
}

export async function jcdpInit(): Promise<number> {
  const mod = await getJCDPModule();
  const handle = mod.ccall('jcdp_init', 'number', [], []);
  jcdpGetStatePtr(handle);
  return handle;
}

function writeStateControl(mod: any, ptr: number, value: number) {
  // StateControl stored at offset 12 bytes; use setValue for clarity.
  mod.setValue(ptr + 12, value, 'i32');
}

export async function jcdpPause(handle: number): Promise<void> {
  const mod = await getJCDPModule();
  const ptr = statePtrMap.get(handle);
  if (ptr) {
    // StateControl::PAUSE = 1.
    writeStateControl(mod, ptr, 1);
  }
}

export async function jcdpResume(handle: number): Promise<void> {
  const mod = await getJCDPModule();
  const ptr = statePtrMap.get(handle);
  if (ptr) {
    // StateControl::RUN = 0.
    writeStateControl(mod, ptr, 0);
  }
}

export async function jcdpCancel(handle: number): Promise<void> {
  const mod = await getJCDPModule();
  const ptr = statePtrMap.get(handle);
  if (ptr) {
    // StateControl::CANCEL = 2.
    writeStateControl(mod, ptr, 2);
  }
}

export interface SolverState {
  visited_leafs: number;
  updated_makespans: number;
  pruned_branches: number;
  runtime_ms: number;
  estimated_search_space: number;
  explored_search_space: number;
  state: number;
  result: SequenceStep[];
}

export async function jcdpGetState(handle: number): Promise<SolverState | null> {
  const mod = await getJCDPModule();

  console.log('jcdpGetState handle:', handle);
  console.log('jcdpGetState PtrMap:', statePtrMap);

  // Fast path: read directly from SolverState memory (non-blocking)
  let ptr = statePtrMap.get(handle);
  console.log('jcdpGetState ptr:', ptr);
  if (!ptr) {
    return null;
  }

  const visited_leafs = mod.getValue(ptr, 'i32');
  const updated_makespans = mod.getValue(ptr + 4, 'i32');
  const pruned_branches = mod.getValue(ptr + 8, 'i32');
  const state = mod.getValue(ptr + 12, 'i32');
  const runtime_ms = mod.getValue(ptr + 16, 'double');
  const estimated_search_space = mod.getValue(ptr + 24, 'double');
  const explored_search_space = mod.getValue(ptr + 32, 'double');
  const resultPtr = mod.getValue(ptr + 40, 'i32');

  let result: SequenceStep[] = [];
  if (resultPtr !== 0 && mod.UTF8ToString) {
    const resultJson = mod.UTF8ToString(resultPtr);
    result = JSON.parse(resultJson) as SequenceStep[];
  }

  return {
    visited_leafs,
    updated_makespans,
    pruned_branches,
    state,
    runtime_ms,
    estimated_search_space,
    explored_search_space,
    result,
  };
}

export async function jcdpSync(
  graph: JCDPGraph | string,
  partial_sequence: SequenceStep[] | string = [],
  options: JCDPOptions = {},
  handle: number = 0
): Promise<number> {
  const mod = await getJCDPModule();

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

  let usedHandle = handle;
  if (usedHandle === 0) {
    usedHandle = await jcdpInit();
  }

  try {
    console.log(`Running JCDP with ${omp_threads} OpenMP threads (handle: ${usedHandle})...`);
    const startTime = performance.now();
    const ret = mod.ccall(
      'jcdp_run_from_json',
      'number',
      [
        'number',
        'string',
        'string',
        'string',
        'string',
        'number',
        'number',
        'number',
        'number',
        'number',
      ],
      [
        usedHandle,
        graph_str,
        sequence_str,
        optimizer,
        scheduler,
        omp_threads,
        available_threads,
        available_memory,
        time_to_solve,
        matrix_free,
      ]
    );
    const endTime = performance.now();
    console.log(`JCDP execution took ${(endTime - startTime).toFixed(2)} ms`);

    if (ret !== 0) {
      throw new Error(`JCDP execution failed with code ${ret}`);
    }

    return usedHandle;
  } catch (e) {
    // Free the result if we created the handle and failed
    if (handle === 0) {
      mod.ccall('jcdp_free_result', 'void', ['number'], [usedHandle]);
    }
    throw e;
  }
}
