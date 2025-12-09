import { jcdpSync } from './core.js';
import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';

// Define the message format
export type WorkerMessage = {
  id: number;
  graph: JCDPGraph | string;
  partial_sequence: SequenceStep[] | string;
  options: JCDPOptions;
  handle: number;
};

export type WorkerResponse = {
  id: number;
  success: boolean;
  error?: string;
};

const activeHandles = new Map<number, number>();

async function handleMessage(data: WorkerMessage, postMessage: (response: WorkerResponse) => void) {
  const { id, graph, partial_sequence, options, handle } = data;
  try {
    activeHandles.set(id, handle);
    const usedHandle = await jcdpSync(graph, partial_sequence, options, handle);
    if (usedHandle !== handle) {
      throw new Error(`jcdpSync returned unexpected handle ${usedHandle}, expected ${handle}`);
    }

    postMessage({ id, success: true });
  } catch (error: any) {
    postMessage({
      id,
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
