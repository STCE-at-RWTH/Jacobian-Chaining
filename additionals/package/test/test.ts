import { jcdpGetState, jcdpSync, terminateJCDPWorker } from '../src/index.js';
import { runTests, JCDPFunction } from './common.js';
import { JCDPSequenceStep, JCDPOptions } from '../src/types.js';

const jcdpSyncWrapper: JCDPFunction = async (
  graph: string,
  seq: JCDPSequenceStep[],
  options: JCDPOptions,
) => {
  const handle = await jcdpSync(graph, seq, options);
  const state = await jcdpGetState(handle);
  return state ? state.result : [];
};

runTests('JCDP Sync Tests', jcdpSyncWrapper, terminateJCDPWorker);
