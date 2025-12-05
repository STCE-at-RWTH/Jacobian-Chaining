import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

export const dataRootDir = path.join(__dirname, 'data');

export const testCases = [
  { optimizer: 'dp', scheduler: 'none', expectedFile: 'dp_none.json' },
  { optimizer: 'dp', scheduler: 'list', expectedFile: 'dp_list.json' },
  { optimizer: 'dp', scheduler: 'bnb', expectedFile: 'dp_bnb.json' },
  { optimizer: 'bnb', scheduler: 'list', expectedFile: 'bnb_list.json' },
  { optimizer: 'bnb', scheduler: 'bnb', expectedFile: 'bnb_bnb.json' },
];

export function parseConfig(configPath) {
  const content = fs.readFileSync(configPath, 'utf8');
  const config = {};
  content.split('\n').forEach((line) => {
    const parts = line.trim().split(/\s+/);
    if (parts.length >= 2) {
      const key = parts[0];
      const value = parts[1];
      if (key === 'available_threads') {
        config.availableThreads = parseInt(value, 10);
      }
      if (key === 'available_memory') {
        config.availableMemory = parseInt(value, 10);
      }
      if (key === 'time_to_solve') {
        config.timeToSolve = parseInt(value, 10);
      }
      if (key === 'matrix_free') {
        config.matrixFree = parseInt(value, 10) === 1;
      }
    }
  });
  return config;
}

export async function runTests(testName, jcdpFunction, cleanupFunction = null) {
  try {
    console.log(`Running ${testName} with data files...`);
    let failed = false;

    const dirs = fs
      .readdirSync(dataRootDir)
      .filter((f) => fs.statSync(path.join(dataRootDir, f)).isDirectory());

    for (const dir of dirs) {
      const currentDataDir = path.join(dataRootDir, dir);
      console.log(`\n--- Testing Dataset: ${dir} ---`);

      const configPath = path.join(currentDataDir, 'config.in');
      const config = parseConfig(configPath);
      console.log('Config:', config);

      const chainPath = path.join(currentDataDir, 'chain.json');
      const chainData = fs.readFileSync(chainPath, 'utf8');

      for (const OpenMPThreads of [1, 2, 4]) {
        for (const testCase of testCases) {
          const { optimizer, scheduler, expectedFile } = testCase;
          console.log(
            `\nTesting Optimizer: ${optimizer}, Scheduler: ${scheduler}, OpenMP Threads: ${OpenMPThreads}`,
          );

          const expectedPath = path.join(currentDataDir, expectedFile);
          if (!fs.existsSync(expectedPath)) {
            console.log(`Skipping ${expectedFile} (not found)`);
            continue;
          }
          const expectedData = JSON.parse(fs.readFileSync(expectedPath, 'utf8'));

          const startTime = performance.now();
          const result = await jcdpFunction(chainData, [], {
            optimizer: optimizer,
            scheduler: scheduler,
            OpenMPThreads: OpenMPThreads,
            availableThreads: config.availableThreads,
            availableMemory: config.availableMemory,
            timeToSolve: config.timeToSolve,
            matrixFree: config.matrixFree,
          });
          const endTime = performance.now();
          console.log(`Time: ${(endTime - startTime).toFixed(2)}ms`);

          const resultStr = JSON.stringify(result);
          const expectedStr = JSON.stringify(expectedData);

          if (resultStr === expectedStr) {
            console.log('✅ Passed');
          } else {
            console.error('❌ Failed');
            console.error('Expected:', JSON.stringify(expectedData, null, 2));
            console.error('Actual:', JSON.stringify(result, null, 2));
            failed = true;
          }
        }
      }
    }

    if (cleanupFunction) {
      cleanupFunction();
    }

    if (failed) {
      process.exit(1);
    }
  } catch (error) {
    console.error('Error:', error);
    if (cleanupFunction) {
      cleanupFunction();
    }
    process.exit(1);
  }
}
