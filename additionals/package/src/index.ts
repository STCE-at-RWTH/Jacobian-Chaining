import { JCDPGraph, JCDPOptions, SequenceStep } from "./types";

const createModule = require("../lib/jcdp.js");

// 1. Resolve assets using import.meta.url
// Bundlers (Vite/Webpack) will see these, bundle the files, and replace these variables with the final public URLs.
const wasmUrl = new URL("/lib/jcdp.wasm", import.meta.url).href;
const jsUrl = new URL("../lib/jcdp.js", import.meta.url).href;

let moduleInstance: any = null;

async function getModule() {
  if (!moduleInstance) {
    moduleInstance = await createModule({
      // 2. Tell Emscripten where to find the WASM file
      locateFile: (path: string) => {
        if (path.endsWith(".wasm")) {
          return wasmUrl;
        }
        if (path.endsWith(".js")) {
          return jsUrl;
        }
        return path;
      },
      // 3. Critical for Workers: Tell Emscripten the URL of its own script
      // This fixes the "Unexpected token '<'" error by preventing it from trying to load index.html
      mainScriptUrlOrBlob: jsUrl,
    });
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

  // Prepare input parameters for C function
  const json_str = typeof input === "string" ? input : JSON.stringify(input);
  const optimizer = options.optimizer || "dp";
  const scheduler = options.scheduler || "list";
  const omp_threads = options.OpenMPThreads || 1;
  const available_threads = options.availableThreads || 1;
  const available_memory = options.availableMemory || 0;
  const time_to_solve = options.timeToSolve || 60;
  const matrix_free = options.matrixFree ? 1 : 0;

  // Allocate pointer to pointer for result
  const result_buffer = mod._malloc(8);

  try {
    const ret = mod.ccall(
      "jcdp_run_from_json",
      "number",
      [
        "string",
        "string",
        "string",
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
      ],
      [
        json_str,
        optimizer,
        scheduler,
        omp_threads,
        available_threads,
        available_memory,
        time_to_solve,
        matrix_free,
        result_buffer,
      ]
    );

    if (ret !== 0) {
      throw new Error(`JCDP execution failed with code ${ret}`);
    }

    // Read the result pointer from the pointer-pointer
    const resultPtr = mod.getValue(result_buffer, "i8*");

    if (resultPtr === 0) {
      return [];
    }

    let resultJson: string = "";
    if (mod.UTF8ToString) {
      resultJson = mod.UTF8ToString(resultPtr);
    }

    return JSON.parse(resultJson) as SequenceStep[];
  } finally {
    mod._free(result_buffer);
  }
}

export type * from "./types";
