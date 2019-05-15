//
// Created by ubuntu on 15/05/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "GInt.h"

typedef uint8_t gint[GInt_Size];
gint ggint_cache[GInt_Base+1];
bool ggint_cache_empty = true;
gint ggint_lastnum;

bool ggint_equal(gint a, gint b)
{
    size_t i=0;
    for (i = 0; i < GInt_Size; i++)
        if (a[i] != b[i])
            return false;
    return true;
}
// a < b
bool ggint_less(gint a, gint b)
{
    size_t i=0;
    for (i = GInt_Size - 1; ; i--)
    {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
        if (i == 0) break;
    }
    return false;
}
// a <= b
bool ggint_less_or_equal(gint a, gint b)
{
    size_t i=0;
    for (i = GInt_Size - 1; ; i--)
    {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
        if (i == 0) break;
    }
    return true;
}
// a == 0
bool ggint_is_zero(gint a)
{
    size_t i=0;
    for (i = 0; i < GInt_Size; ++i)
        if (a[i] != 0) return false;
    return true;
}
// a & 1 == 0
bool ggint_is_even(gint a)
{
    return (a[0] & 1) == 0;
}
// a & 1 == 1
bool ggint_is_odd(gint a)
{
    return (a[0] & 1) == 1;
}
// a = 0
void ggint_zero(gint a)
{
    memset(a,0,GInt_Size);
}
// a = 1
void ggint_one(gint a)
{
    ggint_zero(a);
    a[0]=1;
}
// a = value
void ggint_set(gint a, size_t value)
{
    ggint_zero(a);
    size_t i=0;
    while (value>0 && i<GInt_Size)
    {
        a[i] = value % GInt_Base;
        value /= GInt_Base;
        i++;
    }
}
// a = b
void ggint_set_gint(gint a, gint b)
{
    memcpy(a,b,GInt_Size);
}
//b = b + a
void ggint_add_gint( gint a, gint b)
{
    uint16_t x = 0;
    size_t i = 0;
    for (i = 0; i < GInt_Size; i++)
    {
        x = (uint16_t) b[i] + a[i] + x;
        b[i] = x % GInt_Base;
        x /= GInt_Base;
    }
}
//b = b + a
void ggint_add_int(uint8_t a, gint b)
{
    uint16_t x = a;
    size_t i = 0;
    for (i = 0; i < GInt_Size; i++)
    {
        x = (uint16_t) b[i] + x;
        b[i] = x % GInt_Base;
        x /= GInt_Base;
        if (x==0)
            break;
    }
}
//b = b - a
void ggint_sub_gint( gint a, gint b)
{
    uint16_t r = 0;
    size_t i = 0;
    for (i = 0; i < GInt_Size; i++)
        if (b[i] < a[i] + r)
        {
            b[i] = r = GInt_Base + b[i] - a[i] - r;
            r = 1;
        }
        else
        {
            b[i] -= r + a[i];
            r = 0;
        }
}
void ggint_sub_int( uint8_t a, gint b)
{
    uint16_t r = a;
    size_t i = 0;
    for (i = 0; i < GInt_Size; i++)
        if (b[i] < r)
        {
            b[i] = r = GInt_Base + b[i] - r;
            r = 1;
        }
        else
        {
            b[i] -= r;
            r = 0;
        }
}
//a = a << 8*sh (base 256)
void ggint_shl(gint a, size_t sh)
{
    if (sh == 0)
        return;
    sh = min(GInt_Size, sh);
    size_t i = 0;
    for (i = GInt_Size - 1; i >= sh; i--)
        a[i] = a[i - sh];

    for (i = 0; i < sh; i++)
        a[i] = 0;
}
//a = a >> 8*sh (base 256)
void ggint_shr(gint a, size_t sh)
{
    if (sh == 0)
        return;
    sh = min(GInt_Size, sh);
    size_t i = 0;
    for (i = 0; i < GInt_Size - sh; i++)
        a[i] = a[i + sh];

    for (i = 0; i < sh; i++)
        a[GInt_Size - 1 - i] = 0;
}
//a = a << sh
void ggint_shbl(gint a, size_t sh)
{
    if (sh == 0)
        return;
    ggint_shl(a, sh / GInt_DigitBits);
    sh = sh % GInt_DigitBits;

    uint16_t mask = GInt_Base - (1 << (GInt_DigitBits - sh));
    uint8_t bits0 = 0, bits1 = 0;
    size_t i = 0;
    for (i = 0; i < GInt_Size; ++i)
    {
        bits1 = a[i] & mask;
        a[i] <<= sh;
        a[i] |= bits0 >> (GInt_DigitBits - sh);
        bits0 = bits1;
    }
}
// a = a >> sh
void ggint_shbr(gint a, size_t sh)
{
    if (sh == 0)
        return;
    ggint_shr(a, sh / GInt_DigitBits);
    sh = sh % GInt_DigitBits;

    uint16_t mask = (1 << sh) - 1;
    uint8_t bits0 = 0, bits1 = 0;
    size_t i = 0;
    for (i = GInt_Size - 1;; --i)
    {
        bits1 = a[i] & mask;
        a[i] >>= sh;
        a[i] |= bits0 << (GInt_DigitBits - sh);
        bits0 = bits1;
        if (i == 0)
            break;
    }
}
// b = b * a
void ggint_mul_int( uint8_t a, gint b)
{
    uint16_t r = 0;
    size_t i = 0;
    for (i = 0; i < GInt_Size; ++i)
    {
        r += (uint16_t) a * b[i];
        b[i] = r % GInt_Base;
        r /= GInt_Base;
    }
}
// p = a * b
void ggint_mul_gint( gint a,  gint b, gint p)
{
    gint t;
    ggint_zero(p);
    size_t i=0;
    for (i = 0; i < GInt_Size; ++i)
    {
        memcpy(t,b,GInt_Size);
        ggint_mul_int(a[i], t);
        ggint_shl(t, i);
        ggint_add_gint(t, p);
    }
}
// b / a = q, b % a = r
void ggint_div_gint( gint a,  gint b, gint q, gint r)
{
    ggint_zero(q);
    ggint_zero(r);
    gint t;
    ggint_zero(t);
    size_t i = 0;
    for (i = GInt_Size - 1;; --i)
    {
        ggint_shl(r, 1);
        ggint_add_int(b[i], r);
        if (ggint_less_or_equal(a, r))
        {
            ggint_zero(t);
            uint8_t k = 0;
            do
            {
                ++k;
                ggint_add_gint(a, t);
            } while (ggint_less_or_equal(t, r));
            q[i] = k-1;
            ggint_sub_gint(a, t);
            ggint_sub_gint(t, r);
        }
        else
            q[i] = 0;
        if (i == 0)
            break;
    }
}

// b % a = r
void ggint_mod_gint2( gint a,  gint b, gint r)
{
    gint q;
    ggint_div_gint(a,b,q,r);
}
void ggint_mod_gint(gint a, gint b, gint r)
{
    if (ggint_less(b, a))
    {
        ggint_set_gint(r,b);
        return;
    }

    ggint_zero(r);
    gint t;

    ggint_set_gint(ggint_lastnum, a);
    if (ggint_cache_empty || ggint_equal(a, ggint_lastnum) == false)
    {
        ggint_zero(t);
        size_t k;
        for (k = 0; k <= GInt_Base; k++)
        {
            ggint_set_gint(ggint_cache[k], t);
            ggint_add_gint(a, t);
        }
        ggint_set_gint(ggint_lastnum, a);
    }

    size_t i;
    for (i = GInt_Size - 1; ; --i)
    {
        ggint_shl(r,1);
        ggint_add_int(b[i], r);

        if (ggint_less_or_equal(a, r))
        {
            if (ggint_less_or_equal(ggint_cache[GInt_Base - 1], r))
            {
                ggint_sub_gint(ggint_cache[GInt_Base - 1], r);
            }
            else
            {
                int k0 = 0;
                int k1 = GInt_Base;
                while (true)
                {
                    int m = (k0 + k1)/2;
                    if (ggint_less_or_equal(ggint_cache[m], r))
                    {
                        k0 = m;
                    }
                    else
                    {
                        k1 = m;
                    }
                    if (k1 == k0 + 1) break;
                }
                ggint_sub_gint(ggint_cache[k0], r);
            }
        }
        if (i == 0) break;
    }
}
// b % a = r
void ggint_mod_int(size_t a,  gint b, size_t *r)
{
    *r = 0;
    size_t i = 0;
    for (i = GInt_Size - 1;; --i)
    {
        *r = (*r * GInt_Base + b[i]) % a;
        if (i == 0)
            break;
    }
}

// generate random number a
void ggint_rand(gint a)
{
    srand(time(0));
    size_t i = 0;
    for (i = 0; i < GInt_Size; i++)
        a[i] = rand() % GInt_Base;
}

// generate random number a < b
void ggint_rand_range(gint a,  gint b)
{
    ggint_rand(a);
    gint t;
    ggint_set_gint(t, a);

    gint q;
    ggint_div_gint(b, t, q, a);
}

// r = a^x mod n
void ggint_pow_mod(gint a, gint x,  gint n, gint r)
{
    gint t;
    ggint_one(r);
    ggint_zero(t);

    while (ggint_is_zero(x) == false)
    {
        if (ggint_is_odd(x))
        {
            ggint_mul_gint(a, r, t);
            ggint_mod_gint(n, t, r);
        }
        ggint_shbr(x, 1);
        ggint_mul_gint(a, a, t);
        ggint_mod_gint(n, t, a);
    }
}
void ggint_print_format( char * pref, gint x, bool printBytes)
{
    size_t n = 0;
    for (n = GInt_Size - 1; ; --n)
    {
        if (x[n] != 0) break;
        if (n==0) break;
    }

    if (printBytes)
    {
        printf(" - %16s : ", pref);
        for (int i = 0; i <= n; ++i)
            printf("%3d ", x[i]);
        printf("\n");
    }

    gint _10, q, r;
    ggint_set(_10, 10);
    char str[4096];
    memset(str,0,4096);

    n = 0;
    if (printBytes)
        printf("   %16s : ", "Decimal");
    else
        printf(" - %16s : ", pref);
    while (ggint_is_zero(x) == false)
    {
        ggint_div_gint(_10, x, q, r);
        ggint_set_gint(x,q);
        str[n++] = '0' + r[0];
    }
    size_t i;
    for (i = n - 1; ; --i)
    {
        printf("%c", str[i]);
        if (i==0) break;
    }
    printf("\n");
}
void ggint_print(gint a)
{
    size_t i;
    for (i=GInt_Size-1; ; i--)
    {
        printf("%3d ",a[i]);
        if (i==0)
            break;
    }
    printf("\n\n");
}