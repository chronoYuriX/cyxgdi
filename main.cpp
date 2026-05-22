
#include <windows.h>
#include <string>

#include "cyxspgdi.cpp"

class TextWindow
{
private:
    STDWD* m_window;
    char* m_text;

public:
    TextWindow(int width, int height, char* text)
    {
        // 配置分层窗口
        STDWD_CRE setting4wd = {0};
        setting4wd.base = WS_POPUP | WS_VISIBLE;
        setting4wd.ex = WS_EX_LAYERED;
        setting4wd.pop_x = 100;
        setting4wd.pop_y = 100;
        setting4wd.transp_invoked = true;
        m_text = text;
        // 创建窗口
        m_window = new STDWD(width, height, "TextWindow", "TextClass");
        m_window->cfgstl(setting4wd, false);

        // 初始化为透明
        m_window->initbuf(255);
    }

    ~TextWindow()
    {
        if (m_window) delete m_window;
    }

    void ShowAndDraw()
    {
        // 启动窗口线程
        DWORD threadId;
        m_window->wd_main_nblock(&threadId);

        // 等待窗口创建
        Sleep(100);

        // 绘制文字
        DrawTextToBuffer();

        // 保持窗口运行
        while (m_window->isrunning()) {
            Sleep(100);
        }
    }

private:
    void DrawTextToBuffer()
    {
        // 获取HDC
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        // 创建与窗口缓冲区兼容的位图
        BITMAPINFO bmi = crestdbmi(
            m_window->wd_hbmp->mp_x,
            m_window->wd_hbmp->mp_y
        );

        HBITMAP hBitmap = CreateDIBitmap(
            hdcScreen, &bmi.bmiHeader, CBM_INIT,
            m_window->wd_hbmp->dat, &bmi, DIB_RGB_COLORS
        );

        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

        // 设置GDI文字属性
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextColor(hdcMem, RGB(0, 255, 0));

        // 创建字体
        HFONT hFont = CreateFont(
            36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "黑体"
        );
        HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

        // 绘制文字
        RECT rect = {20, 20, m_window->wd_hbmp->mp_x - 20, m_window->wd_hbmp->mp_y - 20};
        DrawText(hdcMem, m_text, -1, &rect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 获取更新后的像素数据
        GetDIBits(
            hdcMem, hBitmap, 0, m_window->wd_hbmp->mp_y,
            m_window->wd_hbmp->dat, &bmi, DIB_RGB_COLORS
        );

        // 设置Alpha通道（重要！）
        // 对于分层窗口，需要正确设置Alpha值
        for (int64_t i = 3; i < m_window->wd_hbmp->mp_sz; i += 4) {
            BYTE r = m_window->wd_hbmp->dat[i - 2];
            BYTE g = m_window->wd_hbmp->dat[i - 1];
            BYTE b = m_window->wd_hbmp->dat[i - 0];

            // 如果像素不是黑色（透明背景），设置为不透明
            if (r != 0 || g != 0 || b != 0) {
                m_window->wd_hbmp->dat[i] = 255;  // 完全不透明
            } else {
                m_window->wd_hbmp->dat[i] = 128;
            }
        }

        // 预乘Alpha通道（重要！）
        m_window->premult();

        // 清理
        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        // 更新分层窗口
        m_window->update_alpha();
    }
};

// 主函数
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprevinst, LPSTR lpcmdline, int ncmdshow)
{
    TextWindow window(500, 200, "Hello, STDWD 分层窗口！");
    window.ShowAndDraw();

    return 0;
}

