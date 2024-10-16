// ITU-R BT.601 / ITU-R BT.709 (1250/50/2:1)
//
//      Y = 0.299 × R + 0.587 × G + 0.114 × B
//      Cb = -0.168736 × R - 0.331264 × G + 0.5 × B
//      Cr = 0.5 × R - 0.418688 × G - 0.081312 × B

#include <stdio.h>
int main()
{
    long long  k = 65536;
//  long long  k = 100000;
    long long  m[3][3];
    long long  d[3];
    int i = 0;

    m[0][0] = (k *  58661 + 50000) / 100000, m[0][1] = (k *  29891 + 50000) / 100000, m[0][2] = (k * 11448 + 50000) / 100000;
    m[1][0] = (k * -33126 - 50000) / 100000, m[1][1] = (k * -16874 - 50000) / 100000, m[1][2] = (k * 50000 + 50000) / 100000;
    m[2][0] = (k * -41869 - 50000) / 100000, m[2][1] = (k *  50000 + 50000) / 100000, m[2][2] = (k * -8131 - 50000) / 100000;
    d[0]    = m[0][0]+m[0][1]+m[0][2]-k;
    d[1]    = m[1][0]+m[1][1]+m[1][2];
    d[2]    = m[2][0]+m[2][1]+m[2][2];

    printf("dif = %6lld + %6lld + %6lld - %6lld = %6lld\n", m[0][0], m[0][1], m[0][2], k, d[i]);
    printf("dif = %6lld + %6lld + %6lld = %6lld\n", m[1][0], m[1][1], m[1][2], d[i]);
    printf("dif = %6lld + %6lld + %6lld = %6lld\n", m[2][0], m[2][1], m[2][2], d[i]);

    printf("y = (%6lld * g + %6lld * r + %6lld * b) / %6lld\n", m[0][0], m[0][1], m[0][2], k);
    printf("u = (%6lld * g + %6lld * r + %6lld * b + %6lld*255) / %6lld\n", m[1][0], m[1][1], m[1][2], k/2, k);
    printf("v = (%6lld * g + %6lld * r + %6lld * b + %6lld*255) / %6lld\n", m[2][0], m[2][1], m[2][2], k/2, k);
    return 0;
}
