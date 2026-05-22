/* cyxspdefs.cpp
 *
 ###############################################################################
 # DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE                                 #
 # Version 2, December 2004                                                    #
 #                                                                             #
 # Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>                            #
 #                                                                             #
 # Everyone is permitted to copy and distribute verbatim or modified copies of #
 # this license document, and changing it is allowed as long as the name is    #
 # changed.                                                                    #
 #                                                                             #
 # DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE                                 #
 # TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION             #
 #                                                                             #
 # 0. You just DO WHAT THE FUCK YOU WANT TO.                                   #
 ###############################################################################
 */


#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#define _WIN32_WINNT 0x0600
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#include <stdint.h>
#include <string.h>


void erroom() {
    MessageBox(
        NULL, TEXT("Out of memory"), TEXT("ERROR"),
        MB_ICONERROR | MB_OK
    );
    #ifndef CYXSPDEFS_IGNORE_ERROR
    exit(0);
    #endif
}


class STDOUTPUT
{
public:
    HANDLE hconsole;
    STDOUTPUT() {
        hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    void print(char* str, DWORD len) {
        WriteConsole(hconsole, str, len, NULL, NULL);
    }
    void print(char* str) {
        WriteConsole(hconsole, str, strlen(str), NULL, NULL);
    }
    void print(int32_t num, uint8_t maxl = 12) {
        char* numstr = (char*)malloc(maxl);
        itoa(num, numstr, 10);
        print(numstr);
        free(numstr);
    }
};
#ifndef NDEBUG
STDOUTPUT debug;
#endif


#define G_CACHE_DFTLS        1
#define G_CACHE_DFTSZ        1
#define CACHELN         size_t
#define CACHECOL        size_t
class G_CACHE
{
public:
    void** cachels;
    CACHECOL* ls_ptrs;
    CACHELN* ls_szs;
    CACHELN ls_used = 0, ls_max = G_CACHE_DFTLS;
    G_CACHE() {
        ls_ptrs = (CACHECOL*)malloc(G_CACHE_DFTLS * sizeof(CACHECOL));
        ls_szs = (CACHELN*)malloc(G_CACHE_DFTLS * sizeof(CACHELN));
        cachels = (void**)malloc(G_CACHE_DFTLS * sizeof(void*));
        if (!ls_ptrs || !ls_szs || !cachels) erroom();
    }
    ~G_CACHE() {
        for (CACHELN i = 0; i < ls_used; i++) if (cachels[i]) free(cachels[i]);
        free(cachels);
        free(ls_ptrs);
        free(ls_szs);
    }
    CACHELN regcache() {
        if (ls_used >= ls_max) {
            if (ls_max > 2) ls_max += ls_max >> 1;
            else ls_max++;
            ls_ptrs = (CACHECOL*)realloc(ls_ptrs, ls_max * sizeof(CACHECOL));
            ls_szs = (CACHELN*)realloc(ls_szs, ls_max * sizeof(CACHELN));
            cachels = (void**)realloc(cachels, ls_max * sizeof(void*));
            if (!ls_ptrs || !ls_szs || !cachels) erroom();
        }
        cachels[ls_used] = (void*)malloc(G_CACHE_DFTSZ);
        if (!cachels[ls_used]) erroom();
        ls_ptrs[ls_used] = 0;
        ls_szs[ls_used] = G_CACHE_DFTSZ;
        return ls_used++;
    }
    void resz(CACHELN reg, CACHECOL sz) {
        cachels[reg] = (void*)realloc(cachels[reg], sz);
        ls_szs[reg] = sz;
        if (ls_ptrs[reg] >= sz) ls_ptrs[reg] = sz - 1;
    }
    void writecache(CACHELN reg, void* data, CACHECOL datalen) {
        bool reszed = 0;
        while (ls_ptrs[reg] + datalen > ls_szs[reg]) {
            if (ls_szs[reg] > 2) ls_szs[reg] += ls_szs[reg] >> 1;
            else ls_szs[reg]++;
            reszed = 1;
        }
        if (reszed) {
            void* temp = realloc(cachels[reg], ls_szs[reg]);
            if (!temp) {
                free(cachels[reg]);
                erroom();
            }
            cachels[reg] = temp;
        }
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        memcpy(cachels[reg] + ls_ptrs[reg], data, datalen);
        #pragma GCC diagnostic pop
        ls_ptrs[reg] += datalen;
    }
};
#ifndef NOCACHE
G_CACHE g_cache;
#endif


class STR_INT_DICT
{
public:
    

};

