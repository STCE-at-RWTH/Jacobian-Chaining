export interface JCDPNode {
  id: string;
  metadata?: {
    vectorSize?: number;
  };
}

export interface JCDPEdge {
  source: string;
  target: string;
  metadata?: {
    tangentCost?: number;
    adjointCost?: number;
    jacobianAccumulated?: boolean;
  };
}

export interface JCDPGraph {
  nodes: JCDPNode[];
  edges: JCDPEdge[];
}

type EliminationType =
  | 'node'
  | 'edge-forward'
  | 'edge-backward'
  | 'face'
  | 'accumulate-edge'
  | 'accumulate-dual-node';
type GFEMethod = 'acc-tan' | 'acc-adj' | 'elim-tan' | 'elim-adj' | 'elim-mul';

export type JCDPSequenceStep = {
  kind: EliminationType;
  method?: GFEMethod;
  fillIn: number;
  cost: number;
  indices: string[];
  threadID?: number;
  startTime?: number;
};

export interface JCDPOptions {
  optimizer?: 'dp' | 'bnb';
  scheduler?: 'list' | 'bnb' | 'none';
  OpenMPThreads?: number;
  availableThreads?: number;
  availableMemory?: number;
  timeToSolve?: number;
  matrixFree?: boolean;
}

export interface JCDPSolverState {
  visited_leafs: number;
  updated_makespans: number;
  pruned_branches: number;
  runtime_ms: number;
  estimated_search_space: number;
  explored_search_space: number;
  state: number;
  result: JCDPSequenceStep[];
}
