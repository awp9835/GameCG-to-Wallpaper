#include "Shlobj.h"
#include "windows.h"

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

        //以下目录必须存在，且至少含有一张任意壁纸
        constexpr auto onepicdir = L"D:\\wallpaperback"; 
        hr = SHCreateItemFromParsingName(onepicdir, 0, IID_PPV_ARGS(&item));
        if (SUCCEEDED(hr))
        {
                IShellItemArray* arr;
                hr = SHCreateShellItemArrayFromShellItem(item, IID_PPV_ARGS(&arr));
                if (SUCCEEDED(hr))
                {
                    hr = paper->SetSlideshow(arr); //开启自动切换壁纸
                    arr->Release();
                }
                item->Release();
        }
        //hr = paper->SetSlideshowOptions(DSO_SHUFFLEIMAGES, 1000 * 3600 * 24); //系统的默认切换设为1天1次
        paper->Release();
    }
    HWND find = 0,last = 0,last2 = 0;
    //寻找最后两个WorkerW
    while (true)
    {
        find = FindWindowExW(0, find, L"WorkerW", L"");
        if (find == 0) break;
        last2 = last;
        last = find;
    }
    //检验结果是否正确
    if (last == 0 || last2 == 0) return 0;
    else if (0 == FindWindowExW(last2, 0, L"SHELLDLL_DefView", L"")) return 0;
    find = FindWindowExW(0, last, L"Progman", L"Program Manager");//寻找放置位置
    if (find == 0) return 0;
    SetParent(hwnd, find);
    if (autoFullScreen)
    {
        RECT full;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &full, 0);
        MoveWindow(hwnd, 0, 0, full.right, full.bottom, TRUE);
        //PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    } 
    ShowWindow(last, SW_HIDE); //隐藏系统的上层壁纸
    return last;
}

int main()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    //左ctrl开启，在此之前，需要打开本程序，并且使游戏按F11全屏
    while (GetAsyncKeyState(VK_LCONTROL) >= 0) Sleep(10); 
    HWND hwnd = GetForegroundWindow();
    SetAsWallpaperWindow(hwnd, false);
    
    //右ctrl关闭，也可以直接点X退出，但是无法还原了
    while (GetAsyncKeyState(VK_RCONTROL) >= 0) Sleep(10); 
    SetParent(hwnd, NULL);

    CoUninitialize();
    CoUninitialize();
    return 0;
}
