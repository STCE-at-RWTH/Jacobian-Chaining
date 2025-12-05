import { jcdpSync } from './core.js';
import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';

// Define the message format
export type WorkerMessage = {
  id: number;
  type: 'run';
  graph: JCDPGraph | string;
  partial_sequence: SequenceStep[] | string;
  options: JCDPOptions;
};

export type WorkerResponse = {
  id: number;
  success: boolean;
  result?: SequenceStep[];
  error?: string;
};

async function handleMessage(data: WorkerMessage, postMessage: (response: WorkerResponse) => void) {
  const { id, type, graph, partial_sequence, options } = data;

  if (type === 'run') {
    try {
      const result = await jcdpSync(graph, partial_sequence, options);
      postMessage({ id, success: true, result });
    } catch (error: any) {
      postMessage({
        id,
        success: false,
        error: error.message || String(error),
      });
    }
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
