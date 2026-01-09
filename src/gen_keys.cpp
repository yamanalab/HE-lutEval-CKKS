#include "common.h"
#include "utils.h"
#include "time.h"
#include <iostream>
#include <random>


#include <iomanip>
#include <tuple>
#include <unistd.h>

#include "openfhe.h"

// header files needed for serialization
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"

using namespace std;
using namespace lbcrypto;


int main(int argc, char *argv[]){
    cmdline::parser parser;

    parser.add<size_t>("poly-deg", 'N', "Degree of polynomial ring");
    parser.add<size_t>("level", 'L',
                        "Initial level of ciphertext (Multiplicative depth)");
    parser.add<size_t>("q0", 0, "Bit number of the first prime in coeff_modulus");
    parser.add<size_t>("qi", 0,
                        "Bit number of intermediate primes in coeff_modulus");
    parser.add<size_t>("ql", 0, "Bit number of the last prime in coeff_modulus");
    parser.add<string>("type", 0, "onehot or treebased");
    parser.add<int>("bitNum", 0, "the number of bit");


    parser.parse_check(argc, argv);
    const size_t poly_modulus_degree = parser.get<size_t>("poly-deg");
    const size_t level = parser.get<size_t>("level");
    const size_t log2_q0 = parser.get<size_t>("q0");
    const size_t log2_qi = parser.get<size_t>("qi");
    const size_t log2_ql = parser.get<size_t>("ql");
    const string type = parser.get<string>("type");
    const int bitNum = parser.get<int>("bitNum");



    auto startWhole=chrono::high_resolution_clock::now();
    cout << "Generate key" << endl;  
    // Step1: Set CryptoContext
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetRingDim(poly_modulus_degree);
    parameters.SetMultiplicativeDepth(level);
    parameters.SetFirstModSize(log2_q0);
    parameters.SetScalingModSize(log2_qi);
    parameters.SetBatchSize(poly_modulus_degree/2);
    // parameters.SetBatchSize(pow(2,bitNum+3));//for test
    parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetScalingTechnique(FIXEDMANUAL);
    CryptoContext<DCRTPoly> context = GenCryptoContext(parameters);
    
    context->Enable(PKE);
    context->Enable(KEYSWITCH);
    context->Enable(LEVELEDSHE);
    context->Enable(ADVANCEDSHE);
    
    print_parameters(context);
  
    const size_t log2_f = 30;
    const double scale = static_cast<double>(static_cast<uint64_t>(1) << log2_f);
    const int slot_count = context->GetEncodingParams()->GetBatchSize();

    cout << "slot_count:"<<slot_count<<endl;
    KeyPair<DCRTPoly> key_pair = context->KeyGen();
    PrivateKey<DCRTPoly> secret_key = key_pair.secretKey;
    PublicKey<DCRTPoly> public_key = key_pair.publicKey;
  
    // Generate the evaluation keys
    context->EvalMultKeyGen(key_pair.secretKey);
    // Generate the rotation keys
    vector<int> rotation_steps;
    int64_t rotNum = slot_count/pow(2,bitNum);
    rotation_steps.push_back(-rotNum);
    for(int i=0;i<log2(slot_count);i++){
        rotation_steps.push_back(rotNum*pow(2,i));
    }
    print_vector(rotation_steps,NAME(rotation_steps));
    context ->EvalRotateKeyGen(key_pair.secretKey, rotation_steps);
    cout << "Finish generating keys!\n" << endl;
    

    cout << "Save parameter...\n" << flush;
    fs::path typeDirPath = "data/Key/"+type;
    fs::path dirPath = "data/Key/"+type+"/"+to_string(bitNum)+"bit";

    if (fs::exists(typeDirPath) && fs::is_directory(typeDirPath)) {
        std::cout << typeDirPath << " exist!" << std::endl;
    } else {
        std::cout  << typeDirPath << " not exist!" << std::endl;
        fs::create_directory(typeDirPath);
        std::cout  << typeDirPath << " created!" << std::endl;
    }

    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
        std::cout << dirPath << " exist!" << std::endl;
    } else {
        std::cout  << dirPath << " not exist!" << std::endl;
        fs::create_directory(dirPath);
        std::cout  << dirPath << " created!" << std::endl;
    }

    if (!Serial::SerializeToFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/Params", context, SerType::BINARY)) {
        std::cerr << "Error writing serialization of the crypto context to "
                     "cryptocontext.txt"
                  << std::endl;
        std::exit(1);
    }
    cout << "Save public key and secret key...\n" << flush;
    if (!Serial::SerializeToFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/PublicKey", key_pair.publicKey, SerType::BINARY)) {
        std::cerr << "Error writing serialization of public key to util/Key/"+type+"/"+to_string(bitNum)+"bit/PublicKey.txt" << std::endl;
        std::exit(1);
    }

    if (!Serial::SerializeToFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/SecretKey", key_pair.secretKey, SerType::BINARY)) {
        std::cerr << "Error writing serialization of private key to util/Key/SecretKey.txt" << std::endl;
        return 1;
    }

    std::ofstream emkeyfile("data/Key/"+type+"/"+to_string(bitNum)+"bit/RelinKey", std::ios::out | std::ios::binary);
    if (emkeyfile.is_open()) {
        if (context->SerializeEvalMultKey(emkeyfile, SerType::BINARY) == false) {
            std::cerr << "Error writing serialization of the eval mult keys to "
                         "util/Key/RelinKey"
                      << std::endl;
            return 1;
        }
        std::cout << "The eval mult keys have been serialized." << std::endl;

        emkeyfile.close();
    }
    else {
        std::cerr << "Error serializing eval mult keys" << std::endl;
        return 1;
    }

    // Serialize the rotation keyhs
    std::ofstream erkeyfile("data/Key/"+type+"/"+to_string(bitNum)+"bit/RotateKey", std::ios::out | std::ios::binary);
    if (erkeyfile.is_open()) {
        if (context->SerializeEvalAutomorphismKey(erkeyfile, SerType::BINARY) == false) {
            std::cerr << "Error writing serialization of the eval rotation keys to "
                         "key-eval-rot.txt"
                      << std::endl;
            return 1;
        }
        std::cout << "The eval rotation keys have been serialized." << std::endl;

        erkeyfile.close();
    }
    else {
        std::cerr << "Error serializing eval rotation keys" << std::endl;
        return 1;
    }
    
    cout << "Finish generating keys!\n" << endl;
 

    auto endWhole=chrono::high_resolution_clock::now();
    chrono::duration<double> diffWhole = endWhole-startWhole;
    cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

    return 0;
}
  