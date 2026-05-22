/* cyxspgdi.cpp
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


#ifndef INCLUDED_CYXSPGDI
#define INCLUDED_CYXSPGDI

#pragma comment(lib, "gdi32.lib")

#ifdef CYXSPGDI_IGNORE_ERROR
    #ifndef CYXSPDEFS_IGNORE_ERROR
        #define CYXSPDEFS_IGNORE_ERROR
    #endif
#endif
#include "cyxspdefs.cpp"
#ifdef CYXSPGDI_IGNORE_ERROR
    #undef CYXSPDEFS_IGNORE_ERROR
#endif


//### MAIN GDI FUNCTIONS #######################################################

#define CYXSPGDI_DFTALPHA 255
struct STDWD_RGB
{
	BYTE r, g, b;
	STDWD_RGB(BYTE r, BYTE g, BYTE b): r(r), g(g), b(b) { }
};

BITMAPINFO crestdbmi(int32_t x, int32_t y) {
    BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  x;
   	bmi.bmiHeader.biHeight      = -y;
   	bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = (int64_t)x * y * 4;
    return bmi;
}
HBITMAP crestdhbmp(int32_t x, int32_t y, BYTE* data) {
    BITMAPINFO bmi = crestdbmi(x, y);
    HDC screendc = GetDC(NULL);
    HBITMAP hbmp = CreateDIBitmap(
        screendc, &bmi.bmiHeader, CBM_INIT, data, &bmi, DIB_RGB_COLORS
    );
    ReleaseDC(NULL, screendc);
    return hbmp;
}


class STDBMP
{
public:
    BYTE* dat = nullptr;
    int32_t mp_x = 0, mp_y = 0;
    int64_t mp_sz = 0;
//### CRE & DEL ################################################################
    STDBMP(int32_t x, int32_t y) {
        mp_x = x; mp_y = y; mp_sz = (int64_t)x * y * 4;
        dat = (BYTE*)malloc(mp_sz);
    }
    STDBMP(STDBMP* pbmp2) {
        mp_x = pbmp2->mp_x; mp_y = pbmp2->mp_y; mp_sz = pbmp2->mp_sz;
        dat = (BYTE*)malloc(mp_sz);
        memcpy(dat, pbmp2->dat, mp_sz);
    }
    ~STDBMP() { free(dat); }
    STDBMP(const STDBMP&) = delete;
    STDBMP& operator=(const STDBMP&) = delete;
//### CALL #####################################################################
    void resz(int32_t x, int32_t y) {
        mp_x = x; mp_y = y; mp_sz = (int64_t)x * y * 4;
        BYTE* tmp = (BYTE*)realloc(dat, mp_sz);
        if (tmp) dat = tmp;
        else erroom();
    }
    int64_t getoffset(int32_t x, int32_t y) {
        return (int64_t(y) * mp_x + x) * 4;
    }
    bool cekdot(int64_t offset) {
        return offset >= mp_sz || offset < 0;
    }
    inline void __dot(int64_t* offset, BYTE r, BYTE g, BYTE b) {
        dat[(*offset)++] = b; dat[(*offset)++] = g; dat[(*offset)++] = r;
    }
    inline void __dot_a(int64_t* offset, BYTE r, BYTE g, BYTE b, BYTE alpha) {
        BYTE clr0rate = 255 - alpha, tmp;
        #define GEN_ALPHAMIX(CHAN) \
            tmp = (CHAN * alpha + dat[*offset] * clr0rate + 127) >> 8; \
            dat[(*offset)++] = tmp;
        GEN_ALPHAMIX(r) GEN_ALPHAMIX(g) GEN_ALPHAMIX(b);
        #undef GEN_ALPHAMIX
        (*offset)++;
    }
    void dot(int64_t* offset, STDWD_RGB rgb, BYTE alpha = CYXSPGDI_DFTALPHA,
            bool overwrite = 1) {
        if (overwrite || alpha == 255) {
            __dot(offset, rgb.r, rgb.g, rgb.b);
            dat[(*offset)++] = alpha;
        } else if (alpha != 0) __dot_a(offset, rgb.r, rgb.g, rgb.b, alpha);
    }
    void msk_overwrite_ncek(STDBMP* bmp2, int32_t tar_x, int32_t tar_y) {
        int64_t offset = getoffset(tar_x, tar_y), offset_tar = 0;
        size_t dst = size_t(bmp2->mp_x) * 4;
        for (int64_t i = 0; i < bmp2->mp_y; i++) {
            memcpy(dat + offset, bmp2->dat + offset_tar, dst);
            offset += mp_x * 4;
            offset_tar += bmp2->mp_x * 4;
        }
    }
    void setalpha1(BYTE alpha) {
        for (int64_t i = 3; i < mp_sz; i += 4) dat[i] = alpha;
    }
    void setalpha_cek0(STDWD_RGB rgb, BYTE alpha_0, BYTE alpha_1) {
        for (int64_t i = 0; i < mp_sz; ) {
            if (dat[i] | dat[i + 1] | dat[i + 2]) {
                dat[i + 3] = alpha_1;
                i += 4;
            } else dot(&i, rgb, alpha_0, 1);
        }
    }
};


#define SUBWD_DFTITERWDS 4
class SUBWD
{
public:
    STDBMP* tar_hbmp = nullptr;
    uint16_t swd_subwdnum = 0, swd_maxsubwdbum = SUBWD_DFTITERWDS;
    int32_t swd_x = 0, swd_y = 0, tar_x = 0, tar_y = 0;
    int64_t swd_sz = 0;
    bool swd_havesubwd = 0, swd_isbase = 0, swd_hidden = 0;
    void (*swd_redr)(STDBMP*, SUBWD*) = subwd_voidredr;
    void (*swd_proc)(HWND, UINT, WPARAM, LPARAM, SUBWD*) = subwd_voidfunc;
    SUBWD* swd_parent = nullptr;
    SUBWD** swd_subwds = nullptr;
    void* swd_pmemstruct = nullptr;
//### GEN & DEL ################################################################
    SUBWD(int32_t x, int32_t y, bool havesubwd, bool isbase = 1,
            SUBWD* parent = nullptr) {
        swd_x = x; swd_y = y; swd_sz = (int64_t)x * y * 4;
        swd_havesubwd = havesubwd;
        if (havesubwd) swd_subwds = (SUBWD**)malloc(SUBWD_DFTITERWDS);
        swd_isbase = isbase;
        if (!isbase) swd_parent = parent;
    }
    void settar(STDBMP* redr_tar, int32_t x, int32_t y) {
        tar_hbmp = redr_tar;
        tar_x = x; tar_y = y;
    }
    inline void setredrproc(void (*proc)(STDBMP*, SUBWD*)) {
        swd_redr = proc;
    }
    inline void setswdproc(void (*proc)(HWND, UINT, WPARAM, LPARAM, SUBWD*)) {
        swd_proc = proc;
    }
    inline void setmemstruct(void* pmemstruct) {
        swd_pmemstruct = pmemstruct;
    }
    ~SUBWD() {
        if (swd_havesubwd) {
            for (uint16_t i = 0; i < swd_subwdnum; i++) {
                delete swd_subwds[i];
            }
        }
        free(swd_subwds);
    }
    SUBWD(const SUBWD&) = delete;
    SUBWD& operator=(const SUBWD&) = delete;
//### CALL #####################################################################
    static void subwd_voidredr(STDBMP* bmp, SUBWD* rootswd) { }
    static void subwd_voidfunc(HWND hwnd, UINT msg, WPARAM wparam,
            LPARAM lparam, SUBWD* root) { }
    static void distrimsg(HWND hwnd, UINT msg, WPARAM wparam,
            LPARAM lparam, SUBWD* root) {
        for (uint16_t i = 0; i < root->swd_subwdnum; i++)
            root->swd_subwds[i]->run_swd_proc(hwnd, msg, wparam, lparam);
    }
    void resz(int32_t x, int32_t y) {
        swd_x = x; swd_y = y; swd_sz = (int64_t)x * y * 4;
    }
    inline void dot(int64_t* offset, STDWD_RGB rgb,
            BYTE alpha = CYXSPGDI_DFTALPHA, bool overwrite = 1) {
        tar_hbmp->dot(offset, rgb, alpha, overwrite);
    }
    inline void dot_nptr(int64_t offset, STDWD_RGB rgb,
            BYTE alpha = CYXSPGDI_DFTALPHA, bool overwrite = 1) {
        tar_hbmp->dot(&offset, rgb, alpha, overwrite);
    }
    void iterxy(int32_t* px, int32_t* py) {
        if (swd_isbase) return;
        *px += tar_x; *py += tar_y;
        swd_parent->iterxy(px, py);
    }
    int64_t getabsoffset(int32_t x, int32_t y) {
        int32_t add_x = 0, add_y = 0;
        iterxy(&add_x, &add_y);
        return (int64_t(y + add_y) * tar_hbmp->mp_x + x + add_x) * 4;
    }
    void absxy2curxy(int32_t* x, int32_t* y) {
        int32_t zero_x = 0, zero_y = 0;
        iterxy(&zero_x, &zero_y);
        *x -= zero_x; *y -= zero_y;
    }
    bool isabsxyinswd(int32_t x, int32_t y) {
        return (tar_x <= x) && (x <= tar_x + swd_x) &&
            (tar_y <= y) && (y <= tar_y + swd_y);
    }
    void redr() {
        if (swd_hidden) return;
        swd_redr(tar_hbmp, this);
        if (swd_havesubwd) {
            uint16_t cur_subwd = 0;
            for (; cur_subwd < swd_subwdnum; cur_subwd++) {
                swd_subwds[cur_subwd]->redr();
            }
        }
    }
    void addsubwd(SUBWD* swda, int32_t x, int32_t y) {
        if (!swd_havesubwd) return;
        swda->settar(tar_hbmp, x + tar_x, y + tar_y);
        swd_subwds[swd_subwdnum++] = swda;
        if (swd_subwdnum >= swd_maxsubwdbum) {
            swd_maxsubwdbum += swd_maxsubwdbum >> 1;
            SUBWD** tmp = (SUBWD**)realloc(swd_subwds, swd_maxsubwdbum);
            if (tmp) swd_subwds = tmp;
            else erroom();
        }
    }
    uint16_t cresubwd(int32_t x, int32_t y, int32_t loc_x, int32_t loc_y,
            bool iter = 0) {
        SUBWD* pcursubwd = new SUBWD(x, y, iter, 0, this);
        addsubwd(pcursubwd, loc_x, loc_y);
        return swd_subwdnum - 1;
    }
    LRESULT run_swd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        swd_proc(hwnd, msg, wparam, lparam, this);
        return 0;
    }
};


#define STDWD_CLSNM_LEN                       64 // this one must below 255
#define STDWD_DFT_CLSNM_HEADER         "CYXSTD_"
#define STDWD_WDNM_LEN                        64 // < 255
#define STDWD_DFT_GRAY                         0 // 0 ~ 255
#define STDWD_DFT_POPUP_X                     20
#define STDWD_DFT_POPUP_Y                     15
#define STDWD_EXTERNAL_EXIT                    1 // int
#define STDWD_WAIT_TIMEOUT                 10000 // DWORD
#define STDWD_CURSORLNKNM                     10

#define STDWD_FXRESZ             (WM_USER + 100)
#define STDWD_RESZ_DELAY                      16 // DWORD

struct STDWD_CRE {
    DWORD base, ex;
    int32_t pop_x, pop_y;
    LONG_PTR base_cfg_add, ex_cfg_add, base_cfg_del, ex_cfg_del;
    bool transp_invoked;
};
struct CURSORLNK {
    LRESULT hitmsg;
    LPCSTR cursornm;
};
class STDWD_MSGLST
{
public:
    UINT* data = nullptr;
    UINT sz_used;
    bool sorted = 0;
    CACHELN lst_reg;
    STDWD_MSGLST(): sz_used(0) {
        lst_reg = g_cache.regcache();
        data = (UINT*)(g_cache.cachels[lst_reg]);
    }
    ~STDWD_MSGLST() {
        if (data) data = nullptr;
    }
    void print() {
        for (int i = 0; i < sz_used; i++) {
            debug.print(data[i]);
            debug.print(" ");
        }
        debug.print("\n");
    }
    void add(UINT msg) {
        if (find(msg)) return;
        g_cache.writecache(lst_reg, &msg, sizeof(UINT));
        sorted = 0;
        sz_used++;
    }
    bool find(UINT msg) {
        if (sorted) {
            UINT left = 0, right = sz_used - 1, mid;
            while (left <= right) {
                mid = left + ((right - left) >> 1);
                if (data[mid] > msg) right = mid - 1;
                else if (data[mid] < msg) left = mid + 1;
                else return 1;
            }
            return 0;
        }
        for (UINT i = 0; i < sz_used; i++)
            if (data[i] == msg) return 1;
        return 0;
    }
    __fastcall void sort_iter(UINT lo, UINT hi) {
        if (lo < hi) {
            UINT i = lo, j = hi;
            UINT p = data[lo];
            while (i < j) {
                while (i < j && data[j] >= p) j--;
                data[i] = data[j];
                while (i < j && data[i] <= p) i++;
                data[j] = data[i];
            }
            data[i] = p;
            sort_iter(lo, i - 1);
            sort_iter(i + 1, hi);
        }
    }
    void sort() {
        return;
        if (sorted || sz_used < 3) return;
        sort_iter(0, sz_used - 1);
        sorted = 1;
    }
};

LRESULT stdwd_voidfunc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
class STDWD
{
public:
    STDBMP*     wd_hbmp;
    SUBWD*      wd_hmainsubwd;
    PAINTSTRUCT wd_ps;
    HDC         wd_hdc, wd_mem_dc;
    HBITMAP     wd_mem_bmp;
	HWND        wd_hwnd;
	HINSTANCE   wd_hinstance, wd_hprevinstance;
	HANDLE      wd_hd;
	WNDCLASSEX  wd_wc;
	LPSTR       wd_lpcmdline;
	int32_t     wd_ncmdshow;
	char        wd_wdclsnm[STDWD_CLSNM_LEN], wd_wdnm[STDWD_WDNM_LEN];
	bool        wd_nready = 1;
	STDWD_CRE   wd_settings;
    int32_t     clk_framesz = 5, clk_edge_x, clk_edge_y, clk_topicsz = 20;
	LRESULT (*wd_procex)(HWND, UINT, WPARAM, LPARAM) = DefWindowProc;
	LRESULT (*wd_redrex)(HWND, UINT, WPARAM, LPARAM) = stdwd_voidfunc;
	BLENDFUNCTION wd_blend;
    POINT         wd_p0;
    STDWD_MSGLST* wd_exceptmsglst;
	CURSORLNK wd_cursorlnk[STDWD_CURSORLNKNM] = {
        {HTLEFT,        IDC_SIZEWE},   {HTRIGHT,       IDC_SIZEWE},
        {HTTOP,         IDC_SIZENS},   {HTBOTTOM,      IDC_SIZENS},
        {HTTOPLEFT,     IDC_SIZENWSE}, {HTBOTTOMRIGHT, IDC_SIZENWSE},
        {HTTOPRIGHT,    IDC_SIZENESW}, {HTBOTTOMLEFT,  IDC_SIZENESW},
        {HTCAPTION,     IDC_SIZEALL},  {HTCLIENT,      IDC_ARROW}
    };
    bool     wd_havebtns = 0;
    int32_t  clk_btns_x_0, clk_btns_x_1, clk_btns_height, wd_min_x, wd_min_y;
    DWORD    wd_lastresz;
//### CRE & DEL ################################################################
	STDWD(int32_t sz_x, int32_t sz_y, char* wdnm, char* clsnm = ".RAND") {
	    wd_hinstance = GetModuleHandle(NULL);
	    wd_lpcmdline = GetCommandLineA();
	    wd_ncmdshow = SW_SHOW;
	    if (clsnm[0] == '.') randclsnm();
	    else strncpy(wd_wdclsnm, clsnm, STDWD_CLSNM_LEN);
	    strncpy(wd_wdnm, wdnm, STDWD_WDNM_LEN);
        wd_hbmp = new STDBMP(sz_x, sz_y);
        wd_hmainsubwd = new SUBWD(sz_x, sz_y, 1, 1, nullptr);
        wd_hmainsubwd->settar(wd_hbmp, 0, 0);
        ZeroMemory(&wd_settings, sizeof(STDWD_CRE));
        wd_exceptmsglst = new STDWD_MSGLST();
        clk_edge_x = sz_x - clk_framesz - 1;
        clk_edge_y = sz_y - clk_framesz - 1;
        ZeroMemory(&wd_p0, sizeof(POINT));
        wd_blend = {
            .BlendOp = AC_SRC_OVER,
            .SourceConstantAlpha = 255,
            .AlphaFormat = AC_SRC_ALPHA
        };
        wd_lastresz = GetTickCount();
    }
    ~STDWD() { delbuf(); }
    STDWD(const STDWD&) = delete;
    STDWD& operator=(const STDWD&) = delete;
//### BUFFER ###################################################################
	void cfgbufsz(int32_t x, int32_t y) {
	    wd_hbmp->resz(x, y);
	    wd_hmainsubwd->resz(x, y);
    }
    void delbuf() {
        if (wd_nready) return;
        delete wd_hbmp;
        delete wd_hmainsubwd;
        delete wd_exceptmsglst;
        wd_nready = 1;
    }
	void initbuf(BYTE gray) {
		memset(wd_hbmp->dat, gray, wd_hbmp->mp_sz);
	}
//### PAINT ####################################################################
	inline void dot(int64_t* offset, STDWD_RGB rgb,
            BYTE alpha = CYXSPGDI_DFTALPHA, bool overwrite = 1) {
		wd_hbmp->dot(offset, rgb, alpha, overwrite);
	}
    inline void dot_nptr(int64_t offset, STDWD_RGB rgb,
            BYTE alpha = CYXSPGDI_DFTALPHA, bool overwrite = 1) {
        wd_hbmp->dot(&offset, rgb, alpha, overwrite);
    }
	int64_t getoffset(int32_t x, int32_t y) {
		return (int64_t(y) * wd_hbmp->mp_x + x) * 4;
	}
//### WINDOW ###################################################################
	static LRESULT CALLBACK wd_proc(HWND hwnd, UINT msg,
			WPARAM wparam, LPARAM lparam) {
		STDWD* pthis = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pcreate = (CREATESTRUCT*)lparam;
            pthis = (STDWD*)(pcreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pthis);
        } else pthis = (STDWD*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (pthis) return pthis->wd_proc_member(hwnd, msg, wparam, lparam);
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    inline BYTE __premult(BYTE chan, BYTE alpha) {
        uint16_t tmp = (uint16_t)chan;
        tmp = (tmp * alpha + 127) >> 8;
        return (BYTE)tmp;
    }
    inline void premult3(int64_t offset) {
        BYTE alpha = wd_hbmp->dat[offset + 3];
        wd_hbmp->dat[offset]     = __premult(wd_hbmp->dat[offset], alpha);
        wd_hbmp->dat[offset + 1] = __premult(wd_hbmp->dat[offset + 1], alpha);
        wd_hbmp->dat[offset + 2] = __premult(wd_hbmp->dat[offset + 2], alpha);
    }
    void premult() {
        if (wd_settings.transp_invoked)
            for (int64_t i = 0; i < wd_hbmp->mp_sz; i += 4) premult3(i);
    }
    void premultzone(int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1) {
        if (!wd_settings.transp_invoked) return;
        int64_t offset;
        for (; y_0 < y_1; y_0++) {
            offset = getoffset(x_0, y_0);
            for (int32_t x_0_cpy = x_0; x_0_cpy < x_1; x_0_cpy++) {
                premult3(offset);
                offset += 4;
            }
        }
    }
    void update_alpha()
    {
        HBITMAP hbmp = crestdhbmp(wd_hbmp->mp_x, wd_hbmp->mp_y, wd_hbmp->dat);
        HDC hdc_screen = GetDC(NULL);
        HDC hdc_mem = CreateCompatibleDC(hdc_screen);
        SelectObject(hdc_mem, hbmp);
        SIZE wdsz = {wd_hbmp->mp_x, wd_hbmp->mp_y};
        UpdateLayeredWindow(
            wd_hwnd, hdc_screen, NULL, &wdsz, hdc_mem, &wd_p0, 0,
            &wd_blend, ULW_ALPHA
        );
        ReleaseDC(NULL, hdc_screen);
        DeleteObject(hbmp);
        DeleteDC(hdc_mem);
    }
    bool hitonbuttons(int32_t x, int32_t y) {
        if (!wd_havebtns) return 0;
        POINT p = {x, y}; ScreenToClient(wd_hwnd, &p);
        x = wd_hbmp->mp_x - p.x;
        return clk_btns_x_0 < x && x < clk_btns_x_1 && p.y < clk_btns_height;
    }
    LRESULT mousehit(int32_t x, int32_t y) {
        if (hitonbuttons(x, y)) return HTCLIENT;
        POINT p = {x, y}; ScreenToClient(wd_hwnd, &p);
        uint8_t cklst = p.x < clk_framesz;        // left
        cklst |= uint8_t(p.x >= clk_edge_x) << 1; // right
        cklst |= uint8_t(p.y < clk_framesz) << 2; // top
        cklst |= uint8_t(p.y >= clk_edge_y) << 3; // bottom
        switch (cklst) {
            case 1: return HTLEFT;       case 2: return HTRIGHT;
            case 4: return HTTOP;        case 5: return HTTOPLEFT;
            case 6: return HTTOPRIGHT;   case 8: return HTBOTTOM;
            case 9: return HTBOTTOMLEFT; case 10: return HTBOTTOMRIGHT;
        }
        return (p.y < clk_topicsz) ? HTCAPTION : HTCLIENT;
    }
    void cursor4wd(LRESULT hitmsg) {
        for (uint8_t i = 0; i < STDWD_CURSORLNKNM; i++)
            if (wd_cursorlnk[i].hitmsg == hitmsg) {
                SetCursor(LoadCursor(NULL, wd_cursorlnk[i].cursornm));
                return;
            }
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
	LRESULT wd_proc_member(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	    wd_hmainsubwd->run_swd_proc(hwnd, msg, wparam, lparam);
		switch (msg) {
			case WM_PAINT: {
			    if (wd_settings.transp_invoked) {
			        PAINTSTRUCT ps;
			        BeginPaint(hwnd, &ps);
                    EndPaint(hwnd, &ps);
                    break;
                }
			    wd_redrex(hwnd, msg, wparam, lparam);
			    wd_hdc = BeginPaint(hwnd, &wd_ps);
        	    wd_mem_dc = CreateCompatibleDC(wd_hdc);
        	    wd_mem_bmp = CreateCompatibleBitmap(
                    wd_hdc, wd_hbmp->mp_x, wd_hbmp->mp_y
                );
                SelectObject(wd_mem_dc, wd_mem_bmp);
                BITMAPINFO bmi = crestdbmi(wd_hbmp->mp_x, wd_hbmp->mp_y);
    	        SetDIBitsToDevice(
        	        wd_mem_dc, 0, 0, wd_hbmp->mp_x, wd_hbmp->mp_y,
                    0, 0, 0, wd_hbmp->mp_y, wd_hbmp->dat, &bmi, DIB_RGB_COLORS
		        );
        	    BitBlt(
                    wd_hdc, 0, 0, wd_hbmp->mp_x, wd_hbmp->mp_y,
                    wd_mem_dc, 0, 0, SRCCOPY
                );
        	    SelectObject(wd_mem_dc, wd_mem_bmp);
                DeleteObject(wd_mem_bmp);
                DeleteDC(wd_mem_dc);
                EndPaint(hwnd, &wd_ps);
                break;
       	 	}
       	 	case WM_ERASEBKGND: return 1;
       	 	case WM_SIZE: {
                DWORD curtm = GetTickCount();
                if (curtm < wd_lastresz + STDWD_RESZ_DELAY) break;
                wd_lastresz = curtm;
                int32_t x = LOWORD(lparam), y = HIWORD(lparam);
                if (x < wd_min_x || y < wd_min_y) break;
                cfgbufsz(x, y);
                clk_edge_x = x - clk_framesz - 1;
                clk_edge_y = y - clk_framesz - 1;
                SendMessage(wd_hwnd, STDWD_FXRESZ, lparam, 0);
				break;
			}
			case STDWD_FXRESZ: return 1;
			case WM_SETCURSOR: { cursor4wd(LOWORD(lparam)); return 1; }
			case WM_NCHITTEST: {
                int32_t x = LOWORD(lparam), y = HIWORD(lparam);
                return mousehit(x, y);
            }
            case WM_COMMAND: {
                switch (LOWORD(wparam)) {
                    case 1001: { DestroyWindow(hwnd); break; }
                    case 1002: { ShowWindow(hwnd, SW_MINIMIZE); break; }
                    case 1003: {
                        int32_t ncmdshow;
                        if (IsZoomed(wd_hwnd)) ncmdshow = SW_RESTORE;
                        else ncmdshow = SW_MAXIMIZE;
                        ShowWindow(wd_hwnd, ncmdshow);
                        break;
                    }
                }
                break;
            }
            case WM_ACTIVATE: { update(); break; }
			case WM_CLOSE: { DestroyWindow(hwnd); break; }
			case WM_DESTROY: { PostQuitMessage(0); break; }
			default: {
                if (wd_exceptmsglst->find(msg)) return 1;
                return wd_procex(hwnd, msg, wparam, lparam);
            }
		}
		return 0;
	}
	void randclsnm() {
	    const uint8_t header_len = strlen(STDWD_DFT_CLSNM_HEADER);
	    strcpy(wd_wdclsnm, STDWD_DFT_CLSNM_HEADER);
	    DWORD pid = GetCurrentProcessId(), tick = GetTickCount();
        srand(pid ^ tick);
	    char tmpchr;
	    for (uint8_t i = header_len; i < STDWD_CLSNM_LEN - 1; i++) {
	        tmpchr = rand() % 62;
            if (tmpchr < 26) tmpchr += 'A';
            else if (tmpchr < 52) tmpchr += 'a' - 26;
            else tmpchr += '0' - 52;
            wd_wdclsnm[i] = tmpchr;
        }
        wd_wdclsnm[STDWD_CLSNM_LEN - 1] = '\0';
    }
	void wd_reg() {
		wd_wc.cbSize        = sizeof(WNDCLASSEX);
        wd_wc.style         = 0;
    	wd_wc.lpfnWndProc   = wd_proc;
    	wd_wc.cbClsExtra    = 0;
    	wd_wc.cbWndExtra    = 0;
    	wd_wc.hInstance     = wd_hinstance;
    	wd_wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    	wd_wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    	wd_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    	wd_wc.lpszMenuName  = NULL;
    	wd_wc.lpszClassName = wd_wdclsnm;
    	wd_wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
		if (!RegisterClassEx(&wd_wc)) {
        	MessageBox(
          	    NULL, TEXT("Window registration failed"), TEXT("ERROR"),
            	MB_ICONERROR | MB_OK
			);
			#ifndef CYXSPGDI_IGNORE_ERROR
        	exit(0);
        	#endif
    	}
	}
	void wd_cre() {
	    DWORD transptg = wd_settings.transp_invoked ? WS_EX_LAYERED : 0;
        wd_hwnd = CreateWindowEx(
        	wd_settings.ex | transptg, wd_wdclsnm, TEXT(wd_wdnm),
            wd_settings.base, wd_settings.pop_x, wd_settings.pop_y,
            wd_hbmp->mp_x, wd_hbmp->mp_y, NULL, NULL, wd_hinstance, this
		);
		if (wd_hwnd == NULL) {
        	MessageBox(
            	NULL, TEXT("Window creation failed"), TEXT("ERROR"),
            	MB_ICONERROR | MB_OK
			);
			#ifndef CYXSPGDI_IGNORE_ERROR
        	exit(0);
        	#endif
    	}
    	ShowWindow(wd_hwnd, wd_ncmdshow);
    	UpdateWindow(wd_hwnd);
	}
	static DWORD WINAPI wd_main(LPVOID lpParam) {
        STDWD* pthis = (STDWD*)lpParam;
        return pthis->wd_main_member();
    }
    DWORD WINAPI wd_main_member() {
		cfgbufsz(wd_hbmp->mp_x, wd_hbmp->mp_y);
		initbuf(STDWD_DFT_GRAY);
    	wd_reg();
    	wd_cre();
    	MSG msg;
    	wd_nready = 0;
    	while (GetMessage(&msg, NULL, 0, 0) > 0) {
        	TranslateMessage(&msg);
        	DispatchMessage (&msg);
    	}
    	delbuf();
    	return msg.wParam;
	}
	HANDLE wd_main_nblock(DWORD* threadid) {
		wd_hd = CreateThread(NULL, 0, wd_main, this, 0, threadid);
		return wd_hd;
	}
	void err(char* errinfo) {
	    char errcode[11];
	    itoa(GetLastError(), errcode, 10);
	    strcat(errinfo, errcode);
	    MessageBox(
            wd_hwnd, errinfo, TEXT("ERROR"), MB_ICONERROR | MB_OK
		);
		#ifndef CYXSPGDI_IGNORE_ERROR
		exit(0);
		#endif
    }
	bool isrunning() {
	    if (wd_nready) return 0;
	    DWORD res = WaitForSingleObject(wd_hd, 0);
        switch (res) {
            case WAIT_OBJECT_0: {
                CloseHandle(wd_hd);
                wd_nready = 1;
                return 0;
            }
            case WAIT_TIMEOUT: return 1;
            case WAIT_FAILED: {
	            char errinfo[48] = "Fatal error occured while seeking: ";
	            err(errinfo);
                break;
            }
        }
        return 0;
    }
    void join() {
        if (wd_nready) return;
        WaitForSingleObject(wd_hd, INFINITE);
        CloseHandle(wd_hd);
        wd_nready = 1;
    }
    void exit(bool force = 0) {
        if (wd_nready) return;
        bool failed = 1;
        if (force) {
            if (TerminateProcess(wd_hd, STDWD_EXTERNAL_EXIT)) failed = 0;
        } else {
            if (wd_hwnd && IsWindowVisible(wd_hwnd)) {
                PostMessage(wd_hwnd, WM_CLOSE, 0, 0);
                DWORD res = WaitForSingleObject(wd_hd, STDWD_WAIT_TIMEOUT);
                if (res == WAIT_OBJECT_0) failed = 0;
            }
        }
        if (failed) {
            char errinfo[48] = "Fatal error occured while exiting: ";
            err(errinfo);
        }
        wd_nready = 1;
        CloseHandle(wd_hd);
    }
	void update() {
	    if (wd_settings.transp_invoked) update_alpha();
	    else {
	        InvalidateRect(wd_hwnd, NULL, FALSE);
		    UpdateWindow(wd_hwnd);
        }
	}
//### CONFIG ###################################################################
    inline void setprocex(LRESULT (*procex)(HWND, UINT, WPARAM, LPARAM)) {
        wd_procex = procex;
    }
    inline void setredrex(LRESULT (*redrex)(HWND, UINT, WPARAM, LPARAM)) {
        wd_redrex = redrex;
    }
    void cfgstl(STDWD_CRE setting4wd, bool reset = 0) {
        memcpy(&wd_settings, &setting4wd, sizeof(STDWD_CRE));
        if (!wd_nready && reset) {
            LONG_PTR style = GetWindowLongPtr(wd_hwnd, GWL_STYLE),
            stylex = GetWindowLongPtr(wd_hwnd, GWL_EXSTYLE);
            style &= ~setting4wd.base_cfg_del;
            style |= setting4wd.base_cfg_add;
            stylex &= ~setting4wd.ex_cfg_del;
            stylex |= setting4wd.ex_cfg_add;
            SetWindowLongPtr(wd_hwnd, GWL_EXSTYLE, stylex);
            SetWindowLongPtr(wd_hwnd, GWL_STYLE, style);
            SetWindowPos(wd_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED
            );
        }
    }
    void exceptmsg(UINT msg) {
        wd_exceptmsglst->add(msg);
        wd_exceptmsglst->sort();
    }
    void exceptmsgs(UINT* msgs, UINT msgnm) {
        for (UINT i = 0; i < msgnm; i++)
            wd_exceptmsglst->add(msgs[i]);
        wd_exceptmsglst->sort();
    }
};


//### ADDITIONAL FUNCS #########################################################

struct FLP2D {
	float x, y;
	FLP2D(float x, float y): x(x), y(y) { }
	FLP2D(): x(0.f), y(0.f) { }
};

void straight(int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1,
		STDWD_RGB rgb, STDBMP* bmp, BYTE alpha = CYXSPGDI_DFTALPHA,
        bool overwrite = 1) {
    int32_t dx = abs(x_1 - x_0), dy = abs(y_1 - y_0);
	int32_t opx = (x_0 < x_1) ? 1 : -1, opy = (y_0 < y_1) ? 1 : -1,
		err = dx - dy, errc;
    int64_t offset;
    while (1)
    {
        offset = bmp->getoffset(x_0, y_0);
        bmp->dot(&offset, rgb, alpha, overwrite);
        if (!((x_0 ^ x_1) | (y_0 ^ y_1))) break;
        errc = err << 1;
        if (errc > -dy) {
		    err -= dy;
            x_0 += opx;
		}
		if (errc < dx) {
            err += dx;
            y_0 += opy;
		}
    }
}

inline void fl2dcut(FLP2D p_0, FLP2D p_1, float m, FLP2D* res) {
	#define GEN_COP(TYPE) res->TYPE = (p_1.TYPE - p_0.TYPE) * m + p_0.TYPE;
	GEN_COP(x) GEN_COP(y)
	#undef GEN_COP
}
__fastcall FLP2D bezier_point_iter(FLP2D* posl, size_t size, float m) {
    if (size == 1) return posl[0];
    size--;
    for (size_t i = 0; i < size; i++)
        fl2dcut(posl[i], posl[i + 1], m, &(posl[i]));
    return bezier_point_iter(posl, size, m);
}
FLP2D bezier_point(FLP2D* posl, size_t size, float m, FLP2D* posl_cpy) {
    memcpy(posl_cpy, posl, size * sizeof(FLP2D));
	FLP2D result = bezier_point_iter(posl_cpy, size, m);
	return result;
}
void bezier(FLP2D* posl, size_t size, float accu, STDWD_RGB rgb, STDBMP* bmp,
        BYTE alpha = CYXSPGDI_DFTALPHA, bool overwrite = 1) {
	float accuc = accu;
	FLP2D cur = posl[0], next, *posl_cpy = (FLP2D*)malloc(size * sizeof(FLP2D));
	while(accuc <= 1.f){
		next = bezier_point(posl, size, accuc, posl_cpy);
		straight(
			int32_t(cur.x), int32_t(cur.y), int32_t(next.x), int32_t(next.y),
			rgb, bmp, alpha, overwrite
		);
		cur = next;
		accuc += accu;
	}
	free(posl_cpy);
}

void fill_rect(int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1,
        STDWD_RGB rgb, BYTE alpha, STDBMP* bmp) {
    int32_t x_0_cpy = x_0;
    int64_t offset; int64_t* p_offset = &offset;
    for (; y_0 < y_1; y_0++) {
        offset = bmp->getoffset(x_0, y_0);
        for (; x_0 < x_1; x_0++) bmp->dot(p_offset, rgb, alpha, 1);
        x_0 = x_0_cpy;
    }
}


class TEXTREDRER
{
public:
    LOGFONT    redr_font;
    HFONT      redr_hfont;
    int32_t    redr_x, redr_y, sz_x, sz_y;
    COLORREF   redr_clr;
    HDC        redr_hdc;
    BITMAPINFO redr_bmi;
    void*      redr_bmpdat = nullptr;
    HBITMAP    redr_hbmp;
    RECT       redr_rect;
    BYTE*      redr_tar = nullptr;
    size_t     redr_tar_sz = 0;
    bool       redr_didinit_sz = 0, redr_didinit_font = 0, redr_targeted = 0;
//### CRE & DEL ################################################################
    TEXTREDRER() {
        redr_hdc = CreateCompatibleDC(NULL);
    }
    ~TEXTREDRER() {
        if (redr_didinit_sz) DeleteObject(redr_hbmp);
        if (redr_didinit_font) DeleteObject(redr_hfont);
        DeleteDC(redr_hdc);
    }
//### INIT #####################################################################
    void setfont(LOGFONT* ft, bool cpynm = 0, char* ftnm = "Consolas") {
        memcpy(&redr_font, ft, sizeof(LOGFONT));
        if (cpynm) memcpy(redr_font.lfFaceName, ftnm, LF_FACESIZE);
        if (redr_didinit_font) DeleteObject(redr_hfont);
        redr_hfont = CreateFontIndirect(&redr_font);
        SelectObject(redr_hdc, redr_hfont);
        redr_didinit_font = 1;
    }
    void setsz(int32_t x, int32_t y) {
        sz_x = x; sz_y = y;
        if (redr_didinit_sz) {
            DeleteObject(redr_hbmp);
            redr_bmpdat = nullptr;
        }
        redr_bmi = crestdbmi(sz_x, sz_y);
        redr_hbmp = CreateDIBSection(
            redr_hdc, &redr_bmi, DIB_RGB_COLORS, &redr_bmpdat, NULL, 0
        );
        SelectObject(redr_hdc, redr_hbmp);
        SetBkMode(redr_hdc, TRANSPARENT);
        redr_rect = {redr_x, redr_y, sz_x, sz_y};
        redr_tar_sz = (size_t)sz_x * sz_y * 4;
        redr_didinit_sz = 1;
    }
    void setrgb(STDWD_RGB rgb) {
        redr_clr = RGB(rgb.r, rgb.g, rgb.b);
        SetTextColor(redr_hdc, redr_clr);
    }
    void setpos(int32_t x, int32_t y) {
        redr_x = x; redr_y = y;
        if (redr_didinit_sz) redr_rect = {redr_x, redr_y, sz_x, sz_y};
    }
    void settar(BYTE* tar) {
        redr_tar = tar;
        redr_targeted = 1;
    }
//### CALL #####################################################################
    void cpytar() {
        if (redr_targeted && redr_didinit_sz)
            memcpy(redr_bmpdat, redr_tar, redr_tar_sz);
    }
    void loadtar() {
        if (redr_targeted && redr_didinit_sz)
            memcpy(redr_tar, redr_bmpdat, redr_tar_sz);
    }
    void str2buf(char* str, UINT format) {
        if (redr_didinit_sz && redr_didinit_font)
            DrawText(redr_hdc, str, -1, &redr_rect, format);
    }
    void str2buf_imm(BYTE* buf, char* str, UINT format) {
        if (redr_didinit_sz && redr_didinit_font) {
            memcpy(redr_bmpdat, buf, redr_tar_sz);
            DrawText(redr_hdc, str, -1, &redr_rect, format);
            memcpy(buf, redr_bmpdat, redr_tar_sz);
        }
    }
    void gettextsz(char* str, int32_t len, int32_t* sz_x, int32_t* sz_y) {
        SIZE sz;
        GetTextExtentPoint32(redr_hdc, str, len, &sz);
        *sz_x = sz.cx; *sz_y = sz.cy;
    }
    void index2pos(char* str, int32_t maxlen, int32_t* tar) {
        SIZE sz;
        for (int32_t i = 0; i < maxlen; i++) {
            GetTextExtentPoint32(redr_hdc, str, i + 1, &sz);
            tar[i] = sz.cx;
        }
    }
};

#pragma GCC diagnostic pop

#endif // #ifndef INCLUDED_CYXSPGDI

