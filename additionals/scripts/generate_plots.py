#!/usr/bin/env python3

description = """\
Uses clang-format to format a source file. Applies some pre- and
post-processing to handle OpenMP pragmas correctly."""

help = """\
----------------------------------------------------------------------
Example uses:

  generate_plots.py data.csv
    Formats the file in-place.

"""

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import seaborn as sns
import numpy as np

# ---------------------------------------------------------------------------- #
# command line options
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description,
    epilog=help,
)

parser.add_argument("args", nargs="*", action="store", help="Files to process")
opts = parser.parse_args()

solvers = [
  "DP",
  "DP_BnB",
  "BnB_List",
  "BnB_BnB",
]

processors = ["1", "2", "3"]

heuristic_labels = {
  "GreedyMinCost": "GreedyMinCost",
  "GreedyEdgeMinFill": "GreedyEdgeMinFill",
  "GreedyJacobianMinFill": "GreedyEdgeMinFill",
  "SparseTangentAdjoint": "Sparse Tangent / Adjoint",
  "BranchAndBoundScheduler": "Branch and Bound",
  "GrahamsListScheduler": "Graham's List",
  "MinIdleTimeScheduler": "Greedy Online Scheduling",
  "Minimum": "Overall minimum",
}

def main():
    df_list = list(pd.read_csv(arg) for arg in opts.args)

    # Box plots over processor amounts for each heuristic combination
    reference = "BnB_BnB/"
    axs = plt.axes()

    df2 = pd.DataFrame()
    for df in df_list:
        for p in processors:
            label = "DP/"
            df2[p] = df[reference + p] / df[label + p] * 100
            print(label + " " + str(len(list(filter(lambda x: x >= 100, list(df2[p])))) / len(df2[p])))
            print(label + " " + str(min(list(df2[p]))))

    axs.yaxis.set_major_formatter(mtick.PercentFormatter())
    axs.set(ylim=(50, 105))
    axs.axhline(100, color=".3", dashes=(2, 2))

    sns.boxplot(
        # df2[df["finished"] == 1],
        df2,
        whis=[5, 95],
        width=0.5,
        boxprops={"facecolor": (0.3, 0.5, 0.7, 0.5)},
        flierprops={"marker": "x"},
        medianprops={"color": "orange", "linewidth": 1.5},
        ax=axs,
    )
    plt.gca().set_xlabel("threads")

    # dfa = pd.DataFrame()
    # dfb = pd.DataFrame()
    # dfc = pd.DataFrame()
    # for p in processors:
    #     dfa[p] = df["None/bound/" + p] / df["None/discovered/" + p] * 100
    #     dfa["Lower Bound"] = "None"
    #     dfa = dfa.dropna()
    #     dfb[p] = df["SimpleMinAccCostBound/bound/" + p] / df["SimpleMinAccCostBound/discovered/" + p] * 100
    #     dfb["Lower Bound"] = "MinAccCost"
    #     dfb = dfb.dropna()
    #     dfc[p] = df["SimpleMinAccCostBound/lowerbound/" + p] / df["SimpleMinAccCostBound/bound/" + p] * 100
    #     print(np.min(dfc[p]))
    #     # dfc[p] = dfb[p] / dfa[p] * 100
    #     dfc["Lower Bound"] = "MinAccCost Pruning Ratio"
    #     dfc = dfc.dropna()
    #     # print(np.average(df2[p]) - np.average(df3[p]))
    #     # t = (np.average(df3[p]) - np.average(df2[p])) / np.sqrt(np.var(df2[p])/len(df2[p]) + np.var(df3[p])/len(df3[p]))
    #     # print(p + ": " + str(t))

    # df2 = pd.concat([dfa, dfb, dfc])
    # df2 = pd.melt(df2, id_vars='Lower Bound', value_vars=processors)

    # sns.boxplot(
    #     df,
    #     x="variable",
    #     y="value",
    #     hue="Lower Bound",
    #     whis=[5, 95],
    #     width=0.5,
    #     flierprops={"marker": "x"},
    #     medianprops={"linewidth": 1.5},
    # )
    # plt.gcf().set_size_inches(7, 6)
    # plt.gca().set_xlabel("machines")
    # plt.gca().set_ylabel("")
    # plt.gca().yaxis.set_major_formatter(mtick.PercentFormatter())
    # plt.gca().set_title("Pruned Branches / Discovered Branches")
    # plt.ylim(0, 120)
    # plt.gca().axhline(100, color=".3", dashes=(2, 2))

    # Desity plots over all heuristc combinations for each processor amount
    # for p in processors:
    #     plt.figure()
    #     df2 = pd.DataFrame()
    #     for opt in optimizers:
    #         for sched in schedulers:
    #             label = opt + "/" + sched + "/" + p
    #             df2[opt + "/" + sched] = df[reference + p] / df[label] * 100

    #     sns.kdeplot(df2[df.finished == 1], clip=(0.0, 100.0))
    #     plt.title("Machines: " + p)

    plt.savefig('statistical_results_combined.png', dpi=1000)
    plt.show()


if __name__ == "__main__":
    main()
