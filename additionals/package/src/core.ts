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

export async function getJCDPModule() {
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

const statePtrMap = new Map<number, number>();
const HEAP8Map = new Map<number, Uint8Array>();
const HEAP32Map = new Map<number, Uint32Array>();
const HEAPF64Map = new Map<number, Float64Array>();

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

export function setHEAP8(handle: number, HEAP8: Uint8Array) {
  HEAP8Map.set(handle, HEAP8);
}

export function setHEAP32(handle: number, HEAP32: Uint32Array) {
  HEAP32Map.set(handle, HEAP32);
}

export function setHEAPF64(handle: number, HEAPF64: Float64Array) {
  HEAPF64Map.set(handle, HEAPF64);
}

export function setStatePtr(handle: number, ptr: number) {
  statePtrMap.set(handle, ptr);
}

function writeStateControl(HEAP32: Uint32Array, ptr: number, value: number) {
  // StateControl stored at offset 12 bytes.
  HEAP32[(ptr + 12) >> 2] = value;
}

export async function jcdpPause(handle: number): Promise<void> {
  const HEAP32 = HEAP32Map.get(handle);
  const ptr = statePtrMap.get(handle);
  if (HEAP32 && ptr) {
    // StateControl::PAUSE = 1.
    writeStateControl(HEAP32, ptr, 1);
  }
}

export async function jcdpResume(handle: number): Promise<void> {
  const HEAP32 = HEAP32Map.get(handle);
  const ptr = statePtrMap.get(handle);
  if (HEAP32 && ptr) {
    // StateControl::RUN = 0.
    writeStateControl(HEAP32, ptr, 0);
  }
}

export async function jcdpCancel(handle: number): Promise<void> {
  const HEAP32 = HEAP32Map.get(handle);
  const ptr = statePtrMap.get(handle);
  if (HEAP32 && ptr) {
    // StateControl::CANCEL = 2.
    writeStateControl(HEAP32, ptr, 2);
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
  // Fast path: read directly from SolverState memory (non-blocking)
  let ptr = statePtrMap.get(handle);
  if (!ptr) {
    return null;
  }

  const mod = await getJCDPModule();
  let HEAP8 = HEAP8Map.get(handle);
  let HEAP32 = HEAP32Map.get(handle);
  let HEAPF64 = HEAPF64Map.get(handle);
  if (!HEAP8) {
    HEAP8 = mod.HEAP8;
  }
  if (!HEAP32) {
    HEAP32 = mod.HEAP32;
  }
  if (!HEAPF64) {
    HEAPF64 = mod.HEAPF64;
  }
  if (!HEAP8 || !HEAP32 || !HEAPF64) {
    return null;
  }

  const visited_leafs = HEAP32[ptr >> 2];
  const updated_makespans = HEAP32[(ptr + 4) >> 2];
  const pruned_branches = HEAP32[(ptr + 8) >> 2];
  const state = HEAP32[(ptr + 12) >> 2];

  // Double (8 bytes) - requires 8-byte alignment usually, or careful reading.
  // Assuming standard alignment in the struct.
  const runtime_ms = HEAPF64[(ptr + 16) >> 3];
  const estimated_search_space = HEAPF64[(ptr + 24) >> 3];
  const explored_search_space = HEAPF64[(ptr + 32) >> 3];

  // Pointer to result string (Int8)
  const resultPtr = HEAP32[(ptr + 40) >> 2];

  let result: SequenceStep[] = [];
  if (resultPtr !== 0 && mod.UTF8ArrayToString) {
    const resultJson = mod.UTF8ArrayToString(HEAP8, resultPtr);
    try {
      result = JSON.parse(resultJson) as SequenceStep[];
    } catch (e) {
      console.error('Failed to parse result JSON:', e);
    }
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
