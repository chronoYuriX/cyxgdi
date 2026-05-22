#include "cyxspgdi.cpp"
#define CYXSPFL_IGNORE_ERROR
#include "cyxspfl.cpp"


inline clrmix1(BYTE base, BYTE decor, BYTE rate) {
    base = BYTE((uint16_t(base) * (255 - rate) + 127) >> 8);
    decor = BYTE((uint16_t(decor) * rate + 127) >> 8);
    return base + decor;
}
STDWD_RGB clrmix(STDWD_RGB base, STDWD_RGB decor, BYTE rate) {
    #define GEN_MIX1(CHAN) clrmix1(base.CHAN, decor.CHAN, rate)
    return STDWD_RGB(GEN_MIX1(r), GEN_MIX1(g), GEN_MIX1(b));
    #undef GEN_MIX1
}


#ifndef CYXEXGDI_CUSTOM_COLOR
#define FRAME_EDGE_COLOR                   {34, 143, 253}
#define FRAME_EDGE_ALPHA                               93
#define FRAME_FILL_COLOR                  {168, 173, 221}
#define FRAME_FILL_ALPHA                               17
#define FRAME_INNR_COLOR                  {192, 194, 210}
#define FRAME_INNR_ALPHA                              195
#define FRAME_BTN_EDGE_FULLSCREEN_COLOR       {0, 250, 0}
#define FRAME_BTN_FILL_FULLSCREEN_COLOR       {0, 250, 0}
#define FRAME_BTN_EDGE_MINIMIZE_COLOR       {250, 250, 0}
#define FRAME_BTN_FILL_MINIMIZE_COLOR       {250, 250, 0}
#define FRAME_BTN_EDGE_CLOSE_COLOR            {250, 0, 0}
#define FRAME_BTN_FILL_CLOSE_COLOR            {250, 0, 0}
#define FRAME_BTN_EDGE_ALPHA                          235
#define FRAME_BTN_FILL_ALPHA                          113
#define FRAME_BTN_TEXTURE_COLOR           {221, 221, 221}
#define FRAME_BTN_TEXTURE_ALPHA                       248
#define FRAME_NAME_BACKGROUND_COLOR          {3, 52, 134}
#define FRAME_NAME_BACKGROUND_ALPHA                    76
#define FRAME_NAME_FONT_COLOR             {220, 220, 220}
#define FRAME_NAME_FONT_ALPHA                         255
#define FRAME_EDGE_MIX_RATE                            28
#define FRAME_BTN_EDGE_MIX_RATE                        68
#endif

#define FRAMED_STDWD_VOIDMSG                            0
#define FRAMED_STDWD_CLOSE                              1
#define FRAMED_STDWD_FULLSCREEN                         2
#define FRAMED_STDWD_MINIMIZE                           3

class FRAMED_STDWD;
void framed_stdwd_mainswd_proc(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, SUBWD* rootswd
);
struct FRAMED_STDWD_MMSTR {
    int32_t clk_x, clk_y;
    FRAMED_STDWD* base = nullptr;
    uint8_t forekey = FRAMED_STDWD_VOIDMSG;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
class FRAMED_STDWD
{
public:
    STDWD* buildin_wd;
    STDBMP* buildin_fluid_bmp;
    TEXTREDRER buildin_tredrer;
    FRAMED_STDWD_MMSTR buildin_clkmemstr;
    int32_t show_framesz = 3, show_topicsz = 18, show_btn_y = 13,
        show_btn_close_x = 40, show_btn_fullscreen_x = 20,
        show_btn_minimize_x = 20;
    bool fwd_isfullscreen = 0;
    uint32_t fwd_clientid;
//### cre & del ################################################################
    void reszclient() {
        buildin_wd->wd_hmainsubwd->swd_subwds[fwd_clientid]->resz(
            buildin_wd->wd_hbmp->mp_x - (show_framesz * 2) - 4,
            buildin_wd->wd_hbmp->mp_y - show_framesz - show_topicsz - 4
        );
    }
    FRAMED_STDWD(int32_t sz_x, int32_t sz_y, char* wdnm,
            char* clsnm = ".RAND") {
        int32_t screen_x = GetSystemMetrics(SM_CXSCREEN),
            screen_y = GetSystemMetrics(SM_CYSCREEN);
        DWORD pid = GetCurrentProcessId(), tick = GetTickCount();
        srand(pid ^ tick);
        STDWD_CRE stl = {
            .base = WS_POPUP,
            .ex = WS_EX_APPWINDOW,
            .pop_x = rand() % (screen_x - sz_x),
            .pop_y = rand() % (screen_y - sz_y)
        };
        stl.transp_invoked = 1;
        buildin_wd = new STDWD(sz_x, sz_y, wdnm, clsnm);
        buildin_wd->cfgstl(stl, 0);
        buildin_wd->wd_havebtns = 1;
        buildin_wd->clk_btns_x_0 = 5;
        buildin_wd->clk_btns_x_1 = show_btn_close_x + show_btn_fullscreen_x +
            show_btn_minimize_x + 6;
        buildin_wd->clk_btns_height = show_btn_y + 2;
        buildin_wd->clk_framesz = show_framesz;
        buildin_wd->clk_topicsz = show_topicsz + 1;
        buildin_wd->wd_min_x = buildin_wd->clk_btns_x_1;
        buildin_wd->wd_min_y = show_topicsz + 1;
        UINT handledmsgs[2] = {WM_LBUTTONDOWN, WM_LBUTTONUP};
        buildin_wd->exceptmsgs(handledmsgs, 2);
        buildin_wd->wd_hmainsubwd->setswdproc(framed_stdwd_mainswd_proc);
        buildin_wd->wd_hmainsubwd->setmemstruct((void*)&buildin_clkmemstr);
        fwd_clientid = buildin_wd->wd_hmainsubwd->cresubwd(
            0, 0, show_framesz + 2, show_topicsz + 2, 1
        );
        reszclient();
        buildin_wd->wd_main_nblock(nullptr);
        buildin_fluid_bmp = new STDBMP(0, 0);
        buildin_clkmemstr.base = this;
        while (buildin_wd->wd_nready) Sleep(1);
    }
    ~FRAMED_STDWD() {
        delete buildin_wd;
    }
    void paintframe() {
        //### [ corners ] ###
        // corner WN
        STDWD_RGB rgb(0, 0, 0);
        int64_t offset = 0;
        int64_t* p_offset = &offset;
        #define DOT buildin_wd->dot(p_offset, rgb, 0, 1);
        DOT; DOT; DOT;
        offset = buildin_wd->getoffset(0, 2); DOT;
        // corner EN
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 3, 0);
        DOT; DOT; DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 1, 1); DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 1, 2); DOT;
        // corner WS
        offset = buildin_wd->getoffset(0, buildin_wd->wd_hbmp->mp_y - 1);
        DOT; DOT; DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 1, 1); DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 1, 2); DOT;
        // corner ES
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 3, buildin_wd->wd_hbmp->mp_y - 1
        ); DOT; DOT; DOT;
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 1, buildin_wd->wd_hbmp->mp_y - 2
        ); DOT;
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 1, buildin_wd->wd_hbmp->mp_y - 3
        ); DOT;
        #undef DOT
        //### [ edges ] ###
        rgb = FRAME_EDGE_COLOR;
        fill_rect( // edge top
            3, 0, buildin_wd->wd_hbmp->mp_x - 3, 1, rgb, FRAME_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect( // edge bottom
            3, buildin_wd->wd_hbmp->mp_y - 1, buildin_wd->wd_hbmp->mp_x - 3,
            buildin_wd->wd_hbmp->mp_y, rgb, FRAME_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect( // edge left
            0, 3, 1, buildin_wd->wd_hbmp->mp_y - 3, rgb, FRAME_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect( // edge right
            buildin_wd->wd_hbmp->mp_x - 1, 3, buildin_wd->wd_hbmp->mp_x,
            buildin_wd->wd_hbmp->mp_y - 3, rgb, FRAME_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        // edge WN
        #define DOT buildin_wd->dot(p_offset, rgb, FRAME_EDGE_ALPHA, 1);
        offset = buildin_wd->getoffset(1, 1); DOT; DOT;
        offset = buildin_wd->getoffset(1, 2); DOT;
        // edge EN
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 3, 1);
        DOT; DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 2, 2); DOT;
        // edge WS
        offset = buildin_wd->getoffset(1, buildin_wd->wd_hbmp->mp_y - 2);
        DOT; DOT;
        offset = buildin_wd->getoffset(1, buildin_wd->wd_hbmp->mp_y - 3); DOT;
        // edge ES
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 3, buildin_wd->wd_hbmp->mp_y - 2
        ); DOT; DOT;
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 2, buildin_wd->wd_hbmp->mp_y - 3
        ); DOT;
        #undef DOT
        //### [ fill ] ###
        rgb = FRAME_FILL_COLOR;
        #define DOT buildin_wd->dot(p_offset, rgb, FRAME_FILL_ALPHA, 1);
        // 4 corners
        offset = buildin_wd->getoffset(2, 2); DOT;
        offset = buildin_wd->getoffset(buildin_wd->wd_hbmp->mp_x - 3, 2); DOT;
        offset = buildin_wd->getoffset(2, buildin_wd->wd_hbmp->mp_y - 3); DOT;
        offset = buildin_wd->getoffset(
            buildin_wd->wd_hbmp->mp_x - 3, buildin_wd->wd_hbmp->mp_y - 3
        ); DOT;
        #undef DOT
        // 4 edges
        fill_rect( // edge top
            3, 1, buildin_wd->wd_hbmp->mp_x - 3, 3, rgb, FRAME_FILL_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect(
            1, 3, buildin_wd->wd_hbmp->mp_x - 1, show_topicsz + 1, rgb,
            FRAME_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect( // edge left
            1, show_topicsz + 1, show_framesz + 1,
            buildin_wd->wd_hbmp->mp_y - 3, rgb, FRAME_FILL_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect( // edge right
            buildin_wd->wd_hbmp->mp_x - show_framesz - 1, show_topicsz + 1,
            buildin_wd->wd_hbmp->mp_x - 1, buildin_wd->wd_hbmp->mp_y - 3,
            rgb, FRAME_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect( // edge bottom
            3, buildin_wd->wd_hbmp->mp_y - show_framesz - 1,
            buildin_wd->wd_hbmp->mp_x - 3, buildin_wd->wd_hbmp->mp_y - 1,
            rgb, FRAME_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        //### [ inner edges ] ###
        rgb = FRAME_INNR_COLOR;
        fill_rect(
            show_framesz + 1, show_topicsz + 1,
            buildin_wd->wd_hbmp->mp_x - show_framesz - 1, show_topicsz + 2,
            rgb, FRAME_INNR_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            show_framesz + 1, show_topicsz + 2, show_framesz + 2,
            buildin_wd->wd_hbmp->mp_y - show_framesz - 1, rgb, FRAME_INNR_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect(
            buildin_wd->wd_hbmp->mp_x - show_framesz - 2, show_topicsz + 2,
            buildin_wd->wd_hbmp->mp_x - show_framesz - 1,
            buildin_wd->wd_hbmp->mp_y - show_framesz - 1, rgb, FRAME_INNR_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect(
            show_framesz + 2, buildin_wd->wd_hbmp->mp_y - show_framesz - 2,
            buildin_wd->wd_hbmp->mp_x - show_framesz - 2, 
            buildin_wd->wd_hbmp->mp_y - show_framesz - 1,
            rgb, FRAME_INNR_ALPHA, buildin_wd->wd_hbmp
        );
        //### [ title ] ###
        LOGFONT lf = {
            .lfHeight = show_topicsz - 2,
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
        buildin_tredrer.setfont(&lf, 1, "Tahoma");
        rgb = FRAME_NAME_FONT_COLOR;
        buildin_tredrer.setrgb(rgb);
        rgb = FRAME_NAME_BACKGROUND_COLOR;
        int32_t textsz_x, textsz_y;
        buildin_tredrer.gettextsz(
            buildin_wd->wd_wdnm, strlen(buildin_wd->wd_wdnm),
            &textsz_x, &textsz_y
        );
        buildin_fluid_bmp->resz(textsz_x + 6, textsz_y);
        ZeroMemory(buildin_fluid_bmp->dat, buildin_fluid_bmp->mp_sz);
        buildin_tredrer.setsz(textsz_x + 6, textsz_y);
        buildin_tredrer.setpos(3, 0);
        buildin_tredrer.str2buf_imm(
            buildin_fluid_bmp->dat, buildin_wd->wd_wdnm, DT_LEFT | DT_SINGLELINE
        );
        buildin_fluid_bmp->setalpha_cek0(
            rgb, FRAME_NAME_BACKGROUND_ALPHA, FRAME_NAME_FONT_ALPHA
        );
        buildin_wd->wd_hbmp->msk_overwrite_ncek(buildin_fluid_bmp, 6, 2);
        //### [ buttons' frame ] ###
        // "close"
        rgb = FRAME_BTN_EDGE_CLOSE_COLOR;
        fill_rect(
            buildin_wd->wd_hbmp->mp_x - show_btn_close_x - 5, 0,
            buildin_wd->wd_hbmp->mp_x - 5, 1, rgb, FRAME_BTN_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        STDWD_RGB rgb_decor(0, 0, 0); rgb_decor = FRAME_EDGE_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_EDGE_MIX_RATE);
        int32_t cur_edge_right = buildin_wd->wd_hbmp->mp_x - 5;
        int32_t cur_edge_left = cur_edge_right - show_btn_close_x;
        fill_rect(
            cur_edge_right - 1, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left, show_btn_y + 1, cur_edge_right, show_btn_y + 2, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        rgb_decor = FRAME_BTN_EDGE_FULLSCREEN_COLOR;
        rgb = FRAME_BTN_EDGE_CLOSE_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_BTN_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_left, 1, cur_edge_left + 1, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        // "fullscreen"
        cur_edge_right = cur_edge_left;
        cur_edge_left -= show_btn_fullscreen_x;
        rgb_decor = FRAME_EDGE_COLOR;
        rgb = FRAME_BTN_EDGE_FULLSCREEN_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_left, 0, cur_edge_right, 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left, show_btn_y + 1, cur_edge_right, show_btn_y + 2, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        rgb_decor = FRAME_BTN_EDGE_CLOSE_COLOR;
        rgb = FRAME_BTN_EDGE_FULLSCREEN_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_BTN_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_right - 1, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        rgb_decor = FRAME_BTN_EDGE_MINIMIZE_COLOR;
        rgb = FRAME_BTN_EDGE_FULLSCREEN_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_BTN_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_left, 1, cur_edge_left + 1, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        // "minimize"
        cur_edge_right = cur_edge_left;
        cur_edge_left -= show_btn_minimize_x;
        rgb_decor = FRAME_EDGE_COLOR;
        rgb = FRAME_BTN_EDGE_MINIMIZE_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_left, 0, cur_edge_right, 1, rgb, FRAME_BTN_EDGE_ALPHA,
            buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left, show_btn_y + 1, cur_edge_right, show_btn_y + 2, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left, 1, cur_edge_left + 1, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        rgb_decor = FRAME_BTN_EDGE_FULLSCREEN_COLOR;
        rgb = FRAME_BTN_EDGE_MINIMIZE_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_BTN_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_right - 1, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        //### [ buttons' filling ] ###
        cur_edge_right = buildin_wd->wd_hbmp->mp_x - 6;
        cur_edge_left = cur_edge_right - show_btn_close_x + 2;
        rgb = FRAME_BTN_FILL_CLOSE_COLOR;
        fill_rect(
            cur_edge_left, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        cur_edge_right = cur_edge_left - 2;
        cur_edge_left -= show_btn_fullscreen_x;
        rgb = FRAME_BTN_FILL_FULLSCREEN_COLOR;
        fill_rect(
            cur_edge_left, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        cur_edge_right = cur_edge_left - 2;
        cur_edge_left -= show_btn_minimize_x;
        rgb = FRAME_BTN_FILL_MINIMIZE_COLOR;
        fill_rect(
            cur_edge_left, 1, cur_edge_right, show_btn_y + 1, rgb,
            FRAME_BTN_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        //### [ buttons' texture ] ###
        // "close"
        rgb = FRAME_BTN_TEXTURE_COLOR;
        buildin_tredrer.setrgb(rgb);
        rgb = FRAME_BTN_FILL_CLOSE_COLOR;
        buildin_fluid_bmp->resz(show_btn_close_x - 4, show_btn_y);
        ZeroMemory(buildin_fluid_bmp->dat, buildin_fluid_bmp->mp_sz);
        buildin_tredrer.setsz(show_btn_close_x - 4, show_btn_y);
        buildin_tredrer.setpos(1, 0);
        lf.lfHeight = show_btn_y;
        lf.lfWidth  = show_btn_y - 3;
        lf.lfWeight = FW_MEDIUM;
        buildin_tredrer.setfont(&lf, 1, "Calibri");
        buildin_tredrer.str2buf_imm(
            buildin_fluid_bmp->dat, "X", DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );
        buildin_fluid_bmp->setalpha_cek0(
            rgb, FRAME_BTN_FILL_ALPHA, FRAME_BTN_TEXTURE_ALPHA
        );
        cur_edge_left = buildin_wd->wd_hbmp->mp_x - show_btn_close_x - 3;
        buildin_wd->wd_hbmp->msk_overwrite_ncek(
            buildin_fluid_bmp, cur_edge_left, 1
        );
        // "fullscreen"
        rgb = FRAME_BTN_TEXTURE_COLOR;
        cur_edge_left -= show_btn_fullscreen_x;
        cur_edge_right = ((cur_edge_left << 1) + show_btn_fullscreen_x) >> 1;
        fill_rect(
            cur_edge_right - 6, 4, cur_edge_right + 2, show_btn_y - 1, rgb,
            FRAME_BTN_TEXTURE_ALPHA, buildin_wd->wd_hbmp
        );
        rgb = FRAME_BTN_FILL_FULLSCREEN_COLOR;
        fill_rect(
            cur_edge_right - 5, 6, cur_edge_right + 1, show_btn_y - 2, rgb,
            FRAME_BTN_FILL_ALPHA, buildin_wd->wd_hbmp
        );
        // "minimize"
        cur_edge_left -= show_btn_minimize_x;
        cur_edge_right = ((cur_edge_left << 1) + show_btn_minimize_x) >> 1;
        rgb = FRAME_BTN_TEXTURE_COLOR;
        fill_rect(
            cur_edge_right - 6, show_btn_y - 3, cur_edge_right + 2,
            show_btn_y - 1, rgb, FRAME_BTN_TEXTURE_ALPHA, buildin_wd->wd_hbmp
        );
        // additional paintings
        cur_edge_left = buildin_wd->wd_hbmp->mp_x - show_btn_close_x -
            show_btn_fullscreen_x - show_btn_minimize_x - 6;
        cur_edge_right = buildin_wd->wd_hbmp->mp_x - 5;
        rgb = FRAME_FILL_COLOR;
        rgb_decor = FRAME_EDGE_COLOR;
        rgb = clrmix(rgb, rgb_decor, FRAME_EDGE_MIX_RATE);
        fill_rect(
            cur_edge_right, 1, cur_edge_right + 1, show_btn_y + 2, rgb,
            FRAME_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left + 1, show_btn_y + 2, cur_edge_right, show_btn_y + 3,
            rgb, FRAME_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
        fill_rect(
            cur_edge_left, 1, cur_edge_left + 1, show_btn_y + 2, rgb,
            FRAME_EDGE_ALPHA, buildin_wd->wd_hbmp
        );
    }
    void premultframe() {
        int32_t client_x_0 = show_framesz + 2,
            client_y_0 = show_topicsz + 2,
            client_x_1 = buildin_wd->wd_hbmp->mp_x - show_framesz - 2,
            client_y_1 = buildin_wd->wd_hbmp->mp_y - show_framesz - 2;
        buildin_wd->premultzone(
            0, 0, buildin_wd->wd_hbmp->mp_x, client_y_0
        );
        buildin_wd->premultzone(
            0, client_y_0, client_x_0, client_y_1
        );
        buildin_wd->premultzone(
            client_x_1, client_y_0, buildin_wd->wd_hbmp->mp_x, client_y_1
        );
        buildin_wd->premultzone(
            0, client_y_1, buildin_wd->wd_hbmp->mp_x, buildin_wd->wd_hbmp->mp_y
        );
    }
    void premultclient() {
        buildin_wd->premultzone(
            show_framesz + 2, show_topicsz + 2,
            buildin_wd->wd_hbmp->mp_x - show_framesz - 2,
            buildin_wd->wd_hbmp->mp_y - show_framesz - 2
        );
    }
};
#pragma GCC diagnostic pop

uint8_t framed_stdwd_kscan(FRAMED_STDWD_MMSTR* mmstr) {
    if (mmstr->clk_y > mmstr->base->show_btn_y + 1) return FRAMED_STDWD_VOIDMSG;
    int32_t btn_right = mmstr->base->buildin_wd->wd_hbmp->mp_x - 5;
    int32_t btn_left = btn_right - mmstr->base->show_btn_close_x;
    if (btn_left <= mmstr->clk_x && mmstr->clk_x < btn_right)
        return FRAMED_STDWD_CLOSE;
    btn_right = btn_left;
    btn_left -= mmstr->base->show_btn_fullscreen_x;
    if (btn_left <= mmstr->clk_x && mmstr->clk_x < btn_right)
        return FRAMED_STDWD_FULLSCREEN;
    btn_right = btn_left;
    btn_left -= mmstr->base->show_btn_minimize_x;
    if (btn_left <= mmstr->clk_x && mmstr->clk_x < btn_right)
        return FRAMED_STDWD_MINIMIZE;
    return FRAMED_STDWD_VOIDMSG;
}

void framed_stdwd_mainswd_proc(HWND hwnd, UINT msg, WPARAM wparam,
        LPARAM lparam, SUBWD* rootswd) {
    FRAMED_STDWD_MMSTR* mmstr = (FRAMED_STDWD_MMSTR*)rootswd->swd_pmemstruct;
    switch (msg) {
        case WM_LBUTTONDOWN: {
            mmstr->clk_x = LOWORD(lparam); mmstr->clk_y = HIWORD(lparam);
            mmstr->forekey = framed_stdwd_kscan(mmstr);
            break;
        }
        case WM_LBUTTONUP: {
            if (mmstr->forekey == FRAMED_STDWD_VOIDMSG) break;
            mmstr->clk_x = LOWORD(lparam); mmstr->clk_y = HIWORD(lparam);
            if (framed_stdwd_kscan(mmstr) == mmstr->forekey) {
                switch (mmstr->forekey) {
                    case FRAMED_STDWD_CLOSE: {
                        SendMessage(
                            mmstr->base->buildin_wd->wd_hwnd, WM_CLOSE, 0, 0
                        );
                        break;
                    }
                    case FRAMED_STDWD_FULLSCREEN: {
                        if (mmstr->base->fwd_isfullscreen) {
                            SendMessage(
                                mmstr->base->buildin_wd->wd_hwnd,
                                WM_SYSCOMMAND, SC_RESTORE, 0
                            );
                            mmstr->base->fwd_isfullscreen = 0;
                        } else {
                            SendMessage(
                                mmstr->base->buildin_wd->wd_hwnd,
                                WM_SYSCOMMAND, SC_MAXIMIZE, 0
                            );
                            mmstr->base->fwd_isfullscreen = 1;
                        }
                        break;
                    }
                    case FRAMED_STDWD_MINIMIZE: {
                        SendMessage(
                            mmstr->base->buildin_wd->wd_hwnd,
                            WM_SYSCOMMAND, SC_MINIMIZE, 0
                        );
                        break;
                    }
                }
            } else mmstr->forekey = FRAMED_STDWD_VOIDMSG;
            break;
        }
        case STDWD_FXRESZ: {
            mmstr->base->paintframe();
            mmstr->base->premultframe();
            mmstr->base->buildin_wd->update_alpha();
            break;
        }
    }
    rootswd->distrimsg(hwnd, msg, wparam, lparam, rootswd);
}

class TEXTEDIT_SUBWD;
struct TEXTEDITWD_MMSTR {
    TEXTEDIT_SUBWD* editor_root;
    FRAMED_STDWD* renderer_root;
};

void textedit_subwd_redr(STDBMP*, SUBWD*);
void textedit_subwd_proc(HWND, UINT, WPARAM, LPARAM, SUBWD*);
class TEXTEDIT_SUBWD
{
public:
    SUBWD* tx_root = nullptr;
    STDBMP* tx_buf;
    TEXTEDITWD_MMSTR txmmstr;
    char* show_str = nullptr;
    TEXTEDIT_SUBWD(SUBWD* root, FRAMED_STDWD* renderer_root): tx_root(root) {
        txmmstr = {
            .editor_root = this,
            .renderer_root = renderer_root
        };
        root->setredrproc(textedit_subwd_redr);
        root->setswdproc(textedit_subwd_proc);
        root->setmemstruct((void*)&txmmstr);
        tx_buf = new STDBMP(root->swd_x, root->swd_y);
    }
    ~TEXTEDIT_SUBWD() {
        delete tx_buf;
    }
};

void textedit_subwd_redr(STDBMP* hbmp, SUBWD* rootswd) {
    TEXTEDITWD_MMSTR* txmmstr = (TEXTEDITWD_MMSTR*)(rootswd->swd_pmemstruct);
    LOGFONT lf = {
        .lfHeight = 16,
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
    txmmstr->renderer_root->buildin_tredrer.setfont(&lf, 1, "Consolas");
    txmmstr->renderer_root->buildin_tredrer.setsz(rootswd->swd_x, rootswd->swd_y);
    txmmstr->renderer_root->buildin_tredrer.setrgb(STDWD_RGB(0, 255, 0));
    txmmstr->renderer_root->buildin_tredrer.setpos(0, 0);
    ZeroMemory(
        txmmstr->editor_root->tx_buf->dat, txmmstr->editor_root->tx_buf->mp_sz
    );
    txmmstr->renderer_root->buildin_tredrer.str2buf_imm(
        txmmstr->editor_root->tx_buf->dat,
        "114514\n1919810\nchrono_yurix@outlook.com\n²âÊÔÊä³ö\nœyÔ‡Ý”³ö",
        DT_LEFT
    );
    bmp2fl(
        "lol_result.bmp", txmmstr->editor_root->tx_buf->dat,
        txmmstr->editor_root->tx_buf->mp_x,
        txmmstr->editor_root->tx_buf->mp_y,
        4, 0, 1
    );
    debug.print(txmmstr->editor_root->tx_buf->mp_x); debug.print(" ");
    debug.print(txmmstr->editor_root->tx_buf->mp_y); debug.print("\n");
    int32_t x_offset = 0, y_offset = 0;
    rootswd->iterxy(&x_offset, &y_offset);
    txmmstr->editor_root->tx_buf->setalpha1(255);
    rootswd->tar_hbmp->msk_overwrite_ncek(txmmstr->editor_root->tx_buf, x_offset, y_offset);
}

void textedit_subwd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
        SUBWD* rootswd) {
    switch (msg)
    {
        case WM_MOUSEWHEEL: {
            debug.print("mswl");
            break;
        }
    }
}

