import json

from pprint import pprint

import pandas as pd
from matplotlib import pyplot as plt


with open("result.json", "r") as f:
    data = json.load(f)


benches = pd.DataFrame(data["benchmarks"])
benches["name"] = benches["name"].str.replace("::", "_")
benches[["name", "misc"]] = benches["name"].str.split("/", 1, expand=True)
benches[["misc", "threads"]] = benches["misc"].str.split(":", 1, expand=True)
benches.drop("misc", axis="columns", inplace=True)
benches["threads"] = pd.to_numeric(benches["threads"])

benches = benches[benches.name != "Noop"]

data = benches.pivot(
    "threads",
    columns="name",
    values="cpu_time",
).sort_values(by="threads")

print(data)
data.to_csv("result.csv")
data.plot()
plt.show(block=False)
_ = input()
