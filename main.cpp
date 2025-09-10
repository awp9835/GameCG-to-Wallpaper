#include "Shlobj.h"
#include "windows.h"
#include <vector>
#define SET_WALLPAPER_HOTKEY VK_LCONTROL
#define RESET_WALLPAPER_HOTKEY VK_RCONTROL 
#define TASK_VIEW_COMPATIBILITY true
#define SET_KEY_CHECK_INTERVAL 10
#define STATE_CHECK_INTERVAL 100
#define RETRY_DELAY_MS 3000
#define RETRY_TIMES 3

//将动画窗口设为壁纸窗口，用于实现动态壁纸。
//成功：返回被隐藏的一个系统窗口。
//失败：返回NULL
HWND SetAsWallpaperWindow(HWND hwnd, bool autoFullScreen = false, bool taskViewCompatibility = TASK_VIEW_COMPATIBILITY)
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
        paper->Release();
    }
    HWND deskhwnd = 0,wpaperhwnd = 0,iconshwnd = 0;
    //寻找最后两个WorkerW
    
    for (int i = 0; i < RETRY_TIMES; i++)
    { 
        while (true)
        {
            deskhwnd = FindWindowExW(0, deskhwnd, L"WorkerW", L"");
            if (deskhwnd == 0) break;
            iconshwnd = wpaperhwnd;
            wpaperhwnd = deskhwnd;
        }
        //检验结果是否正确，否则等3秒钟再试一次
        //首次设置可能失败，所以延迟
        if (wpaperhwnd == 0 || iconshwnd == 0)
        {
            Sleep(RETRY_DELAY_MS);
            continue;
        }
        else break;
    } 
    if (wpaperhwnd == 0 || iconshwnd == 0) return 0;

    HWND defviewhwnd = FindWindowExW(iconshwnd, 0, L"SHELLDLL_DefView", L"");
    if (defviewhwnd == 0) return 0;
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
    if(TASK_VIEW_COMPATIBILITY)
    {
        while (GetAsyncKeyState(RESET_WALLPAPER_HOTKEY) >= 0)
        {
            //如果意外关闭了explorer或动态壁纸窗口，则自动退出
            if (!IsWindow(wpaperhwnd)) return NULL;
            if (!IsWindow(hwnd))
            {
                ShowWindow(wpaperhwnd, SW_SHOW);
                return NULL;
            }

            //使用任务视图后，出现Bug，是因为窗口Z序发生了变化，需要重置
            //GetParent无法正常使用，返回NULL，必须使用其他方法
            HWND znext = GetWindow(hwnd, GW_HWNDNEXT);
            if (znext == defviewhwnd) //if(znext != 0)
            { 
                SetParent(hwnd, deskhwnd);  
            }
            ShowWindow(wpaperhwnd, SW_HIDE);
            Sleep(STATE_CHECK_INTERVAL);
        }
    }
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
        if (SET_WALLPAPER_HOTKEY == VK_LCONTROL)        
            printf("请切换到指定窗口，开启全屏模式，长按左Ctrl即可将其设置为桌面壁纸\n");
        else 
            printf("请切换到指定窗口，开启全屏模式，长按自定义的SET_WALLPAPER_HOTKEY即可将其设置为桌面壁纸\n");

        while (GetAsyncKeyState(SET_WALLPAPER_HOTKEY) >= 0) Sleep(SET_KEY_CHECK_INTERVAL);
        hwnd = GetForegroundWindow();
        printf("---------------------------------------------------\n");
        printf("已尝试将指定窗口设置为壁纸，如未成功请重新运行本程序\n");
        printf("---------------------------------------------------\n");
        if (RESET_WALLPAPER_HOTKEY == VK_RCONTROL)
            printf("若要还原，请长按右Ctrl，并将原初壁纸置于当前程序所在目录\n");
        else
            printf("若要还原，请长按自定义的RESET_WALLPAPER_HOTKEY，并将原初壁纸置于当前程序所在目录\n");
        if (!TASK_VIEW_COMPATIBILITY)  while (GetAsyncKeyState(RESET_WALLPAPER_HOTKEY) >= 0) Sleep(STATE_CHECK_INTERVAL);
        else printf("---------------------------------------------------\n请将本控制台移到其他任务视图\n");
        SetAsWallpaperWindow(hwnd, false);
    }

    //还原窗口，并使用当前目录的壁纸，也可以不还原直接退出。

    SetParent(hwnd, NULL);
    printf("---------------------------------------------------\n");
    printf("已退出，请重新设置壁纸\n");
    CoUninitialize();
    CoUninitialize();
    return 0;
}