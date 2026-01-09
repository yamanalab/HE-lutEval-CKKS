#include "common.h"
#include "utils.h"
#include <cmath>
#include <iostream>

// change range of input data here
double Lutxmax = 101;
double Lutxmin = 1;

void print_parameters(
    const lbcrypto::CryptoContext<lbcrypto::DCRTPoly> &context) {
  /*
  Which scheme are we using?
  */
  std::string scheme_name;
  lbcrypto::SCHEME schemeID;
  schemeID = context->getSchemeId();
  switch (schemeID) {
  case lbcrypto::BFVRNS_SCHEME:
    scheme_name = "BFV";
    break;
  case lbcrypto::CKKSRNS_SCHEME:
    scheme_name = "CKKS";
    break;
  default:
    throw std::invalid_argument("unsupported scheme");
  }
  std::cout << "/" << std::endl;
  std::cout << "| Encryption parameters :" << std::endl;
  std::cout << "|   scheme: " << scheme_name << std::endl;
  std::cout
      << "|   poly_modulus_degree: "
      << context->GetCryptoParameters()->GetElementParams()->GetRingDimension()
      << std::endl;

  /*
  Print the size of the true (product) coefficient modulus.
  */
  std::cout << "|   coeff_modulus size: ";
  std::cout << context->GetModulus().GetLengthForBase(2) << " (";
  auto coeff_modulus =
      context->GetCryptoParameters()->GetElementParams()->GetParams();
  std::size_t coeff_modulus_size = coeff_modulus.size();
  for (std::size_t i = 0; i < coeff_modulus_size - 1; ++i) {
    std::cout << coeff_modulus[i]->GetModulus().GetLengthForBase(2) << " + ";
  }
  std::cout << coeff_modulus.back()->GetModulus().GetLengthForBase(2);
  std::cout << ") bits" << std::endl;
  std::cout << "log2 q = " << context->GetElementParams()->GetModulus().GetMSB()
            << std::endl;

  /*
  For the BFV scheme print the plain_modulus parameter.
  */
  if (schemeID == lbcrypto::BFVRNS_SCHEME) {
    std::cout << "|   plain_modulus: "
              << context->GetCryptoParameters()->GetPlaintextModulus()
              << std::endl;
  }
  std::cout << "\\" << std::endl;
}

vector<int> generate_log2_modulus(size_t level, size_t log2_q0, size_t log2_qi,
                                  size_t log2_ql) {
  vector<int> modulus(level + 2);
  modulus[0] = log2_q0;
  for (size_t i = 1; i <= level; ++i) {
    modulus[i] = log2_qi;
  }
  modulus[level + 1] = log2_ql;

  return modulus;
}

double evalFunction(string functionName, double x) {
  if (functionName == "relu") {
    return (x > 0.0) ? x : 0.0;

  } else if (functionName == "sigmoid") {
    return 1.0 / (1.0 + exp(-x));
  } else if (functionName == "tanh") {
    return exp(-x) / (1.0 + exp(-x));
  } else if (functionName == "exp") {
    return exp(-x);
  } else if (functionName == "log") {
    return log(x);
  }
  return 0.0;
}
