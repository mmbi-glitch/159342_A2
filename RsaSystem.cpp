//
// Created by mmbil on 10/05/2023.
//

#include "RsaSystem.h"

RsaSystem::RsaSystem() = default;

void RsaSystem::generate_rsa_key(mp::cpp_int p, mp::cpp_int q) {
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

mp::cpp_int RsaSystem::get_e() {return e;}
mp::cpp_int RsaSystem::get_n() {return n;}
mp::cpp_int RsaSystem::get_d() {return d;}

mp::cpp_int RsaSystem::encrypt(mp::cpp_int m, mp::cpp_int e, mp::cpp_int n) {
//    return mp::powm(m, e, n);
    return repeat_square(m, e, n);
}

mp::cpp_int RsaSystem::decrypt(mp::cpp_int m, mp::cpp_int d, mp::cpp_int n) {
//    return mp::powm(m, d, n);
    return repeat_square(m, d, n);
}

mp::cpp_int RsaSystem::repeat_square(mp::cpp_int a, mp::cpp_int b, mp::cpp_int m) {
    mp::cpp_int X = 1;
    mp::cpp_int A = a % m;
    mp::cpp_int B = b;
    while (B >= 1) {
        if (B % 2 != 0) {
            B = (mp::cpp_int) B - 1;
            X = (mp::cpp_int) (A * X) % m;
        }
        else {
            B = (mp::cpp_int) B / 2;
            A = (mp::cpp_int) mp::powm(A, 2, m);
        }
    }
    return X;
}

mp::cpp_int RsaSystem::euclidean_algo(mp::cpp_int x, mp::cpp_int y) {
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

mp::cpp_int RsaSystem::get_rand_num() {
    rd::mt19937 base_gen(clock());
    rd::independent_bits_engine<rd::mt19937, 128, mp::cpp_int> gen(base_gen);
    return gen();
}

mp::cpp_int RsaSystem::extended_euclidean_algo(mp::cpp_int z, mp::cpp_int e) {
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
