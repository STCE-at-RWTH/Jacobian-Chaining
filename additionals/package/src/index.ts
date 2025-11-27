import { JCDPGraph, JCDPOptions, SequenceStep } from "./types";

const createModule = require("../jcdp/jcdp.js");

let moduleInstance: any = null;
async function getModule() {
  if (!moduleInstance) {
    moduleInstance = await createModule();
  }
  return moduleInstance;
}

// Wrapper to call JCDP from JavaScript/TypeScript. C signature:
// uint32_t jcdp_run_from_json(
//   const char* json_str, const char* optimizer, const char* scheduler,
//   uint32_t threads, uint32_t memory, uint32_t time_to_solve,
//   char* result_buffer)
export async function jcdp(
  input: JCDPGraph | string,
  options: JCDPOptions = {}
): Promise<SequenceStep[]> {
  const mod = await getModule();

  const jsonStr = typeof input === "string" ? input : JSON.stringify(input);
  const optimizer = options.optimizer || "dp";
  const scheduler = options.scheduler || "list";
  const threads = options.threads || 1;
  const memory = options.memory || 0;
  const timeToSolve = options.timeToSolve || 60;

  // Allocate pointer to pointer for result
  const resultPtrPtr = mod._malloc(8);

  try {
    const ret = mod.ccall(
      "jcdp_run_from_json",
      "number",
      ["string", "string", "string", "number", "number", "number", "number"],
      [
        jsonStr,
        optimizer,
        scheduler,
        threads,
        memory,
        timeToSolve,
        resultPtrPtr,
      ]
    );

    if (ret !== 0) {
      throw new Error(`JCDP execution failed with code ${ret}`);
    }

    // Read the result pointer from the pointer-pointer
    const resultPtr = mod.getValue(resultPtrPtr, "i8*");

    if (resultPtr === 0) {
      return []; // Or throw error if result expected
    }

    let resultJson: string = "";
    if (mod.UTF8ToString) {
      resultJson = mod.UTF8ToString(resultPtr);
    }

    return JSON.parse(resultJson) as SequenceStep[];
  } finally {
    mod._free(resultPtrPtr);
  }
}

export * from "./types";
