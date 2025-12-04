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

export type EliminationType =
  | 'node'
  | 'edge-forward'
  | 'edge-backward'
  | 'face'
  | 'accumulate-edge'
  | 'accumulate-dual-node';
export type GFEMethod = 'acc-tan' | 'acc-adj' | 'elim-tan' | 'elim-adj' | 'elim-mul';

export type SequenceStep = {
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
