import json

def translate(fn):
    """Convert a DIMACS file into JSON format."""
    instance = {'edges': 0, 'pairs': []}
    with open('data/' + fn + '.col', 'r') as f:
        for line in f.readlines():
            if line[0] == 'p':
                instance['edges'] = int(line.split(' ')[2])
                instance['number_pairs'] = int(line.split(' ')[3])
            elif line[0] == 'e':
                i = int(line.split(' ')[1])
                j = int(line.split(' ')[2])
                instance['pairs'].append([i, j])
    instance['id'] = fn
    instance['type'] = "Instance_DIMACS"
    if not instance['number_pairs'] == len(instance['pairs']):
        print(f"A problem with {fn}")
    with open(fn + '.col.json', 'w') as f:
        json.dump(instance, f)
    print(f"{fn} converted")


def make_parameters(fn):
    """Make the parameters file."""
    parameters = {
        "instance": f"data/{fn}.col.json",
        "algorithm": "conflict",
        "power": 1.2,
        "noise_mean": 1,
        "noise_var": 0.15,
        "max_queue": 1500000, 
        "max_run_time": 600,
        "dfs": True,
        "easy": True,
        "loop": True,
        "loop_time": 60,
        "power_loop": [1.1, 1.2, 1.3, 1.5, 2.0]
    }
    with open(fn + '.json', 'w') as f:
        json.dump(parameters, f, indent=4)



files = [
    "C2000.5",
    "C4000.5",
    "dsjc1000.1",
    "dsjc1000.5",
    "dsjc1000.9",
    "dsjc250.5",
    "dsjc500.1",
    "dsjc500.5",
    "dsjc500.9",
    "dsjr500.1c",
    "dsjr500.5",
    "flat1000_50_0",
    "flat1000_60_0",
    "flat1000_76_0",
    "flat300_28_0",
    "le450_25c",
    "le450_25d",
    "r1000.1c",
    "r1000.5",
    "r250.5"
]

def write_scripts(files, n):
    """Make `n` scripts with the jobs."""
    lines = []
    for fn in files:
        line = 'echo "%%%%%%%%%% $(date)"; echo; '
        line += f"./cgshop2022 --parameters parameter_files/{fn}.json\n"
        lines.append(line)
    for i in range(n):
        with open(f'jobs-{i}.sh', 'w') as f:
            f.write('#!/bin/bash\n')
            f.write('hostname\n')
            for j in range(len(lines)):
                if j % n == i:
                    f.write(lines[j])

# for fn in files:
    # translate(fn)
    # make_parameters(fn)
    # print(f"./build/cgshop2022 --parameters parameter_files/{fn}.json")


write_scripts(files, 8)