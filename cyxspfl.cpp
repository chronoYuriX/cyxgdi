/* cyxspfl.cpp
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
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#include <string.h>
#include <stdint.h>

#define CHUNK_SIZE 1048576


void flerr(char* errinfo) {
	char errcode[11];
	itoa(GetLastError(), errcode, 10);
	strcat(errinfo, errcode);
	MessageBox(NULL, errinfo, TEXT("ERROR"), MB_ICONERROR | MB_OK);
	#ifndef CYXSPFL_IGNORE_ERROR
	exit(0);
	#endif
}

DWORD rdfl(char* path, BYTE** tar, bool* isbig) {
	HANDLE hfile = CreateFile(
		path, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
	);
	if (hfile == INVALID_HANDLE_VALUE) {
		MessageBox(
			NULL, TEXT("Failed to open file"), TEXT("ERROR"),
			MB_ICONERROR | MB_OK
		);
        #ifndef CYXSPFL_IGNORE_ERROR
	    exit(0);
	    #endif
	}
	LARGE_INTEGER filesize;
	if (!GetFileSizeEx(hfile, &filesize)) {
		char errop[36] = "Failed to get file size: ";
		// "Failed ... size: " + "4294967295" + "\0" = 36
		flerr(errop);
	}
	DWORD bytesread;
	BYTE* buffer = nullptr;
	if (filesize.HighPart != 0 || filesize.LowPart > CHUNK_SIZE) {
	    buffer = (BYTE*)malloc(CHUNK_SIZE + 1);
	    ZeroMemory(buffer, CHUNK_SIZE + 1);
	    if (!ReadFile(hfile, buffer, CHUNK_SIZE, &bytesread, NULL)) {
	        char errop[36] = "Failed to read big file: ";
	        flerr(errop);
        }
        bytesread = 0;
        *isbig = 1;
    } else {
        buffer = (BYTE*)malloc(CHUNK_SIZE + 1);
	    ZeroMemory(buffer, filesize.LowPart + 1);
	    if (!ReadFile(hfile, buffer, filesize.LowPart, &bytesread, NULL)) {
		    char errop[32] = "Failed to read file: ";
		    flerr(errop);
	    }
	    CloseHandle(hfile);
	    *isbig = 0;
    }
    *tar = buffer;
    return bytesread;
}

bool rdfl_pt2(HANDLE hfile, BYTE* tar) {
    ZeroMemory(tar, CHUNK_SIZE + 1);
    DWORD bytesread;
    if (!ReadFile(hfile, tar, CHUNK_SIZE, &bytesread, NULL)) {
	    char errop[41] = "Failed to read big file(pt2): ";
	    flerr(errop);
    }
    if (bytesread == 0) {
        CloseHandle(hfile);
        return 0;
    }
    return 1;
}

void wtfl(char* path, BYTE** data, DWORD* szs, size_t datchunks) {
    HANDLE hfile = CreateFile(
        path, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if (hfile == INVALID_HANDLE_VALUE) {
		char errop[32] = "Failed to open file: ";
		// "Failed ... file: " + "4294967295" + "\0" = 32
		flerr(errop);
    }
    DWORD lwritten;
    for (size_t i = 0; i < datchunks; i++) {
        if (!WriteFile(hfile, data[i], szs[i], &lwritten, NULL)) {
            char errop[33] = "Failed to write file: ";
		    flerr(errop);
        }
        if (lwritten != szs[i]) {
    	    MessageBox(
		        NULL, TEXT("An error occurred while writing"), TEXT("ERROR"),
		        MB_ICONERROR | MB_OK
		    );
		    #ifndef CYXSPFL_IGNORE_ERROR
	        exit(0);
	        #endif
	    }
    }
    CloseHandle(hfile);
}

inline void wtfl1(char* path, BYTE* data, DWORD sz) {
    wtfl(path, &data, &sz, 1);
}

void bmp2fl(char* path, BYTE* img, int32_t sz_x, int32_t sz_y, uint8_t pxsz,
        bool rev_x = 0, bool rev_y = 0) {
    const int32_t i32_0b11 = 3;
    int32_t linesz = sz_x * pxsz;
    if (linesz & i32_0b11) {
        linesz += pxsz;
        linesz &= ~i32_0b11;
    }
    int32_t header[] = {
        linesz * sz_y + 54, 0, 54, 40,
        rev_x ? -sz_x : sz_x, rev_y ? -sz_y : sz_y, 1 | pxsz * 8 << 16,
        0, linesz * sz_y, 0, 0, 0, 0
    };
    BYTE imgfltag[2] = {'B', 'M'};
    BYTE* chunks[3] = {imgfltag, (BYTE*)header, img};
    DWORD szs[3] = {2, sizeof(header), (DWORD)sz_x * sz_y * pxsz};
    wtfl(path, chunks, szs, 3);
}

#include <stdio.h>
BYTE* fl24_2bmp32(char* path, int32_t* _sz_x, int32_t* _sz_y) {
    BYTE* data;
    bool unused;
    rdfl(path, &data, &unused);
    int32_t header[13];
    memcpy(header, data + 2, 52);
    const int32_t i32_0b11 = 3;
    int32_t sz_x = (header[4] < 0) ? -header[4] : header[4],
        sz_y = (header[5] < 0) ? -header[5] : header[5];
    (*_sz_x) = sz_x; (*_sz_y) = sz_y;
    int32_t linesz = (header[0] - 54) / sz_y, col_cnt = 0, ln_cnt = 0;
    BYTE* res = (BYTE*)malloc((int64_t)sz_x * sz_y * 4);
    for (int32_t ptr = 54; ptr < header[0]; ptr < header[0]) {
        res[col_cnt++] = data[ptr++];
        res[col_cnt++] = data[ptr++];
        res[col_cnt++] = data[ptr++];
        res[col_cnt++] = 255;
        ln_cnt += 3;
        if (ln_cnt + linesz - (sz_x * 3) == linesz) {
            ptr += linesz - sz_x * 3;
            ln_cnt = 0;
        }
    }
    free(data);
    return res;
}

