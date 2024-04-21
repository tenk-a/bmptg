/*
    ITU-R BT.601 / ITU-R BT.709 (1250/50/2:1)
        G = Y - 0.344136 × Cb - 0.714136 × Cr
        R = Y                  + 1.402    × Cr
        B = Y + 1.772    × Cb
*/

#include <stdio.h>
int main()
{
    long long  k = 65536;
//  long long  k = 100000;
    long long  m[3][3];
    long long  d[3];
    int i = 0;

    //  g = (100000*y -  34414*u -  71414*v) / 100000;
    //  r = (100000*y            + 140200*v) / 100000;
    //  b = (100000*y + 177200*u           ) / 100000;

    m[0][0] = (k * 100000 + 50000) / 100000, m[0][1] = (k * -34414 - 50000) / 100000, m[0][2] = (k * -71414 - 50000) / 100000;
    m[1][0] = (k * 100000 + 50000) / 100000, m[1][1] = 0                            , m[1][2] = (k * 140200 + 50000) / 100000;
    m[2][0] = (k * 100000 + 50000) / 100000, m[2][1] = (k * 177200 + 50000) / 100000, m[2][2] = 0;
    d[0]    = m[0][0]+m[0][1]+m[0][2];
    d[1]    = m[1][0]+m[1][1]+m[1][2];
    d[2]    = m[2][0]+m[2][1]+m[2][2];

    printf("dif = %6lld + %6lld + %6lld = %6lld\n", m[0][0], m[0][1], m[0][2], d[i]);
    printf("dif = %6lld + %6lld + %6lld = %6lld\n", m[1][0], m[1][1], m[1][2], d[i]);
    printf("dif = %6lld + %6lld + %6lld = %6lld\n", m[2][0], m[2][1], m[2][2], d[i]);

    printf("g = (%6lld * y + %6lld * u + %6lld * v) / %6lld\n", m[0][0], m[0][1], m[0][2], k);
    printf("r = (%6lld * y +            + %6lld * v) / %6lld\n", m[1][0],          m[1][2], k);
    printf("b = (%6lld * y + %6lld * u             ) / %6lld\n", m[2][0], m[2][1],          k);
    return 0;
}
