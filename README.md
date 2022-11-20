# CG SHOP 2022

Software of the team Shadoks for the the CG:SHOP 2022 challenge [Minimum Partition into Plane Subgraphs](https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2022/#problem-description)

## Installation

Our software does not have any dependencies. To compile our code, simply do

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

If you want to compute an initial solution for an instance, use one of the available: greedy, angle, dsatur, dsaturhull and bad. For instance,

```bash
./build/cgshop --instance instances/rvispecn2615.instance.json --algorithm bad --repetitions 100 --time 3600
```
Note that the instances are not included in this project, you have to download them from the [website](https://cgshop.ibr.cs.tu-bs.de/competition/cg-shop-2022) of the challenge

If you want to improve a solution using the conflict optimizer, it is better to load a JSON file with the parameters. For instance,

```bash
./build/cgshop --parameters parameter_files/2615.json
```

An exemple of a parameters JSON file is:

```json
{
    "instance": "instances/rvispecn2615.instance.json",
    "solution": "rvispecn2615.greedy.20220512-153636.sol.json",
    "algorithm": "conflict",
    "power": 1.2,
    "noise_mean": 1,
    "noise_var": 0.15,
    "max_queue": 1500000, 
    "max_run_time": 10,
    "dfs": true,
    "easy": true,
    "loop": true,
    "loop_time": 3600,
    "power_loop": [1.1, 1.2, 1.3, 1.5, 2.0]
}
```

## Third-party libraries

We use the libraries [rapidJson](https://rapidjson.org/) and [cxxopts](https://github.com/jarro2783/cxxopts). 
