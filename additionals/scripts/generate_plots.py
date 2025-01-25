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

optimizers = [
  "GreedyMinCost",
  "GreedyEdgeMinFill",
  "GreedyJacobianMinFill",
  "SparseTangentAdjoint",
  "Minimum",
]

schedulers = [
    "BranchAndBoundScheduler",
    # "GrahamsListScheduler",
    "MinIdleTimeScheduler",
]

# processors = ["5", "6", "7", "8", "9", "10"]
processors = ["2", "3", "4", "5"]

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

    # Only use the minimum of SparseTangent and SparseAdjoint
    # for sched in schedulers:
    #     for p in processors:
    #         df["SparseTangentAdjoint/" + sched + "/" + p] = np.minimum(
    #             df["SparseTangent/" + sched + "/" + p],
    #             df["SparseAdjoint/" + sched + "/" + p],
    #         )
    #         df["Minimum/" + sched + "/" + p] = df["SparseTangentAdjoint/" + sched + "/" + p]
    #         df["Minimum/" + sched + "/" + p] = np.minimum(
    #             df["GreedyMinCost/" + sched + "/" + p],
    #             df["Minimum/" + sched + "/" + p]
    #         )
    #         df["Minimum/" + sched + "/" + p] = np.minimum(
    #             df["GreedyEdgeMinFill/" + sched + "/" + p],
    #             df["Minimum/" + sched + "/" + p]
    #         )

    # Box plots over processor amounts for each heuristic combination
    reference = "BranchAndBound/BranchAndBoundScheduler/"
    axs = plt.axes()
    # fig.set_size_inches(9, 3)
    # for i, opt in enumerate(optimizers):
    #     axs[i, 0].set_ylabel(heuristic_labels[opt], fontsize='large')
    #     for j, sched in enumerate(schedulers):
    #         axs[0, j].set_title(heuristic_labels[sched])

            # if j < len(schedulers) - 1:
            #     if j == 0:
            #         axs[i, j].set_yticks([])
            #     else:
            #         axs[i, j].yaxis.set_visible(False)
            # else:
            #     axs[i, j].yaxis.set_label_position("right")
            #     axs[i, j].yaxis.tick_right()

    df2 = pd.DataFrame()
    for i, df in enumerate(df_list):
        label = "dp"
        df2[processors[i]] = df[reference + processors[i]] / df[label] * 100
        # if any(df2[p] < 25):
        #     print(label)
        #     print(df2[p][df2[p] < 25])
        print(label + " " + str(len(list(filter(lambda x: x >= 100, list(df2[processors[i]])))) / len(df2[processors[i]])))
        print(label + " " + str(min(list(df2[processors[i]]))))

    axs.yaxis.set_major_formatter(mtick.PercentFormatter())
    axs.set(ylim=(50, 105))
    axs.axhline(100, color=".3", dashes=(2, 2))

    sns.boxplot(
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
