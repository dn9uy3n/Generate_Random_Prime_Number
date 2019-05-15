#include "GInt.h"
#include "prime.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>

// Miller-Robin primality test
// return false: number n is composite
// return true:  number n is very likely to be a prime
//
bool is_prime(gint n, size_t trials)
{
    if (ggint_is_even(n))
        return false;

    gint _1;
    ggint_one(_1);
    gint n_1;
    ggint_set_gint(n_1,n);
    ggint_sub_gint(_1, n_1);

    size_t s = 0;
    gint m;
    ggint_set_gint(m,n_1);
    while (ggint_is_even(m))
    {
        ++s;
        ggint_shbr(m, 1);
    }

    gint d;
    {
        ggint_set_gint(m,n_1);
        gint t;
        ggint_set_gint(t, _1);
        gint r;
        ggint_shbl(t, s);
        ggint_div_gint(t, m, d, r);
    }

    if (trials == 0)
        trials = 3;

    size_t i;
    for (i = 0; i < trials; i++)
    {
        gint a;
        {
            gint _max; ggint_set_gint(_max, n);
            gint _2; ggint_set(_2, 2);
            gint _4; ggint_set(_4, 4);
            ggint_sub_gint(_4, _max);

            ggint_rand_range(a, _max);
            ggint_add_gint(_2, a);
        }

        gint x;
        ggint_pow_mod(a, d, n, x);

        if (ggint_equal(x, _1) || ggint_equal(x, n_1))
            continue;
        size_t r;
        for (r = 0; r < s - 1; r++)
        {
            gint x2, g;
            ggint_mul_gint(x, x, x2);
            ggint_mod_gint(n, x2, g);
            ggint_set_gint(x, g);

            if (ggint_equal(x, _1))
                return false;

            if (ggint_equal(x, n_1))
                break;
        }

        if (ggint_equal(x, n_1) == false)
            return false;
    }
    return true;
}

void generate_prime_number(uint8_t number[], int nbits){
    size_t smallPrimes[100] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541};
    size_t smallPrimesCount = 100;

    srand(time(0));

    gint n_lo, n_hi, _1;
    ggint_one(n_lo);
    ggint_one(n_hi);
    ggint_one(_1);
    ggint_shbl(n_lo, nbits - 1);
    ggint_shbl(n_hi, nbits);
    ggint_sub_gint(n_lo, n_hi);

    gint n;
    ggint_zero(n);
    size_t pmod[541];

    printf("Searching for %d-bit prime ...\n", nbits);

    clock_t start,end;
    double cpu_time_used;
    start = clock();

    size_t ncheck = 0;
    size_t i,p,r;
    while (true)
    {
        if (ggint_is_zero(n))
        {
            ggint_rand_range(n, n_hi);
            ggint_add_gint(n_lo, n);
            if (ggint_is_even(n))
                ggint_add_int(1, n);
            for (i=0; i<smallPrimesCount; i++)
            {
                p = smallPrimes[i];
                ggint_mod_int(p, n, &r);
                pmod[i] = r;
            }
        }

        bool do_fast = true;
        while (true)
        {
            for (i = 0; i < smallPrimesCount; ++i)
            {
                if (pmod[i] == 0)
                {
                    do_fast = false;
                    break;
                }
            }
            if (do_fast == false)
            {
                ncheck++;
                ggint_add_int(2, n);
                for (i=0; i < smallPrimesCount; i++)
                {
                    p = smallPrimes[i];
                    pmod[i] += 2;
                    if (pmod[i] >= p)
                        pmod[i] -= p;
                }
                do_fast = true;
                continue;
            }
            else
                break;
            if (do_fast == false)
                break;
        }

        if (is_prime(n, max(10, nbits/16)))
        {
            printf("\nFound prime p:\n");
            for(i = 0; i <128; i++){
                number[i] = n[127-i];
            }
            ggint_print_format("p", n,true);
            break;
        }
        else
        {
            printf(".");
            fflush(stdout);
        }

        ncheck++;
        ggint_add_int(2, n);
        for (i = 0; i < smallPrimesCount; ++i)
        {
            p = smallPrimes[i];
            pmod[i] += 2;
            if (pmod[i] >= p)
                pmod[i] -= p;
        }
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Checked %d numbers in %d ms: %g num/sec\n", (int) ncheck, (int) cpu_time_used, 1000.0*((double)(ncheck))/cpu_time_used);

    return;
}
