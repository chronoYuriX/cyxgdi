#include "cyxexgdi.cpp"


void client_redr(STDBMP* hbmp, SUBWD* rootswd) {
    STDWD_RGB rgb(3, 3, 3);
    int64_t offset;
    for (int32_t y = 0; y < rootswd->swd_y; y++) {
        offset = rootswd->getabsoffset(0, y);
        for (int32_t x = 0; x < rootswd->swd_x; x++)
            rootswd->dot(&offset, rgb, 230, 1);
    }
}

struct CLICKMEMSTRUCT {
    bool mousedown = 0;
    STDWD* pmainwd = nullptr;
    FRAMED_STDWD* psuper = nullptr;
};

void client_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, SUBWD* rootswd) {
    CLICKMEMSTRUCT* pclkmmstr = (CLICKMEMSTRUCT*)(rootswd->swd_pmemstruct);
    switch (msg) {
        case WM_MOUSEMOVE: {
            int32_t x = LOWORD(lparam), y = HIWORD(lparam);
            if (!rootswd->isabsxyinswd(x, y)) break;
            rootswd->absxy2curxy(&x, &y);
            if (pclkmmstr->mousedown) {
                STDWD_RGB rgb(255, 0, 0);
                int64_t offset = rootswd->getabsoffset(x, y);
                rootswd->dot_nptr(offset, rgb, 150, 1);
                pclkmmstr->pmainwd->premult3(offset);
                pclkmmstr->pmainwd->update();
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            pclkmmstr->mousedown = 1;
            int32_t x = LOWORD(lparam), y = HIWORD(lparam);
            if (!rootswd->isabsxyinswd(x, y)) break;
            rootswd->absxy2curxy(&x, &y);
            STDWD_RGB rgb(255, 0, 0);
            int64_t offset = rootswd->getabsoffset(x, y);
            rootswd->dot_nptr(offset, rgb, 150, 1);
            pclkmmstr->pmainwd->premult3(offset);
            pclkmmstr->pmainwd->update();
            break;
        }
        case WM_LBUTTONUP: {
            pclkmmstr->mousedown = 0;
            break;
        }
        case STDWD_FXRESZ: {
            pclkmmstr->psuper->reszclient();
            rootswd->redr();
            pclkmmstr->psuper->premultclient();
            pclkmmstr->pmainwd->update_alpha();
            break;
        }
    }
    rootswd->distrimsg(hwnd, msg, wparam, lparam, rootswd);
}

int main() {
    char windowname[STDWD_WDNM_LEN] = "Result"; // silent -Wwrite-strings
    FRAMED_STDWD testwd(400, 300, windowname);
    SUBWD* client = testwd.buildin_wd->wd_hmainsubwd->swd_subwds[testwd.fwd_clientid];
    client->setredrproc(client_redr);
    CLICKMEMSTRUCT clkmmstr;
    clkmmstr.pmainwd = testwd.buildin_wd;
    clkmmstr.psuper = &testwd;
    testwd.buildin_wd->exceptmsg(WM_MOUSEMOVE);
    testwd.buildin_wd->wd_exceptmsglst->print();
    client->setmemstruct((void*)&clkmmstr);
    client->setswdproc(client_proc);
    testwd.paintframe();
    testwd.buildin_wd->wd_hmainsubwd->redr();
    FLP2D posl[4] = {
        {10.f, 30.f}, {50.f, 280.f}, {300.f, 250.f}, {340.f, 50.f}
    };
    bezier(posl, 4, .03125f, STDWD_RGB(0, 0, 200), testwd.buildin_wd->wd_hbmp);
    uint16_t tx_editor_id = client->cresubwd(200, 160, 100, 80, 0);
    TEXTEDIT_SUBWD editor_test(client->swd_subwds[tx_editor_id], &testwd);
    client->redr();
    testwd.buildin_wd->premult();
    testwd.buildin_wd->update();
    testwd.buildin_wd->join();
    return 0;
}

