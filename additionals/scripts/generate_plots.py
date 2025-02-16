#!/usr/bin/env python3

description = """\
Uses clang-format to format a source file. Applies some pre- and
post-processing to handle OpenMP pragmas correctly."""

help = """\
----------------------------------------------------------------------
Example uses:

  generate_plots.py -t 2 -t 3 -t 4 -t 5 data.csv
    Formats the file in-place.

"""

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import seaborn as sns

# ---------------------------------------------------------------------------- #
# command line options
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description,
    epilog=help,
)

parser.add_argument("args", nargs="*", action="store", help="Files to process")
parser.add_argument(
  '-t', '--threads', action='append', help='Number of')
opts = parser.parse_args()

solvers = [
  "DP",
  "DP_BnB",
  "BnB_List",
  "BnB_BnB",
]

def main():
    df_list = list(pd.read_csv(arg) for arg in opts.args)

    # Box plots over processor amounts for each heuristic combination
    axs = plt.axes()
    df2 = pd.DataFrame()
    for df in df_list:
        for t in opts.threads:
            df2[t] = df["BnB_BnB/" + t] / df["DP/" + t] * 100
            print(str(len(list(filter(lambda x: x >= 100, list(df2[t])))) / len(df2[t])))
            print(str(min(list(df2[t]))))

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
    plt.savefig('statistical_results_combined.png', dpi=1000)
    plt.show()


if __name__ == "__main__":
    main()
