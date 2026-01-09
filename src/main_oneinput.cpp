#include "common.h"
#include "comparision.h"
#include "generateLUT.h"
#include "time.h"
#include "utils.h"
#include <iostream>
#include <random>

using namespace std;
using namespace lbcrypto;

// random seed
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

double uniform_random(double a, double b, unsigned int seed = 42) {
  static std::mt19937 generator(seed);
  std::uniform_real_distribution<double> distribution(a, b);
  return distribution(generator);
}
/* HERE is the main function!!! */
int main(int argc, char *argv[]) {

  cmdline::parser parser;
  parser.add<size_t>("n", 0, "n in comparision");
  parser.add<size_t>("d", 0, "d in comparision");
  parser.add<int>("test", 0, "Number of test data");
  parser.add<int>("bitNum", 0, "the number of bit");

  parser.parse_check(argc, argv);
  const size_t n = parser.get<size_t>("n");
  const size_t d = parser.get<size_t>("d");
  const int test_num = parser.get<int>("test");
  const int bitNum = parser.get<int>("bitNum");
  ;

  cout << "times for update:" << d << endl;
  cout << "the number of bit:" << bitNum << endl;
  cout << "Finish generating the test data!\n" << endl;

  cout << "Generate key" << endl;
  cout << "Setting FHE" << endl;
  string type = "oneInput";
  CryptoContext<DCRTPoly> context;
  if (!Serial::DeserializeFromFile("data/Key/" + type + "/" +
                                       to_string(bitNum) + "bit/Params",
                                   context, SerType::BINARY)) {
    std::cerr << "I cannot read serialization from data/Key/" + type + "/" +
                     to_string(bitNum) + "bit/Params"
              << std::endl;
    return 1;
  }
  print_parameters(context);

  const size_t log2_f = 30;
  const double scale = static_cast<double>(static_cast<uint64_t>(1) << log2_f);
  const int slot_count = context->GetEncodingParams()->GetBatchSize();
  cout << "slot_count:" << slot_count << endl;

  cout << "Loading seal keys..." << endl;
  PrivateKey<DCRTPoly> secret_key;
  if (Serial::DeserializeFromFile("data/Key/" + type + "/" + to_string(bitNum) +
                                      "bit/SecretKey",
                                  secret_key, SerType::BINARY) == false) {
    std::cerr << "Could not read secret key" << std::endl;
    return 1;
  }
  PublicKey<DCRTPoly> public_key;
  if (Serial::DeserializeFromFile("data/Key/" + type + "/" + to_string(bitNum) +
                                      "bit/PublicKey",
                                  public_key, SerType::BINARY) == false) {
    std::cerr << "Could not read public key" << std::endl;
    return 1;
  }
  std::ifstream emkeys("data/Key/" + type + "/" + to_string(bitNum) +
                           "bit/RelinKey",
                       std::ios::in | std::ios::binary);
  if (!emkeys.is_open()) {
    std::cerr << "I cannot read serialization from "
              << "data/Key/" + type + "/" + to_string(bitNum) + "bit/RelinKey"
              << std::endl;
    return 1;
  }
  if (context->DeserializeEvalMultKey(emkeys, SerType::BINARY) == false) {
    std::cerr << "Could not deserialize the eval mult key file" << std::endl;
    return 1;
  }
  std::ifstream erkeys("data/Key/" + type + "/" + to_string(bitNum) +
                           "bit/RotateKey",
                       std::ios::in | std::ios::binary);
  if (!erkeys.is_open()) {
    std::cerr << "I cannot read serialization from "
              << "data/Key/" + type + "/" + to_string(bitNum) + "bit/RotateKey"
              << std::endl;
    return 1;
  }
  if (context->DeserializeEvalAutomorphismKey(erkeys, SerType::BINARY) ==
      false) {
    std::cerr << "Could not deserialize the eval rotation key file"
              << std::endl;
    return 1;
  }
  std::cout << "Deserialized the eval rotation keys." << std::endl;

  cout << "Finish loading keys!\n" << endl;

  string table_in =
      "data/Table/" + type + "/" + to_string(bitNum) + "bit/table_in";
  string table_out =
      "data/Table/" + type + "/" + to_string(bitNum) + "bit/table_out";
  string table_min =
      "data/Table/" + type + "/" + to_string(bitNum) + "bit/table_min";
  string table_max =
      "data/Table/" + type + "/" + to_string(bitNum) + "bit/table_max";

  vector<Ciphertext<DCRTPoly>> temp_enc_in, temp_enc_out;
  Ciphertext<DCRTPoly> enc_Lutxmin, enc_Lutxmax;
  if (Serial::DeserializeFromFile(table_in, temp_enc_in, SerType::BINARY) ==
      false) {
    std::cerr << "Could not read the ciphertext" << std::endl;
    return 1;
  }
  if (Serial::DeserializeFromFile(table_out, temp_enc_out, SerType::BINARY) ==
      false) {
    std::cerr << "Could not read the ciphertext" << std::endl;
    return 1;
  }
  if (Serial::DeserializeFromFile(table_min, enc_Lutxmin, SerType::BINARY) ==
      false) {
    std::cerr << "Could not read the ciphertext" << std::endl;
    return 1;
  }
  if (Serial::DeserializeFromFile(table_max, enc_Lutxmax, SerType::BINARY) ==
      false) {
    std::cerr << "Could not read the ciphertext" << std::endl;
    return 1;
  }

  // set up input here
  vector<float> random(test_num);
  for (int i = 0; i < test_num; ++i) {
    random[i] = uniform_random(Lutxmin, Lutxmax);
  }
  std::cout << "random:" << random << std::endl;

  vector<double> latencyList(test_num, 0);
  vector<double> resultList(test_num, 0);
  vector<double> realresultList(test_num, 0);

  for (int testCount = 0; testCount < test_num; testCount++) {
    cout << "test " << testCount + 1 << endl;

    cout << "\033[32m===query generation===\033[0m" << endl;
    vector<double> input_vec;
    for (int i = 0; i < slot_count; i++)
      input_vec.push_back(random[testCount]);
    Plaintext poly_input;
    poly_input = context->MakeCKKSPackedPlaintext(input_vec);
    Ciphertext<DCRTPoly> temp_input = context->Encrypt(public_key, poly_input);
    // table search
    cout << "\033[32m===Table Search Processing===\033[0m" << endl;
    temp_input = context->EvalSub(temp_input, enc_Lutxmin);
    temp_input = context->EvalMult(temp_input, enc_Lutxmax);
    // context->RelinearizeInPlace(temp_input);
    context->RescaleInPlace(temp_input); // Only one rescale at this point

    auto NewComp_result_begin_time = high_resolution_clock::now();
    Ciphertext<DCRTPoly> result = oneInput_proposed(
        temp_input, temp_enc_in, temp_enc_out, context, n, d, pow(2, bitNum));

    auto NewComp_result_end_time = high_resolution_clock::now();
    duration<double> NewComp_result_sec =
        NewComp_result_end_time - NewComp_result_begin_time;
    cout << "latency:" << NewComp_result_sec.count() << endl;
    {
      vector<double> dec_result;
      Plaintext poly_dec_result;
      context->Decrypt(secret_key, result, &poly_dec_result);
      poly_dec_result->SetLength(slot_count);
      dec_result = poly_dec_result->GetRealPackedValue();
      vector<double> LUTinputHarm(slot_count, 0.0);
      cout << endl;
      resultList[testCount] = dec_result[0];
      cout << "dec_result:" << dec_result[0] << endl;
    }
    latencyList[testCount] = NewComp_result_sec.count();
    realresultList[testCount] = random[testCount];
  }

  cout << "\033[32m===Table Search Processing End===\033[0m" << endl;
  cout << "latency:" << latencyList << endl;
  cout << "avelatency:"
       << accumulate(latencyList.begin(), latencyList.end(), 0.0) /
              double(latencyList.size())
       << endl;
  cout << "queryList:" << random << endl;
  cout << "resultList:" << resultList << endl;
  cout << "realresultList:" << realresultList << endl;

  return 0;
}
