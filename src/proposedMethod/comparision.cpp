#include "common.h"
#include "comparision.h"
#include "types.hpp"
#include "utils.h"

void func(Ciphertext<DCRTPoly> &x, int n, CryptoContext<DCRTPoly> &context) {
  vector<double> func_coeff;
  if (n == 1) {
    func_coeff = {0.0, 3.0, 0.0, -1.0};
    for (auto &ele : func_coeff)
      ele /= 2.0;
  } else if (n == 2) {
    func_coeff = {0.0, 15.0, 0.0, -10.0, 0.0, 3.0};
    for (auto &ele : func_coeff)
      ele /= 8.0;
  } else if (n == 3) {
    func_coeff = {0.0, 35.0, 0.0, -35.0, 0.0, 21.0, 0.0, -5.0};
    for (auto &ele : func_coeff)
      ele /= 16.0;
  } else if (n == 4) {
    func_coeff = {0.0, 315.0, 0.0, -420.0, 0.0, 378.0, 0.0, -180.0, 0.0, 35.0};
    for (auto &ele : func_coeff)
      ele /= 128.0;
  }
  Ciphertext<DCRTPoly> ans;
  Ciphertext<DCRTPoly> x2, x3, x4;
  x2 = context->EvalMult(x, x); // L-1
  context->RescaleInPlace(x2);
  x3 = context->EvalMult(x, x2); // L-2
  context->RescaleInPlace(x3);
  x4 = context->EvalMult(x2, x2); // L-2
  context->RescaleInPlace(x4);

  Ciphertext<DCRTPoly> x5, x7, x9;
  if (n > 1) {
    x5 = context->EvalMult(x3, x2); // L-3
    context->RescaleInPlace(x5);
  }
  if (n > 2) {
    x7 = context->EvalMult(x4, x3); // L-3
    context->RescaleInPlace(x7);
  }
  if (n > 3) {
    x9 = context->EvalMult(x5, x4); // L-4
    context->RescaleInPlace(x9);
  }

  ans = context->EvalMult(x, func_coeff[1]);
  x3 = context->EvalMult(x3, func_coeff[3]);
  if (n > 1)
    x5 = context->EvalMult(x5, func_coeff[5]);
  if (n > 2)
    x7 = context->EvalMult(x7, func_coeff[7]);
  if (n > 3)
    x9 = context->EvalMult(x9, func_coeff[9]);

  ans = context->EvalAdd(ans, x3);
  if (n > 1) {
    ans = context->EvalAdd(ans, x5);
  }
  if (n > 2) {
    ans = context->EvalAdd(ans, x7);
  }
  if (n > 3) {
    ans = context->EvalAdd(ans, x9);
  }
  x = ans;
}

void NewComp(Ciphertext<DCRTPoly> &x, Ciphertext<DCRTPoly> ct1,
             Ciphertext<DCRTPoly> ct2, int n, int d, int rotNum,
             CryptoContext<DCRTPoly> &context) {
  x = context->EvalSub(ct1, ct2);
  for (int i = 1; i <= d; i++) {
    func(x, n, context);
    context->RescaleInPlace(x); // Only one rescale at this point
  }
  x = context->EvalAdd(x, 1);

  int slot_count = context->GetEncodingParams()->GetBatchSize();
  vector<double> preX(slot_count, 0.5);
  preX[slot_count - rotNum] = 0;
  // cout << "preX:" <<preX << endl;

  Plaintext encoded_preX = context->MakeCKKSPackedPlaintext(preX);
  x = context->EvalMult(x, encoded_preX);
}

Ciphertext<DCRTPoly> TotalSum(Ciphertext<DCRTPoly> ctxt, int64_t length,
                              const CryptoContext<DCRTPoly> &Context) {
  for (int64_t i = 0; i < log2(length); i++) {
    auto ciphertextRot = ctxt;
    int64_t t = pow(2, i);
    ciphertextRot = Context->EvalRotate(ctxt, t);
    ctxt = Context->EvalAdd(ctxt, ciphertextRot);
  }
  return ctxt;
}

Ciphertext<DCRTPoly> PartialSum(Ciphertext<DCRTPoly> ctxt, int64_t slot_count,
                                int64_t LUTnum,
                                const CryptoContext<DCRTPoly> &cryptoContext) {
  int64_t rotNum = slot_count / LUTnum;
  int64_t count = log2(LUTnum);
  // #pragma omp parallel for
  for (int64_t i = 0; i < count; i++) {
    auto ciphertextRot = ctxt;
    int64_t t = rotNum * pow(2, i);
    ciphertextRot = cryptoContext->EvalRotate(ctxt, t);
    cryptoContext->EvalAddInPlace(ctxt, ciphertextRot);
  }
  return ctxt;
}

Ciphertext<DCRTPoly> oneInput_proposed(Ciphertext<DCRTPoly> query,
                                       vector<Ciphertext<DCRTPoly>> lut_ctx,
                                       vector<Ciphertext<DCRTPoly>> lut_cty,
                                       CryptoContext<DCRTPoly> &context, int n,
                                       int d, int lut_size) {
  int slot_count = context->GetEncodingParams()->GetBatchSize();
  int row_size = lut_ctx.size();
  lut_size = min(slot_count, lut_size);
  vector<Ciphertext<DCRTPoly>> resultList;
  int rotNum = slot_count / lut_size;
  for (int i = 0; i < row_size; i++) {
    Ciphertext<DCRTPoly> rota_lut_ctx, rota_comp_x, comp_x, comp_y;
    NewComp(comp_x, lut_ctx[i], query, n, d, rotNum, context);
    rota_comp_x = context->EvalRotate(comp_x, -rotNum);
    comp_x = context->EvalSub(comp_x, rota_comp_x);
    comp_y = context->EvalMult(comp_x, lut_cty[i]);
    context->RescaleInPlace(comp_y); // Only one rescale at this point
    Ciphertext<DCRTPoly> result_row =
        PartialSum(comp_y, slot_count, lut_size, context);
    resultList.push_back(result_row);
  }
  Ciphertext<DCRTPoly> result = resultList[0];
  for (int i = 1; i < row_size; i++) {
    result = context->EvalAdd(result, resultList[i]);
  }
  return result;
}

Ciphertext<DCRTPoly> twoInput_Li(vector<Ciphertext<DCRTPoly>> query,
                                 vector<Ciphertext<DCRTPoly>> lut_ctx,
                                 vector<Ciphertext<DCRTPoly>> lut_cty,
                                 vector<Ciphertext<DCRTPoly>> pre_lut_y,
                                 CryptoContext<DCRTPoly> &context, int n, int d,
                                 int lut_size) {
  Ciphertext<DCRTPoly> rota_lut_ctx, comp_y, rota_comp_x;
  vector<Ciphertext<DCRTPoly>> comp_x(2);
  const size_t slot_count = context->GetEncodingParams()->GetBatchSize();
  int rotNum = slot_count / lut_size;
  for (int i = 0; i < 2; i++) {
    NewComp(comp_x[i], lut_ctx[i], query[i], n, d, rotNum, context);
    // comp_x right rotation +1
    rota_comp_x = context->EvalRotate(comp_x[i], -slot_count / lut_size);
    comp_x[i] = context->EvalSub(comp_x[i], rota_comp_x);
    context->RescaleInPlace(comp_x[i]);
  }
  for (size_t i = 0; i < pre_lut_y.size(); i++) {
    Ciphertext<DCRTPoly> tmp;
    tmp = context->EvalMult(comp_x[0], pre_lut_y[i]);
    context->RescaleInPlace(tmp);
    tmp = PartialSum(tmp, slot_count, lut_size, context);
    lut_cty[i] = context->EvalMult(tmp, lut_cty[i]);
    lut_cty[i] = context->EvalMult(comp_x[1], lut_cty[i]);
    context->RescaleInPlace(lut_cty[i]);
  }
  for (size_t i = 1; i < lut_cty.size(); i++) {
    lut_cty[0] = context->EvalAdd(lut_cty[0], lut_cty[i]);
  }
  Ciphertext<DCRTPoly> result =
      PartialSum(lut_cty[0], slot_count, lut_size, context);
  return result;
}

Ciphertext<DCRTPoly> twoInput_propsed(vector<Ciphertext<DCRTPoly>> query,
                                      vector<Ciphertext<DCRTPoly>> lut_ctx,
                                      vector<Ciphertext<DCRTPoly>> lut_cty,
                                      CryptoContext<DCRTPoly> &context, int n,
                                      int d, int lut_size) {
  Ciphertext<DCRTPoly> rota_lut_ctx, comp_y, rota_comp_x;
  vector<Ciphertext<DCRTPoly>> oneHot_tbl;
  // Step 1
  vector<Ciphertext<DCRTPoly>> comp_x(2);
  const size_t slot_count = context->GetEncodingParams()->GetBatchSize();
  int rotNum = slot_count / lut_size;
  for (int i = 0; i < 2; i++) {
    NewComp(comp_x[i], lut_ctx[i], query[i], n, d, rotNum, context);
    rota_comp_x = context->EvalRotate(comp_x[i], -slot_count / lut_size);
    comp_x[i] = context->EvalSub(comp_x[i], rota_comp_x);
    context->RescaleInPlace(comp_x[i]);
  }

  // Step 2
  vector<Ciphertext<DCRTPoly>> Rot_in(2);
  Rot_in[0] = comp_x[0];
  Rot_in[1] = comp_x[1];
  for (size_t k = 0; k < lut_cty.size(); k++) {
    // step2.1
    for (size_t j = 0; j < 1; j++) {
      if (k != 0) {
        Rot_in[j] = context->EvalRotate(Rot_in[j], slot_count / lut_size);
      }
    }
    oneHot_tbl.push_back(Rot_in[0]);
    for (size_t j = 1; j < 2; j++) {
      oneHot_tbl[k] = context->EvalMult(oneHot_tbl[k], Rot_in[j]);
    }
    // step2.2
    oneHot_tbl[k] = context->EvalMult(oneHot_tbl[k], lut_cty[k]);
  }

  // step3
  for (size_t i = 1; i < oneHot_tbl.size(); i++) {
    oneHot_tbl[0] = context->EvalAdd(oneHot_tbl[0], oneHot_tbl[i]);
  }

  context->RescaleInPlace(oneHot_tbl[0]);
  // step4
  Ciphertext<DCRTPoly> result =
      PartialSum(oneHot_tbl[0], slot_count, lut_size, context);

  return result;
}

Ciphertext<DCRTPoly> threeInput_propsed(vector<Ciphertext<DCRTPoly>> query,
                                        vector<Ciphertext<DCRTPoly>> lut_ctx,
                                        vector<Ciphertext<DCRTPoly>> lut_cty,
                                        CryptoContext<DCRTPoly> &context, int n,
                                        int d, int lut_size) {
  Ciphertext<DCRTPoly> rota_lut_ctx, comp_y, rota_comp_x;
  vector<Ciphertext<DCRTPoly>> oneHot_tbl;
  // Step 1
  vector<Ciphertext<DCRTPoly>> comp_x(3);
  const size_t slot_count = context->GetEncodingParams()->GetBatchSize();
  int rotNum = slot_count / lut_size;
  for (int i = 0; i < 3; i++) {
    NewComp(comp_x[i], lut_ctx[i], query[i], n, d, rotNum, context);
    rota_comp_x = context->EvalRotate(comp_x[i], -slot_count / lut_size);
    comp_x[i] = context->EvalSub(comp_x[i], rota_comp_x);
    context->RescaleInPlace(comp_x[i]);
  }

  // Step 2
  vector<Ciphertext<DCRTPoly>> Rot_in(3);
  Rot_in[0] = comp_x[0];
  Rot_in[1] = comp_x[1];
  Rot_in[2] = comp_x[2];
  for (size_t k = 0; k < lut_cty.size(); k++) {
    // step2.1
    for (size_t j = 0; j < 2; j++) {
      int step = int(floor(int(k) / int(pow(lut_size, 3 - j - 2)))) % lut_size;
      if (step == 0) {
        Rot_in[j] = comp_x[j];
      }
      if (int(k) % int(pow(lut_size, 3 - j - 2)) == 0 && step != 0) {
        Rot_in[j] = context->EvalRotate(Rot_in[j], slot_count / lut_size);
      }
    }
    oneHot_tbl.push_back(Rot_in[0]);
    for (size_t j = 1; j < 3; j++) {
      oneHot_tbl[k] = context->EvalMult(oneHot_tbl[k], Rot_in[j]);
    }
    // step2.2
    oneHot_tbl[k] = context->EvalMult(oneHot_tbl[k], lut_cty[k]);
  }

  // step3
  for (size_t i = 1; i < oneHot_tbl.size(); i++) {
    oneHot_tbl[0] = context->EvalAdd(oneHot_tbl[0], oneHot_tbl[i]);
  }

  context->RescaleInPlace(oneHot_tbl[0]);
  // step4
  Ciphertext<DCRTPoly> result =
      PartialSum(oneHot_tbl[0], slot_count, lut_size, context);

  return result;
}
