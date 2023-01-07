#include "../../h/kprint.h"
#include "../../h/syscall_c.h"
#include "../../h/kernelcons.h"

void kprintString(const char *string) {
    while (*string != '\0')
    {
        KernelConsole::getInstance()->put(*string);
        string++;
    }
}

char kdigits[] = "0123456789ABCDEF";

void kprintInt(int xx, int base, int sgn) {
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if(sgn && xx < 0){
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;
    do{
        buf[i++] = kdigits[x % base];
    }while((x /= base) != 0);
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
        KernelConsole::getInstance()->put(buf[i]);

}
