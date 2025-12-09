import { jcdp, terminateJCDPWorker, jcdpGetState } from '../src/index.js';
import { runTests, JCDPFunction } from './common.js';
import { SequenceStep, JCDPOptions } from '../src/types.js';

const jcdpWorkerWrapper: JCDPFunction = async (
  graph: string,
  seq: SequenceStep[],
  options: JCDPOptions
) => {
  const job = jcdp(graph, seq, options);
  console.log('Job started, waiting for handle... ', Date.now());
  console.log('1');
  await new Promise((resolve) => setTimeout(resolve, 500));
  let stats = await job.getState();
  console.log('4');
  console.log('Initial stats:', stats);

  await new Promise((resolve) => setTimeout(resolve, 500));
  await job.pause();
  stats = await job.getState();
  console.log('Stats after pause:', stats);
  job.resume();

  await new Promise((resolve) => setTimeout(resolve, 500));
  stats = await job.getState();
  console.log('Stats after resume:', stats);

  await job;

  stats = await job.getState();
  console.log('Done:', stats);
  return stats ? stats.result : [];
};

// Run common tests then stats test
runTests('JCDP Worker Tests', jcdpWorkerWrapper, terminateJCDPWorker);
