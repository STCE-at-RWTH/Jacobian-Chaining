import { jcdpSync, jcdpInit, jcdpGetStatePtr, getJCDPModule } from './core.js';
import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';

// Define the message format
export type WorkerMessage = {
  id: number;
  graph: JCDPGraph | string;
  partial_sequence: SequenceStep[] | string;
  options: JCDPOptions;
};

export type WorkerResponse = {
  id: number;
  type: 'init' | 'done';
  success?: boolean;
  handle?: number;
  statePtr?: number;
  HEAP8?: Uint8Array;
  HEAP32?: Uint32Array;
  HEAPF64?: Float64Array;
  error?: string;
};

const activeHandles = new Map<number, number>();

async function handleMessage(data: WorkerMessage, postMessage: (response: WorkerResponse) => void) {
  const { id, graph, partial_sequence, options } = data;
  try {
    const handle = await jcdpInit();
    activeHandles.set(id, handle);

    postMessage({
      id,
      type: 'init',
      success: true,
      handle,
      statePtr: await jcdpGetStatePtr(handle),
      HEAP8: (await getJCDPModule()).HEAP8,
      HEAP32: (await getJCDPModule()).HEAP32,
      HEAPF64: (await getJCDPModule()).HEAPF64,
    });

    const usedHandle = await jcdpSync(graph, partial_sequence, options, handle);
    if (usedHandle !== handle) {
      throw new Error(`jcdpSync returned unexpected handle ${usedHandle}, expected ${handle}`);
    }

    postMessage({ id, type: 'done', success: true });
  } catch (error: any) {
    postMessage({
      id,
      type: 'done',
      success: false,
      error: error.message || String(error),
    });
  } finally {
    activeHandles.delete(id);
  }
}
// Browser / Web Worker environment
if (typeof self !== 'undefined' && typeof self.postMessage === 'function') {
  self.onmessage = (event: MessageEvent<WorkerMessage>) => {
    handleMessage(event.data, (response) => self.postMessage(response));
  };
}

// Node.js Worker Threads environment
if (typeof process !== 'undefined' && process.versions && process.versions.node) {
  import('node:worker_threads')
    .then(({ parentPort }) => {
      if (parentPort) {
        parentPort.on('message', (data: WorkerMessage) => {
          handleMessage(data, (response) => parentPort!.postMessage(response));
        });
      }
    })
    .catch(() => {
      // Ignore if module not found or not in worker thread
    });
}
