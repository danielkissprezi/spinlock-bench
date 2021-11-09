import json

from pprint import pprint

import pandas as pd


with open("result.json", "r") as f:
    data = json.load(f)


benches = pd.DataFrame(data["benchmarks"])
benches["name"] = benches["name"].str.replace("::", "_")
benches[["name", "misc"]] = benches["name"].str.split("/", 1, expand=True)
benches[["misc", "threads"]] = benches["misc"].str.split(":", 1, expand=True)
benches.drop("misc", axis="columns", inplace=True)
benches["threads"] = pd.to_numeric(benches["threads"])

benches = benches[benches.name != "Noop"]

for key in ['real_time', 'cpu_time']:
    data = benches.pivot(
        "threads",
        columns="name",
        values=key,
    ).sort_values(by="threads")
    
    print(f"{key}\n",data)
    data.to_csv(f"result-{key}.csv")
