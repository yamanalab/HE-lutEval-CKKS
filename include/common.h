#ifndef COMMON_H
#define COMMON_H

// OpenFHE Headers
#include "openfhe.h"
#include "types.hpp"
#include "cryptocontext.h"
#include "cryptocontextfactory.h"
#include "schemebase/base-scheme.h"
#include "scheme/scheme-id.h"
#include "scheme/scheme-utils.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"
#include "binfhe/binfhecontext.h"

// Standard Library Headers
#include <experimental/filesystem>
#include <iomanip>
#include <tuple>
#include <fstream>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <random>
#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "bits/stdc++.h"
#include <cmdline.h>
#include <filesystem>

// Namespace and Common Definitions
using namespace lbcrypto;
using namespace std;
namespace fs = std::experimental::filesystem;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::ifstream;
using std::ios;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using vecInt = std::vector<int64_t>;

// Random Number Generation
extern std::random_device rd;
extern std::mt19937 gen;
extern std::uniform_real_distribution<double> dis;


// Macro Definitions
#define NAME(variable) (#variable)
#define rep(i, n, m) for (int i = n; i <= (int)(m); i++)

#endif // COMMON_H
