#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wstringop-overflow="

#define CYXSPGDI_IGNORE_ERROR
//#include "cyxspdraw.cpp"

#include "cyxspgdi.cpp"
//#include "cyxspfl.cpp"
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <stdio.h>


#define ENABLE_ALPHA // here
STDWD_RGB vf2gray(float val, float vlmax) {
    BYTE n = BYTE(fabs(val) / vlmax * 150.f);
    STDWD_RGB rgb(n, n, n);
    return rgb;
}

void mainswdredr(STDBMP* hbmp, SUBWD* rootswd) {
    STDWD_RGB rgb(0, 0, 0);
    FLP2D p1(100.f, 10.f), p2(120.f, 50.f);
    float omega1 = .3f, omega2 = .5f, theta1 = 0.f, theta2 = .5f, gama1 = 3.f, gama2 = 2.f;
    float vlmax = sqrt(pow(gama1, 2.f) + pow(gama2, 2.f));
    float d1, d2;
    for (int32_t x = 0; x < rootswd->tar_hbmp->mp_x; x++) {
        for (int32_t y = 0; y < rootswd->tar_hbmp->mp_y; y++) {
            d1 = sqrt(pow(fabs(p1.x - (float)x), 2.f) + pow(fabs(p1.y - (float)y), 2.f));
            d2 = sqrt(pow(fabs(p2.x - (float)x), 2.f) + pow(fabs(p2.y - (float)y), 2.f));
            rgb = vf2gray(gama1 * sin(d1 * omega1 + theta1) + gama2 * sin(d2 * omega2 + theta2), vlmax);
            rootswd->dot_nptr(rootswd->getabsoffset(x, y), rgb, 255 - rgb.r, 1);
        }
    }
}

struct CLICKMEMSTRUCT {
    bool mousedown = 0;
    STDWD* pmainwd = nullptr;
};

void childswdredr(STDBMP* hbmp, SUBWD* rootswd) {
    STDWD_RGB rgb(0, 128, 0);
    for (int32_t x = 0; x < rootswd->swd_x; x++) {
        for (int32_t y = 0; y < rootswd->swd_y; y++) {
            rootswd->dot_nptr(rootswd->getabsoffset(x, y), rgb, 128, 0);
        }
    }
    CLICKMEMSTRUCT* pclickmemstruct = (CLICKMEMSTRUCT*)(rootswd->swd_pmemstruct);
    //bmp2fl("result.bmp", pclickmemstruct->pmainwd->wd_hbmp->dat, pclickmemstruct->pmainwd->wd_hbmp->mp_x, pclickmemstruct->pmainwd->wd_hbmp->mp_y, 4, 0, 1);
}

void childswdproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, SUBWD* rootswd) {
    CLICKMEMSTRUCT* pclickmemstruct;
    memcpy(&pclickmemstruct, &(rootswd->swd_pmemstruct), sizeof(void*));
    switch (msg) {
        case WM_MOUSEMOVE: {
            int32_t x = LOWORD(lparam);
            int32_t y = HIWORD(lparam);
            if (!rootswd->isabsxyinswd(x, y)) return;
            rootswd->absxy2curxy(&x, &y);
            if (pclickmemstruct->mousedown) {
                STDWD_RGB rgb(255, 0, 0);
                rootswd->dot_nptr(rootswd->getabsoffset(x, y), rgb, 255, 1);
                #ifdef ENABLE_ALPHA
                pclickmemstruct->pmainwd->update_alpha();
                #else
                pclickmemstruct->pmainwd->update();
                #endif
            }/*
            bmp2fl(
                "result2.bmp",
                pclickmemstruct->pmainwd->wd_hbmp->dat,
                pclickmemstruct->pmainwd->wd_hbmp->mp_x,
                pclickmemstruct->pmainwd->wd_hbmp->mp_y,
                4, 0, 1
            );*/
            break;
        }
        case WM_LBUTTONDOWN: {
            pclickmemstruct->mousedown = 1;
            int32_t x = LOWORD(lparam), y = HIWORD(lparam);
            if (!rootswd->isabsxyinswd(x, y)) return;
            rootswd->absxy2curxy(&x, &y);
            STDWD_RGB rgb(255, 0, 0);
            rootswd->dot_nptr(rootswd->getabsoffset(x, y), rgb);
            break;
        }
        case WM_LBUTTONUP: pclickmemstruct->mousedown = 0; break;
        default: swdexceptmsg(hwnd, msg, wparam, lparam);
    }
}

int main() {
    int32_t sz_x = 400, sz_y = 300;
    STDWD_CRE stl = {
        .base = WS_POPUP,
        .ex = WS_EX_APPWINDOW,
        .pop_x = CW_USEDEFAULT,
        .pop_y = CW_USEDEFAULT
    };
    #ifdef ENABLE_ALPHA
    stl.transp_invoked = 1;
    #endif
    STDWD wd(sz_x, sz_y, "Result");
    wd.cfgstl(stl, 0);
    wd.wd_main_nblock(nullptr);
    while (wd.wd_nready) Sleep(1);
    wd.update();
    /*
    LOGFONT lf = {
        .lfHeight = 24,
        .lfWidth = 0,
        .lfEscapement = 0,
        .lfOrientation = 0,
        .lfWeight = FW_NORMAL,
        .lfItalic = FALSE,
        .lfUnderline = FALSE,
        .lfStrikeOut = FALSE,
        .lfCharSet = DEFAULT_CHARSET,
        .lfOutPrecision = OUT_DEFAULT_PRECIS,
        .lfClipPrecision = CLIP_DEFAULT_PRECIS,
        .lfQuality = DEFAULT_QUALITY,
        .lfPitchAndFamily = DEFAULT_PITCH
    };
    STDWD_RGB rgb(0, 255, 0);
    FONTRENDERER renderer;
    renderer.setfont(&lf, 1, TEXT("Consolas"));
    renderer.setsz(wd.wd_hbmp->mp_x, wd.wd_hbmp->mp_y);
    renderer.setpos(20, 10);
    renderer.setrgb(rgb);
    renderer.str2buf_imm(wd.wd_hbmp->dat, TEXT("main window proc"));
    renderer.setpos(20, 40);
    renderer.str2buf_imm(wd.wd_hbmp->dat, TEXT("LOL"));
    wd.update();
    Sleep(200);
    */

    wd.wd_hmainsubwd->setredrproc(mainswdredr);
    wd.wd_hmainsubwd->redr();
    wd.update();


    CLICKMEMSTRUCT childswdmemstruct;
    childswdmemstruct.pmainwd = &wd;
    CLICKMEMSTRUCT* pchildswdmemstruct = &childswdmemstruct;
    void* vpchildswdmemstruct = (void*)pchildswdmemstruct;
    uint32_t swdid = wd.wd_hmainsubwd->cresubwd(200, 150, 3, 10, 0);
    wd.wd_hmainsubwd->setswdproc(wd.wd_hmainsubwd->distrimsg);
    printf("id: %d\n", swdid);
    wd.wd_hmainsubwd->swd_subwds[swdid]->setredrproc(childswdredr);
    wd.wd_hmainsubwd->swd_subwds[swdid]->setswdproc(childswdproc);
    wd.wd_hmainsubwd->swd_subwds[swdid]->setmemstruct(vpchildswdmemstruct);
    printf("child window redr\n");
    wd.wd_hmainsubwd->redr();
    wd.wd_hmainsubwd->swd_subwds[swdid]->redr();

    #ifdef ENABLE_ALPHA
    wd.premult();
    wd.update_alpha();
    #else
    wd.update();
    #endif
    printf("running? %s\n", wd.isrunning() ? "Yes" : "No");
    wd.join();
    /*
    Sleep(10000);
    wd.exit(0);
    */
    printf("running? %s\n", wd.isrunning() ? "Yes" : "No");
    return 0;
}

