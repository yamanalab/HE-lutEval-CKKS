# HE-lutEval-CKKS

## Contents
- [HE-lutEval-CKKS](#he-luteval-ckks)
  - [Contents](#contents)
  - [Introduction](#introduction)
  - [Prerequisites](#prerequisites)
  - [Building on Linux](#building-on-linux)
  - [How to us](#how-to-us)
    - [Code construciton](#code-construciton)
    - [Test Examples](#test-examples)
  - [License](#license)


## Introduction
This repository provides a demo of the experimental code used to evaluate functions using CKKS-based lookup tables (LUTs) for the paper:\
H. Zhu, T. Suzuki, H. Yamana, "Private Multivariate Function Evaluation using CKKS-based Homomorphic Encrypted LookUp Tables", In Proceedings of 12th International Conference on Information Systems Security and Privacy (ICISSP 2026).<br>
For testing, you have to change the parameters by yourself. I'm sorry for the inconvenience.


## Prerequisites
- [OpenFHE 1.3.1](https://github.com/openfheorg/openfhe-development)
- [CMake 3.28.1](https://cmake.org/)
- [g++ 9.4.0](https://ftp.gnu.org/gnu/gcc/gcc-9.4.0/)
  
## Building on Linux
```
cd HE-lutEval-CKKS
mkdir build
cd build
cmake .. 
make -j
```

## How to us
1. Change the maximum and minimum of tables in util.cpp <br>
```
double Lutxmax=d;
double Lutxmin=e; // please change d and e to change the maximum and minimum of input table
```
2. To run the demo code, run the commands like below.<br>
   Please change d to change the number of data points in each row of LUTs, ($l_{used}=2^d$). Please change testNum to change the number of test number.
   
    Step1: Generate tables and store in the `Key` file
    ```
    ./bin/gen_keys -N 65536 -L 23 --q0 50 --qi 40 --ql 60 --type oneInput  --bitNum b 
    ```
    Step2: Generate keys and store in the `Table` file
    ```
    ./bin/gen_tables --type oneInput --bitNum b 
    ```
    Step3: LUT evaluation (single-thread execution)
    ```
    OMP_NUM_THREAD=1 ./bin/main_oneinput --n 4 --d 4 --test {testNum} --bitNum {b}
    ```

### Code construciton
```
HE_LUT_CKKS/
├── bin/                  # Generated executable files
│
├── data/             # Store genrated keys and tables
│   ├── Key/
│   └── Table/
│
├── include/              # Library header files
│   ├── proposedMethod/
│   │   ├── comparision.h 
│   │   ├── generateLut.h 
│   │   ├── utils.h 
│   ├── cmdline.h
│   ├── common.h
│   ├── picojson.h
│   ├── cmdline.h
│   ├── types.hpp
└──src/                  # Codes for LUT Evaluation 
│   ├── propsed method/
│   │   ├── comparision.h 
│   │   ├── generateLut.h 
│   │   ├── utils.h 
│   ├── gen_keys.cpp
│   ├── gen_tables.cpp
│   ├── main_Li.cpp
│   ├── main_oneinput.cpp
│   ├── main_twoinput.cpp
│   └── main_threeinput.cpp
│
└── CMakeLists.txt/          
```

### Test Examples
One-input function
```
./bin/gen_keys -N 65536 -L 23 --q0 50 --qi 40 --ql 60 --type oneInput  --bitNum 3
./bin/gen_tables --type oneInput --bitNum 3
OMP_NUM_THREAD=1 ./bin/main_oneinput --n 4 --d 4 --test 6 --bitNum 3
```
Two-input function
```
./bin/gen_keys -N 65536 -L 24 --q0 50 --qi 40 --ql 60 --type twoInput  --bitNum 3
./bin/gen_tables --type twoInput --bitNum 3 
OMP_NUM_THREAD=1 ./bin/main_twoinput --n 4 --d 4 --test 6 --bitNum 3
```
Related work
```
./bin/gen_keys -N 65536 -L 25 --q0 50 --qi 40 --ql 60 --type baseline  --bitNum 3
./bin/gen_tables --type baseline --bitNum 3 
OMP_NUM_THREAD=1 ./bin/main_Li --n 4 --d 4 --test 6 --bitNum 3
```
Three-input function
```
./bin/gen_keys -N 65536 -L 25 --q0 50 --qi 40 --ql 60 --type threeInput  --bitNum 3
./bin/gen_tables --type threeInput --bitNum 3 
OMP_NUM_THREAD=1 ./bin/main_threeinput --n 4 --d 4 --test 6 --bitNum 3
```
## License
- [MIT License](./LICENSE)
