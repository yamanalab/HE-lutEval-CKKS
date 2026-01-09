#ifndef GENERATELUT_H
#define GENERATELUT_H

#include <string>
#include <utility>
#include <vector>

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_oneInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                             CryptoContext<DCRTPoly> &context);

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_twoInput(int bit_num, double Lutxmax, double Lutxmin,
                     CryptoContext<DCRTPoly> &context);

void generateLUT_threeInput(int lut_size_first, int inputNum,
                            vector<vector<double>> &lut_x,
                            vector<vector<double>> &lut_y);

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_twoInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                             CryptoContext<DCRTPoly> &context);
                             
pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_threeInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                               CryptoContext<DCRTPoly> &context);
#endif // GENERATELUT_H