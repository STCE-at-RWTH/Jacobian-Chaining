const fs = require('fs');
const path = require('path');
const { jcdp } = require('../dist/index.js');

const dataDir = path.join(__dirname, 'data');
const chainPath = path.join(dataDir, 'chain.json');
const chainData = fs.readFileSync(chainPath, 'utf8');

const testCases = [
  { optimizer: 'dp', scheduler: 'none', expectedFile: 'dp_none.json' },
  { optimizer: 'dp', scheduler: 'list', expectedFile: 'dp_list.json' },
  { optimizer: 'dp', scheduler: 'bnb', expectedFile: 'dp_bnb.json' },
  { optimizer: 'bnb', scheduler: 'list', expectedFile: 'bnb_list.json' },
  { optimizer: 'bnb', scheduler: 'bnb', expectedFile: 'bnb_bnb.json' },
];

async function run() {
  try {
    console.log('Running JCDP Tests with data files...');
    let failed = false;

    for (const testCase of testCases) {
      const { optimizer, scheduler, expectedFile } = testCase;
      console.log(`\nTesting Optimizer: ${optimizer}, Scheduler: ${scheduler}`);

      const expectedPath = path.join(dataDir, expectedFile);
      const expectedData = JSON.parse(fs.readFileSync(expectedPath, 'utf8'));

      const result = await jcdp(chainData, {
        optimizer: optimizer,
        scheduler: scheduler,
        OpenMPThreads: 1,
        availableThreads: 2,
        availableMemory: 0,
        timeToSolve: 60,
        matrixFree: true,
      });

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

    if (failed) {
      process.exit(1);
    }
  } catch (error) {
    console.error('Error:', error);
    process.exit(1);
  }
}

run();
