//
// Created by mmbil on 10/05/2023.
//

#ifndef INC_159342_A2_CRYPTOSYSTEM_H
#define INC_159342_A2_CRYPTOSYSTEM_H


#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random.hpp>

namespace rd = boost::random;
namespace mp = boost::multiprecision;

class CryptoSystem {
private:
    mp::cpp_int z;
    mp::cpp_int e;
    mp::cpp_int n;
    mp::cpp_int d;
public:
    CryptoSystem();
    void generate_rsa_key(mp::cpp_int p, mp::cpp_int q);
    void generate_fixed_rsa_key(mp::cpp_int p, mp::cpp_int q);
    mp::cpp_int euclidean_algo(mp::cpp_int x, mp::cpp_int y);
    mp::cpp_int extended_euclidean_algo(mp::cpp_int z, mp::cpp_int e);
    mp::cpp_int get_rand_num();
    mp::cpp_int encrypt_rsa(mp::cpp_int m, mp::cpp_int e, mp::cpp_int n);
    mp::cpp_int decrypt_rsa(mp::cpp_int m, mp::cpp_int d, mp::cpp_int n);
    mp::cpp_int encrypt_rsa_cbc(mp::cpp_int m, mp::cpp_int e, mp::cpp_int n, mp::cpp_int rand);
    mp::cpp_int decrypt_rsa_cbc(mp::cpp_int m, mp::cpp_int d, mp::cpp_int n, mp::cpp_int rand);
    mp::cpp_int get_e();
    mp::cpp_int get_n();
    mp::cpp_int get_d();
};


#endif //INC_159342_A2_CRYPTOSYSTEM_H
