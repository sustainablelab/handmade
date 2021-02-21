import json
with open("lib-tags.json") as fin:
    parsed = json.load(fin)
    with open("lib-tags.txt", mode="w") as fout:
        for include in parsed["Data"]["Includes"]:
            fout.write(include)
            fout.write("\n")
