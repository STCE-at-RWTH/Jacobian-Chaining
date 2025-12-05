import { jcdpSync, terminateJCDPWorker } from '../dist/index.js';
import { runTests } from './common.js';

runTests('JCDP Sync Tests', jcdpSync, terminateJCDPWorker);
