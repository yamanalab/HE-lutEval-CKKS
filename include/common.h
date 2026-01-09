#ifndef COMMON_H
#define COMMON_H

// OpenFHE Headers
#include "binfhe/binfhecontext.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "cryptocontext.h"
#include "cryptocontextfactory.h"
#include "key/key-ser.h"
#include "openfhe.h"
#include "scheme/ckksrns/ckksrns-ser.h"
#include "scheme/scheme-id.h"
#include "scheme/scheme-utils.h"
#include "schemebase/base-scheme.h"
#include "types.hpp"

// Standard Library Headers
#include "bits/stdc++.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cmdline.h>
#include <experimental/filesystem>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

// Namespace and Common Definitions
using namespace lbcrypto;
using namespace std;
namespace fs = std::experimental::filesystem;
using std::ifstream;
using std::ios;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
using vecInt = std::vector<int64_t>;

// Random Number Generation
extern std::random_device rd;
extern std::mt19937 gen;
extern std::uniform_real_distribution<double> dis;

// Macro Definitions
#define NAME(variable) (#variable)
#define rep(i, n, m) for (int i = n; i <= (int)(m); i++)

#endif // COMMON_H
