#ifndef UTILS_H
#define UTILS_H
#include "common.h"
#include <string>
#include <vector>

extern double Lutxmax;
extern double Lutxmin;

template <class T>
void print_vector(const std::vector<T> &vec, const std::string &label) {
  std::cout << label << ": ";
  int count = 0;
  for (const auto &val : vec) {
    std::cout << val << ", ";
    count++;
    if (count > 24) {
      std::cout << std::endl;
      return;
    }
  }
  std::cout << std::endl;
}
// double uniform_random(double a, double b, unsigned int seed = 42);

// void show_memory_usage(pid_t pid);

void print_parameters(
    const lbcrypto::CryptoContext<lbcrypto::DCRTPoly> &context);

vector<int> generate_log2_modulus(size_t level, size_t log2_q0, size_t log2_qi,
                                  size_t log2_ql);

double evalFunction(string functionName, double x);
#endif // UTILS_H