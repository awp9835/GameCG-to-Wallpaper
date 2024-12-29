#include "Shlobj.h"
#include "windows.h"
#include <vector>
//将动画窗口设为壁纸窗口，用于实现动态壁纸。
//成功：返回被隐藏的一个系统窗口。
//失败：返回NULL
HWND SetAsWallpaperWindow(HWND hwnd, bool autoFullScreen = false)
{
    //Step1 开启自动切换壁纸，否则无法分离桌面图标和壁纸
    //main需要先 CoInitializeEx
    IDesktopWallpaper* paper;
    HRESULT hr = CoCreateInstance(__uuidof(DesktopWallpaper), 0, CLSCTX_LOCAL_SERVER, __uuidof(IDesktopWallpaper), (void**)&paper);
    if (SUCCEEDED(hr))
    { 
        IShellItem* item;
        //自动获取当前目录，作为还原壁纸目录 
        //printf("已自动获取当前工具所在目录\n");
        {
            DWORD pathlen = GetCurrentDirectoryW(0, nullptr);
            std::vector<WCHAR> path(pathlen);
            pathlen = GetCurrentDirectoryW(pathlen, path.data());
            hr = SHCreateItemFromParsingName(path.data(), 0, IID_PPV_ARGS(&item));
        }

        if (SUCCEEDED(hr))
        {
                IShellItemArray* arr;
                hr = SHCreateShellItemArrayFromShellItem(item, IID_PPV_ARGS(&arr));
                if (SUCCEEDED(hr))
                {
                    hr = paper->SetSlideshow(arr); //开启幻灯片壁纸模式，以分离图标和壁纸的窗口
                    arr->Release();
                }
                item->Release();
        }
        Sleep(100); //首次设置可能失败，所以延迟0.1s
        paper->Release();
    }
    HWND deskhwnd = 0,wpaperhwnd = 0,iconshwnd = 0;
    //寻找最后两个WorkerW
    while (true)
    {
        deskhwnd = FindWindowExW(0, deskhwnd, L"WorkerW", L"");
        if (deskhwnd == 0) break;
        iconshwnd = wpaperhwnd;
        wpaperhwnd = deskhwnd;
    }
    //检验结果是否正确
    if (wpaperhwnd == 0 || iconshwnd == 0) return 0;
    else if (0 == FindWindowExW(iconshwnd, 0, L"SHELLDLL_DefView", L"")) return 0;
    deskhwnd = FindWindowExW(0, wpaperhwnd, L"Progman", L"Program Manager");//寻找放置位置
    if (deskhwnd == 0) return 0;
    SetParent(hwnd, deskhwnd);
    if (autoFullScreen)
    {
        RECT full;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &full, 0);
        MoveWindow(hwnd, 0, 0, full.right, full.bottom, TRUE);
        //PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    } 
    ShowWindow(wpaperhwnd, SW_HIDE); //隐藏系统的上层壁纸
    return wpaperhwnd;
}
int main(int argc, char** argv)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HWND hwnd = NULL;
    if(argc >= 2) 
    {
        //命令行参数指定窗口标题
        printf("命令行参数指定了窗口标题：%s\n", argv[1]);
        hwnd = FindWindowA(nullptr, argv[1]) ;
        SetAsWallpaperWindow(hwnd, false);
    }
    else
    {
        //请切换到指定窗口，按左ctrl开启，在此之前，需要打开本程序，并且使游戏按F11全屏
        printf("请切换到指定窗口，开启全屏模式，按左Ctrl即可将其设置为桌面壁纸\n");
        while (GetAsyncKeyState(VK_LCONTROL) >= 0) Sleep(10); 
        hwnd = GetForegroundWindow();
        SetAsWallpaperWindow(hwnd, false);
    }
    printf("已尝试将指定窗口设置为壁纸\n");
    //按右ctrl还原窗口，并使用当前目录的壁纸，也可以不还原直接退出。
    printf("若要还原，请按右Ctrl，并将原初壁纸置于当前程序所在目录\n");
    while (GetAsyncKeyState(VK_RCONTROL) >= 0) Sleep(10); 
    SetParent(hwnd, NULL);

    CoUninitialize();
    CoUninitialize();
    return 0;
}
