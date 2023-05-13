//
// Created by mmbil on 10/05/2023.
//

#include "CryptoSystem.h"

CryptoSystem::CryptoSystem() = default;

void CryptoSystem::generate_rsa_key(mp::cpp_int p, mp::cpp_int q) {
    n = p * q;
    z = (p - 1) * (q - 1);

    e = 0;

    while (true) {

        e = get_rand_num();

        if ( (e < 1) || (e == p) || (e == q) || (e >= n) ) {
            continue;
        }

        mp::cpp_int gcd_of_e_z = euclidean_algo(e, z);

        if (gcd_of_e_z == 1) {
            break;
        }
    }

    d = extended_euclidean_algo(z, e);
}

void CryptoSystem::generate_fixed_rsa_key(mp::cpp_int p, mp::cpp_int q) {
    n = p * q;
    z = (p - 1) * (q - 1);
    e = mp::cpp_int(159826223);
    d = extended_euclidean_algo(z, e);
}

mp::cpp_int CryptoSystem::get_e() {return e;}
mp::cpp_int CryptoSystem::get_n() {return n;}
mp::cpp_int CryptoSystem::get_d() {return d;}

mp::cpp_int CryptoSystem::encrypt_rsa(mp::cpp_int m, mp::cpp_int e, mp::cpp_int n) {
    return mp::powm(m, e, n);
}

mp::cpp_int CryptoSystem::decrypt_rsa(mp::cpp_int m, mp::cpp_int d, mp::cpp_int n) {
    return mp::powm(m, d, n);
}

mp::cpp_int CryptoSystem::encrypt_rsa_cbc(mp::cpp_int m, mp::cpp_int e, mp::cpp_int n, mp::cpp_int rand) {
    return encrypt_rsa((m ^ rand), e, n);
}

mp::cpp_int CryptoSystem::decrypt_rsa_cbc(mp::cpp_int m, mp::cpp_int d, mp::cpp_int n, mp::cpp_int rand) {
    return decrypt_rsa(m, d, n) ^ rand;
}

mp::cpp_int CryptoSystem::euclidean_algo(mp::cpp_int x, mp::cpp_int y) {
    mp::cpp_int remainder = 0;

    while (true) {
        remainder = x % y;
        if (remainder == 0) {
            break;
        }
        x = y;
        y = remainder;
    }

    return y;
}

mp::cpp_int CryptoSystem::get_rand_num() {
    rd::mt19937 base_gen(clock());
    rd::independent_bits_engine<rd::mt19937, 32, mp::cpp_int> gen(base_gen);
    return gen();
}

mp::cpp_int CryptoSystem::extended_euclidean_algo(mp::cpp_int z, mp::cpp_int e) {
    std::vector<mp::cpp_int> x = std::vector<mp::cpp_int>();
    std::vector<mp::cpp_int> y = std::vector<mp::cpp_int>();
    std::vector<mp::cpp_int> w = std::vector<mp::cpp_int>();
    std::vector<mp::cpp_int> k = std::vector<mp::cpp_int>();

    // initialize

    x.emplace_back(1);
    y.emplace_back(0);

    x.emplace_back(0);
    y.emplace_back(1);

    w.emplace_back(z);
    w.emplace_back(e);

    k.emplace_back(0);

    int i = 1;
    while (w.back() != 1) {
        k.emplace_back(w.at(i-1) / w.at(i));
        i++;
        x.emplace_back(x.at(i-2) - (k.at(i-1)*x.at(i-1)));
        y.emplace_back(y.at(i-2) - (k.at(i-1)*y.at(i-1)));
        w.emplace_back(w.at(i-2) - (k.at(i-1)*w.at(i-1)));
    }
    if (y.back() < 0) {
        return y.back() + z;
    }
    return y.back();
}
