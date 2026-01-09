#include "common.h"
#include "utils.h"
#include "generateLUT.h"
#include "time.h"
#include <iostream>
#include <random>

using namespace std;
using namespace lbcrypto;


int main(int argc, char *argv[]){
    cmdline::parser parser;

    parser.add<int>("bitNum", 0, "the number of bit");
    parser.add<string>("type", 0, "onehot or treebased");



    parser.parse_check(argc, argv);
    const int bitNum = parser.get<int>("bitNum");
    const string type = parser.get<string>("type");

    fs::path typeDirPath = "data/Table/"+type;
    fs::path dirPath = "data/Table/"+type+"/"+to_string(bitNum)+"bit";

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


    string table_in="data/Table/"+type+"/"+to_string(bitNum)+"bit/table_in";
    string table_out="data/Table/"+type+"/"+to_string(bitNum)+"bit/table_out";
    string table_min="data/Table/"+type+"/"+to_string(bitNum)+"bit/table_min";
    string table_max="data/Table/"+type+"/"+to_string(bitNum)+"bit/table_max";
    

    cout << "Generate key" << endl;  
    cout << "Setting FHE" << endl;
    CryptoContext<DCRTPoly> context;
    if (!Serial::DeserializeFromFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/Params", context, SerType::BINARY)) {
        std::cerr << "I cannot read serialization from data/Key/"+type+"/"+to_string(bitNum)+"bit/Params" << std::endl;
        return 1;
    }  
    print_parameters(context);

    const size_t log2_f = 30;
    const double scale = static_cast<double>(static_cast<uint64_t>(1) << log2_f);
    const int slot_count = context->GetEncodingParams()->GetBatchSize();
    cout << "slot_count:"<<slot_count<<endl;

    cout << "Loading seal keys..." << endl;
    // PrivateKey<DCRTPoly> secret_key;
    // if (Serial::DeserializeFromFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/SecretKey", secret_key, SerType::BINARY) == false) {
    //     std::cerr << "Could not read secret key" << std::endl;
    //     return 1;
    // }
    PublicKey<DCRTPoly> public_key;
    if (Serial::DeserializeFromFile("data/Key/"+type+"/"+to_string(bitNum)+"bit/PublicKey", public_key, SerType::BINARY) == false) {
        std::cerr << "Could not read public key" << std::endl;
        return 1;
    }
    std::ifstream emkeys("data/Key/"+type+"/"+to_string(bitNum)+"bit/RelinKey", std::ios::in | std::ios::binary);
    if (!emkeys.is_open()) {
        std::cerr << "I cannot read serialization from " << "data/Key/"+type+"/"+to_string(bitNum)+"bit/RelinKey" << std::endl;
        return 1;
    }
    if (context->DeserializeEvalMultKey(emkeys, SerType::BINARY) == false) {
        std::cerr << "Could not deserialize the eval mult key file" << std::endl;
        return 1;
    }
    std::ifstream erkeys("data/Key/"+type+"/"+to_string(bitNum)+"bit/RotateKey", std::ios::in | std::ios::binary);
    if (!erkeys.is_open()) {
        std::cerr << "I cannot read serialization from " << "data/Key/"+type+"/"+to_string(bitNum)+"bit/RotateKey" << std::endl;
        return 1;
    }
    if (context->DeserializeEvalAutomorphismKey(erkeys, SerType::BINARY) == false) {
        std::cerr << "Could not deserialize the eval rotation key file" << std::endl;
        return 1;
    }
    std::cout << "Deserialized the eval rotation keys." << std::endl;

    cout << "Finish loading keys!\n" << endl;

    cout << "Generate LUT!\n" << endl;
    auto startWhole=chrono::high_resolution_clock::now();

        // Generate LUT
    vector<vector<double>> lut_x;
    vector<vector<double>> lut_y;
    if(type=="oneInput"){
        auto lut_pair = generateLUT_oneInput_propsed(bitNum,Lutxmax,Lutxmin,context);  
        lut_x=lut_pair.first;
        lut_y=lut_pair.second;
    }else if(type=="twoInput"){
        auto lut_pair = generateLUT_twoInput_propsed(bitNum,Lutxmax,Lutxmin,context);  
        lut_x=lut_pair.first;
        lut_y=lut_pair.second;
    }else if(type=="threeInput"){
        auto lut_pair = generateLUT_threeInput_propsed(bitNum,Lutxmax,Lutxmin,context);  
        lut_x=lut_pair.first;
        lut_y=lut_pair.second;
    }else if(type=="baseline"){
        auto lut_pair = generateLUT_twoInput(bitNum,Lutxmax,Lutxmin,context); 
        lut_x=lut_pair.first;
        lut_y=lut_pair.second;     
    }else{
        cout << "Type donot include.\n" << endl;
        return 0;
    }
    
    cout << "Finish generating LUT!\n" << endl;
    int inputNum=lut_x.size();
    vector<Ciphertext<DCRTPoly>> ct_lut_x(lut_x.size()),ct_lut_y(lut_y.size());
    for(int i=0;i<inputNum;i++){
        Plaintext encoded_lut_x    = context->MakeCKKSPackedPlaintext(lut_x[i]);
        ct_lut_x[i] = context->Encrypt(public_key, encoded_lut_x);
    }

    for(size_t i=0;i<lut_y.size();i++){
        Plaintext encoded_lut_y    = context->MakeCKKSPackedPlaintext(lut_y[i]);
        ct_lut_y[i]=context->Encrypt(public_key, encoded_lut_y);
    }
    cout << "Finish encyrpting LUT!\n" << endl;

    cout << "Generate maximum and minimum!\n" << endl;
    // max&min
    Plaintext pla_Lutxmax;
    Ciphertext<DCRTPoly> enc_Lutxmax;
    vector<double> vec_Lutxmax;
    for(int i=0;i<slot_count;i++) vec_Lutxmax.push_back(1.0/(Lutxmax-Lutxmin));
    pla_Lutxmax=context->MakeCKKSPackedPlaintext(vec_Lutxmax);
    enc_Lutxmax = context->Encrypt(public_key,pla_Lutxmax);

    Plaintext pla_Lutxmin;
    Ciphertext<DCRTPoly> enc_Lutxmin;
    vector<double> vec_Lutxmin;
    for(int i=0;i<slot_count;i++) vec_Lutxmin.push_back(Lutxmin);
    pla_Lutxmin=context->MakeCKKSPackedPlaintext(vec_Lutxmin);
    enc_Lutxmin = context->Encrypt(public_key,pla_Lutxmin);

    cout<<"---------Serialization---------"<<endl;
    
    if (!Serial::SerializeToFile(table_in,ct_lut_x, SerType::BINARY)) {
        std::cerr << "Error writing serialization of ciphertext 1 to " << table_in << std::endl;
        return 1;
    }

    if (!Serial::SerializeToFile(table_out,ct_lut_y, SerType::BINARY)) {
        std::cerr << "Error writing serialization of ciphertext 1 to " << table_out << std::endl;
        return 1;
    }

    if (!Serial::SerializeToFile(table_min, enc_Lutxmin, SerType::BINARY)) {
        std::cerr << "Error writing serialization of ciphertext 1 to " << table_min << std::endl;
        return 1;
    }
    if (!Serial::SerializeToFile(table_max, enc_Lutxmax, SerType::BINARY)) {
        std::cerr << "Error writing serialization of ciphertext 1 to " << table_max << std::endl;
        return 1;
    }

    auto endWhole=chrono::high_resolution_clock::now();
    chrono::duration<double> diffWhole = endWhole-startWhole;
    cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

    return 0;
}
  