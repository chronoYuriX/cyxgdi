#include <windows.h>
#include <math.h>


const char window_name[] = "114514";
const int window_width = 400, window_height = 300;
BYTE buffer[window_width * window_height * 4];

void randclsnm(char* clsnm) {
    DWORD pid = GetCurrentProcessId(), tick = GetTickCount();
    srand(pid ^ tick);
    for (int i = 0; i < 255; i++) {
        char tmpchr = rand() % 62;
        if (tmpchr < 26) tmpchr += 'A';
        else if (tmpchr < 52) tmpchr += 'a' - 26;
        else tmpchr += '0' - 52;
        clsnm[i] = tmpchr;
    }
    clsnm[255] = '\0';
}
char window_classname[256];

BITMAPINFO crestdbmi(int x, int y) {
    BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
   	bmi.bmiHeader.biHeight = -y;
   	bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = (long long)x * y * 4;
    return bmi;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		case WM_PAINT: {
		    PAINTSTRUCT wd_ps;
		    HDC wd_hdc = BeginPaint(hwnd, &wd_ps), wd_mem_dc = CreateCompatibleDC(wd_hdc);
        	HBITMAP wd_mem_bmp = CreateCompatibleBitmap(wd_hdc, window_width, window_height);
            SelectObject(wd_mem_dc, wd_mem_bmp);
            BITMAPINFO bmi = crestdbmi(window_width, window_height);
    	    SetDIBitsToDevice(wd_mem_dc, 0, 0, window_width, window_height, 0, 0, 0, window_height, buffer, &bmi, DIB_RGB_COLORS);
        	BitBlt(wd_hdc, 0, 0, window_width, window_height, wd_mem_dc, 0, 0, SRCCOPY);
        	SelectObject(wd_mem_dc, wd_mem_bmp);
            DeleteObject(wd_mem_bmp);
            DeleteDC(wd_mem_dc);
            EndPaint(hwnd, &wd_ps);
            break;
        }
		case WM_CLOSE: case WM_DESTROY: case WM_COMMAND: case WM_SYSCOMMAND: return 1;
		default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void window_register(HINSTANCE hinstance)
{
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX), .style = 0, .lpfnWndProc = window_proc, .cbClsExtra = 0, .cbWndExtra = 0,
        .hInstance = hinstance, .hIcon = LoadIcon(NULL,IDI_APPLICATION), .hCursor = LoadCursor(NULL,IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1), .lpszMenuName = window_classname, .lpszClassName = window_classname,
        .hIconSm = LoadIcon(NULL,IDI_APPLICATION)
    };
    RegisterClassEx(&wc);
}

HWND window_create(HINSTANCE hinstance, int ncmdshow) {
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE, window_classname, window_name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        window_width, window_height, NULL, NULL, hinstance, NULL
    );
    ShowWindow(hwnd, ncmdshow);
    UpdateWindow(hwnd);
    return hwnd;
}

int window_main(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow) {
    window_register(hinstance);
    HWND hwnd = window_create(hinstance,ncmdshow);
    char cnt = 0;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (cnt == 2) {
            SetWindowPos(
                hwnd, HWND_TOPMOST, rand() % (GetSystemMetrics(SM_CXSCREEN) - window_width), rand() % (GetSystemMetrics(SM_CYSCREEN) - window_height),
                window_width, window_height, 0
            );
            cnt = 0;
        } else cnt++;
    }
    return msg.wParam;
}

void mix(BYTE* r, BYTE* g, BYTE* b, int val) {
    val %= 1536;
    BYTE mode = val / 256, rate = val % 256;
    switch (mode) {
        case 0: { *r = 255; *g = rate; *b = 0; break; }
        case 1: { *r = 255 - rate; *g = 255; *b = 0; break; }
        case 2: { *r = 0; *g = 255; *b = rate; break; }
        case 3: { *r = 0; *g = 255 - rate; *b = 255; break; }
        case 4: { *r = rate; *g = 0; *b = 255; break; }
        default: { *r = 255; *g = 0; *b = 255 - rate; break; }
    }
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    while (FindWindowA(NULL, window_name) != NULL) Sleep(1);
    system("start 114514.exe");
    randclsnm(window_classname);
    const float k = 1536.f / sqrt(pow((float)window_height, 2.f) + pow((float)window_width, 2.f));
    for (int y = 0; y < window_height; y++)
        for (int x = 0; x < window_width; x++) {
            long long offset = ((long long)y * window_width + x) * 4;
            int d = int(sqrt((float)x * x + (float)y * y) * k);
            mix(buffer + offset + 2, buffer + offset + 1, buffer + offset, d);
        }
    while (1) window_main(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOW);
    return 114514;
}

