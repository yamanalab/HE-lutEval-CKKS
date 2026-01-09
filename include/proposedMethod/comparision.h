#ifndef COMPUTATION_H
#define COMPUTATION_H
#include "common.h"
#include "openfhe.h"
#include <vector>
#include <cstdint>
#include <chrono>

using namespace lbcrypto;
using std::vector;
using std::size_t;

// Define the func function to compute a  sgn function .
void func(Ciphertext<DCRTPoly>& x, int n, CryptoContext<DCRTPoly>& context);

// homomorphic comparison
void NewComp(Ciphertext<DCRTPoly>& x,
             Ciphertext<DCRTPoly> ct1,
             Ciphertext<DCRTPoly> ct2,
             int n, int d,
             int rotNum,
             CryptoContext<DCRTPoly>& context);

// total sum
Ciphertext<DCRTPoly> TotalSum(Ciphertext<DCRTPoly> ctxt, 
                              int64_t length, 
                              const CryptoContext<DCRTPoly>& context);

// partial sum
Ciphertext<DCRTPoly> PartialSum(Ciphertext<DCRTPoly> ctxt, 
                                int64_t length, 
                                int64_t num, 
                                const CryptoContext<DCRTPoly>& cryptoContext);

// LUT process for oneinput evaluation 
Ciphertext<DCRTPoly> oneInput_proposed(Ciphertext<DCRTPoly> query,
                                vector<Ciphertext<DCRTPoly>> lut_ctx,
                                vector<Ciphertext<DCRTPoly>> lut_cty,
                                CryptoContext<DCRTPoly>& context,
                                int n, int d, int lut_size);
// LUT process for twoinput evaluation (related work)
Ciphertext<DCRTPoly> twoInput_Li(vector<Ciphertext<DCRTPoly>> query,
                                vector<Ciphertext<DCRTPoly>> lut_ctx,
                                vector<Ciphertext<DCRTPoly>> lut_cty,
                                vector<Ciphertext<DCRTPoly>> pre_lut_y,
                                CryptoContext<DCRTPoly>& context,
                                int n, int d, int lut_size);
// LUT process for threeinput evaluation
Ciphertext<DCRTPoly> twoInput_propsed(vector<Ciphertext<DCRTPoly>> query,
                                vector<Ciphertext<DCRTPoly>> lut_ctx,
                                vector<Ciphertext<DCRTPoly>> lut_cty,
                                CryptoContext<DCRTPoly>& context,
                                int n, int d, int lut_size);
// LUT process for threeinput evaluation
Ciphertext<DCRTPoly> threeInput_propsed(vector<Ciphertext<DCRTPoly>> query,
                                vector<Ciphertext<DCRTPoly>> lut_ctx,
                                vector<Ciphertext<DCRTPoly>> lut_cty,
                                CryptoContext<DCRTPoly>& context,
                                int n, int d, int lut_size);

#endif // COMPUTATION_H
