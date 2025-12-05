import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';
import { WorkerMessage, WorkerResponse } from './worker.js';

// Export types
export type * from './types.js';
export { jcdpSync } from './core.js';

// Helper to manage the worker
// We use 'any' for worker to support both Web Worker and Node Worker wrapper
let worker: any = null;
let nextMessageId = 1;
const pendingRequests = new Map<
  number,
  { resolve: (val: any) => void; reject: (err: any) => void }
>();

async function getWorker(): Promise<any> {
  if (worker) return worker;

  if (typeof Worker !== 'undefined') {
    // Web Environment
    worker = new Worker(new URL('./worker.js', import.meta.url), {
      type: 'module',
    });

    worker.onmessage = (event: MessageEvent<WorkerResponse>) => {
      handleWorkerResponse(event.data);
    };

    worker.onerror = (error: any) => {
      console.error('JCDP Worker Error:', error);
    };
  } else {
    // Node.js Environment
    try {
      const { Worker } = await import('node:worker_threads');
      const workerPath = new URL('./worker.js', import.meta.url);
      const nw = new Worker(workerPath);

      // Adapter to match Web Worker interface partially
      worker = {
        postMessage: (data: any) => nw.postMessage(data),
        terminate: () => nw.terminate(),
      };

      nw.on('message', (data: WorkerResponse) => {
        handleWorkerResponse(data);
      });

      nw.on('error', (error: any) => {
        console.error('JCDP Worker Error:', error);
      });
    } catch (e) {
      console.error('Failed to initialize worker in Node.js environment', e);
      throw e;
    }
  }
  return worker;
}

function handleWorkerResponse(data: WorkerResponse) {
  const { id, success, result, error } = data;
  const request = pendingRequests.get(id);

  if (request) {
    if (success) {
      request.resolve(result);
    } else {
      request.reject(new Error(error));
    }
    pendingRequests.delete(id);
  }
}

/**
 * Runs the JCDP algorithm in a background Web Worker.
 * This prevents blocking the main thread during long calculations.
 */
export function jcdp(
  graph: JCDPGraph | string,
  partial_sequence: SequenceStep[] | string = [],
  options: JCDPOptions = {}
): Promise<SequenceStep[]> {
  return new Promise(async (resolve, reject) => {
    try {
      const w = await getWorker();
      const id = nextMessageId++;

      pendingRequests.set(id, { resolve, reject });

      const message: WorkerMessage = {
        id,
        type: 'run',
        graph,
        partial_sequence,
        options,
      };

      w.postMessage(message);
    } catch (e) {
      reject(e);
    }
  });
}

/**
 * Terminates the background worker.
 * Call this when you are done with JCDP to free up resources.
 */
export function terminateJCDPWorker() {
  if (worker) {
    worker.terminate();
    worker = null;
    pendingRequests.clear();
  }
}
