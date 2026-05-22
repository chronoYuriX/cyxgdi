#include "cyxspfl.cpp"
#include <windows.h>
#include <stdio.h>

int main() {
    int32_t sz_x, sz_y;
    BYTE* src = fl24_2bmp32("src.bmp", &sz_x, &sz_y);
    printf("114514\n");
    bmp2fl("des.bmp", src, sz_x, sz_y, 4, 0, 0);
    printf("1919810\n");
    free(src);
    return 0;
}
