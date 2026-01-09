#include "common.h"
#include "comparision.h"
#include "generateLUT.h"
#include "utils.h"

using namespace std;
using namespace lbcrypto;

double uniform_random(double a, double b, unsigned int seed = 42) {
  static std::mt19937 generator(seed);
  std::uniform_real_distribution<double> distribution(a, b);
  return distribution(generator);
}

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
  int inputNum = 3;
  int lut_size = pow(2, bitNum);

  cout << "times for update:" << d << endl;
  cout << "the number of bit:" << bitNum << endl;
  cout << "Finish generating the test data!\n" << endl;

  cout << "Generate key" << endl;
  cout << "Setting FHE" << endl;
  string type = "threeInput";
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

  vector<Ciphertext<DCRTPoly>> ct_lut_x, ct_lut_y;
  Ciphertext<DCRTPoly> enc_Lutxmin, enc_Lutxmax;
  if (Serial::DeserializeFromFile(table_in, ct_lut_x, SerType::BINARY) ==
      false) {
    std::cerr << "Could not read the ciphertext" << std::endl;
    return 1;
  }
  if (Serial::DeserializeFromFile(table_out, ct_lut_y, SerType::BINARY) ==
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
  vector<vector<float>> random(inputNum, vector<float>(test_num));
  srand(time(NULL));
  for (int i = 0; i < inputNum; i++) {
    for (int j = 0; j < test_num; j++) {
      random[i][j] = uniform_random(Lutxmin, Lutxmax);
    }
  }
  std::cout << "random:" << random << std::endl;

  vector<double> outputs;
  vector<double> expect_outputs;
  vector<double> latency_list;
  double ave_latency = 0;
  vector<Ciphertext<DCRTPoly>> ct_query(inputNum);

  for (int testCount = 0; testCount < test_num; testCount++) {
    cout << "test " << testCount + 1 << endl;
    vector<float> query(inputNum);
    std::cout << "input: ";
    for (int i = 0; i < inputNum; i++) {
      query[i] = random[i][testCount];
      std::cout << query[i] << ",";
    }
    std::cout << endl;

    vector<vector<double>> Vector(inputNum, vector<double>(slot_count));
    for (int i = 0; i < inputNum; i++) {
      vector<double> Vector1(slot_count, query[i]);
      Plaintext encoded_query1 = context->MakeCKKSPackedPlaintext(Vector1);
      ct_query[i] = context->Encrypt(public_key, encoded_query1);
      ct_query[i] = context->EvalSub(ct_query[i], enc_Lutxmin);
      ct_query[i] = context->EvalMult(ct_query[i], enc_Lutxmax);
      // context->RelinearizeInPlace(temp_input);
      context->RescaleInPlace(ct_query[i]); // Only one rescale at this point
    }

    auto NewComp_result_begin_time = high_resolution_clock::now();
    cout << "Start calculating!\n" << endl;
    Ciphertext<DCRTPoly> encrypted_result = threeInput_propsed(
        ct_query, ct_lut_x, ct_lut_y, context, n, d, lut_size);
    auto NewComp_result_end_time = high_resolution_clock::now();

    cout << "Decrypting result..." << endl;
    // Decryption
    Plaintext plain_result;
    vector<double> tmp_results(slot_count);
    context->Decrypt(secret_key, encrypted_result, &plain_result);
    plain_result->SetLength(slot_count);
    tmp_results = plain_result->GetRealPackedValue();
    // print_vector(tmp_results,NAME(tmp_results));
    double result = tmp_results[0];

    double expect_output = query[0] + query[1] + query[2];
    outputs.push_back(result);
    expect_outputs.push_back(expect_output);

    std::cout << "input: ";
    for (auto ele : query)
      std::cout << ele << ",";
    std::cout << endl;
    std::cout << "output: " << result << endl;
    std::cout << "real output: " << expect_output << endl;

    duration<double> NewComp_result_sec =
        NewComp_result_end_time - NewComp_result_begin_time;
    cout << "latency:" << NewComp_result_sec.count() << endl;
    latency_list.push_back(NewComp_result_sec.count());
    ave_latency += NewComp_result_sec.count();
  }
  ave_latency /= random[0].size();
  for (size_t i; i < random.size(); i++) {
    print_vector(random[i], NAME(random[i]));
  }

  print_vector(outputs, NAME(outputs));
  print_vector(expect_outputs, NAME(expect_outputs));
  print_vector(latency_list, NAME(latency_list));

  std::cout << "average latency: " << ave_latency << endl;

  return 0;
}