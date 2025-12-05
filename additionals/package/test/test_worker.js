import { jcdp, terminateJCDPWorker } from '../dist/index.js';
import { runTests } from './common.js';

runTests('JCDP Worker Tests', jcdp, terminateJCDPWorker);
