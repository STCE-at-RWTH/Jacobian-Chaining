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

export type GFEMethod =
  | "acc-tan"
  | "acc-adj"
  | "elim-tan"
  | "elim-adj"
  | "elim-mul";

export interface SequenceStep {
  kind: "face";
  method: GFEMethod;
  indices: string[];
}

export interface JCDPOptions {
  optimizer?: "dp" | "bnb";
  scheduler?: "list" | "bnb";
  threads?: number;
  memory?: number;
  timeToSolve?: number;
}
