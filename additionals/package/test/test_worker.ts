import { jcdp, terminateJCDPWorker, jcdpGetState } from '../src/index.js';
import { runTests, JCDPFunction } from './common.js';
import { JCDPSequenceStep, JCDPOptions } from '../src/types.js';

const jcdpWorkerWrapper: JCDPFunction = async (
  graph: string,
  seq: JCDPSequenceStep[],
  options: JCDPOptions,
) => {
  const job = jcdp(graph, seq, options);
  await new Promise((resolve) => setTimeout(resolve, 100).unref());
  let stats = await job.getState();
  console.log('Initial stats:', stats?.state);

  await new Promise((resolve) => setTimeout(resolve, 100).unref());
  await job.pause();
  stats = await job.getState();
  console.log('Stats after pause:', stats?.state);

  await new Promise((resolve) => setTimeout(resolve, 100).unref());
  stats = await job.getState();
  console.log('Stats before resume:', stats?.state);
  job.resume();
  stats = await job.getState();
  console.log('Stats after resume:', stats?.state);

  await job;

  stats = await job.getState();
  console.log('Done:', stats?.state);
  return stats ? stats.result : [];
};

// Run common tests then stats test
runTests('JCDP Worker Tests', jcdpWorkerWrapper, terminateJCDPWorker);
