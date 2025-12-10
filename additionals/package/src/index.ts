import { JCDPGraph, JCDPOptions, SequenceStep } from './types.js';
import { WorkerMessage, WorkerResponse } from './worker.js';
import {
  SolverState,
  jcdpPause,
  jcdpResume,
  jcdpCancel,
  jcdpGetState,
  setHEAP32,
  setHEAPF64,
  setStatePtr,
  setHEAP8,
} from './core.js';

// Export types
export type * from './types.js';
export { jcdpSync, jcdpGetState } from './core.js';

// Helper to manage the worker
// We use 'any' for worker to support both Web Worker and Node Worker wrapper
let worker: any = null;
let nextMessageId = 1;
const pendingRequests = new Map<
  number,
  {
    resolve: (value: number | PromiseLike<number>) => void;
    reject: (err: any) => void;
  }
>();

export class JCDPJob implements PromiseLike<void> {
  private _promise: Promise<void>;
  private _handlePromise: Promise<number>;
  public handle: number | null = null;

  constructor(handlePromise: Promise<number>) {
    this._handlePromise = handlePromise;
    this._promise = handlePromise.then(() => undefined);

    // Capture the handle as soon as the worker provides it
    this._handlePromise.then((h) => {
      this.handle = h;
    });
  }

  then<TResult1 = void, TResult2 = never>(
    onfulfilled?: ((value: void) => TResult1 | PromiseLike<TResult1>) | null,
    onrejected?: ((reason: any) => TResult2 | PromiseLike<TResult2>) | null
  ): PromiseLike<TResult1 | TResult2> {
    return this._promise.then(onfulfilled, onrejected);
  }

  private async _getHandle(): Promise<number> {
    const h = await this._handlePromise;
    this.handle = h;
    return h;
  }

  async pause() {
    await jcdpPause(await this._getHandle());
  }

  async resume() {
    await jcdpResume(await this._getHandle());
  }

  async cancel() {
    await jcdpCancel(await this._getHandle());
  }

  async getState(): Promise<SolverState | null> {
    return jcdpGetState(await this._getHandle());
  }
}

async function getWorker(): Promise<any> {
  if (worker) return worker;

  // Use .ts when running via tsx, .js when running compiled output.
  const workerExt = import.meta.url.endsWith('.ts') ? 'ts' : 'js';
  const workerUrl = new URL(`./worker.${workerExt}`, import.meta.url);

  if (typeof Worker !== 'undefined') {
    // Web Environment
    worker = new Worker(workerUrl, {
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
      const workerPath = workerUrl;
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
  const { id, success, error } = data;
  const request = pendingRequests.get(id);

  if (request) {
    if (!success) {
      request.reject(new Error(error));
    } else {
      if (data.type === 'init') {
        if (
          data.HEAP8 &&
          data.HEAP32 &&
          data.HEAPF64 &&
          data.statePtr !== undefined &&
          data.handle !== undefined
        ) {
          setHEAP8(data.handle, data.HEAP8!);
          setHEAP32(data.handle, data.HEAP32!);
          setHEAPF64(data.handle, data.HEAPF64!);
          setStatePtr(data.handle, data.statePtr!);
          request.resolve(data.handle);
        } else {
          request.reject(new Error('Invalid init response from worker'));
        }
      } else {
        // 'done' message; init already resolved the handle.
      }
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
): JCDPJob {
  const handlePromise = new Promise<number>(async (resolve, reject) => {
    try {
      const w = await getWorker();
      const id = nextMessageId++;
      pendingRequests.set(id, { resolve, reject });

      const message: WorkerMessage = {
        id,
        graph,
        partial_sequence,
        options,
      };

      w.postMessage(message);
    } catch (e) {
      reject(e);
    }
  });

  return new JCDPJob(handlePromise);
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
