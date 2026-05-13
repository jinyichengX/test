#include "el_ipaddr.h"

#if defined(_WIN64)
    typedef unsigned long long size_t;
    typedef signed long long ssize_t;
#else
    typedef unsigned int size_t;
    typedef signed int ssize_t;
#endif

/* convert value to string */
static int itoak(ssize_t value, int unsign, char * str, int radix, int upper)
{
    /* rodata section */
    static const char hex_tbl[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
        'A', 'B', 'C', 'D', 'E', 'F'  
    };
    char * p = str;
    int negative = 0, len = 0;
    size_t val = value;

    if (!value) {
        * p ++ = '0';
        * p ++ = '\0';
        return 1;
    }

    if ((radix > 16)  || (radix < 0))
        return -1;
    
    if (value < 0) {
        if (!unsign) {
            negative = 1;
            val = value = -value;
        }else {
            val = (size_t)value;
        }
    }

    while (val > 0) {
        char idx = val % radix;
        *(p ++) = hex_tbl[(idx < 10) ? idx : (upper ? (idx + 6) : idx)];
        val /= radix;
    }
    if (negative)
        *(p ++) = '-';

    len = p - str;
    for (int i = 0; i < len / 2; i ++) {
        char j = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = j;
    }
    * p = '\0';

    return len;
}

/* ipv4 addr convert to string 
 * buf must be at least 16 bytes
 * for example: 255.255.255.255\0(total 16bytes)
 */

void ip4addr2str(char * buf, ip4addr_t * ipaddr)
{
    int len = 0;

    len += itoak(ipaddr->ipa[0], 1, buf + len, 10, 0);
    buf[len] = '.'; len ++;
    len += itoak(ipaddr->ipa[1], 1, buf + len, 10, 0);
    buf[len] = '.'; len ++;
    len += itoak(ipaddr->ipa[2], 1, buf + len, 10, 0);
    buf[len] = '.'; len ++;
    len += itoak(ipaddr->ipa[3], 1, buf + len, 10, 0);
    buf[len] = '\0';
}