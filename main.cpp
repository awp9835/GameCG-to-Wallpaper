#include "Shlobj.h"
#include "windows.h"
#include <stdio.h>

#define SET_WALLPAPER_HOTKEY VK_LCONTROL
#define RESET_WALLPAPER_HOTKEY VK_PAUSE 
#define TASK_VIEW_COMPATIBILITY true
#define SET_KEY_CHECK_INTERVAL 10
#define STATE_CHECK_INTERVAL 100
#define RETRY_DELAY_MS 1000
#define RETRY_TIMES 10

//将动画窗口设为壁纸窗口，用于实现动态壁纸。
//成功：返回分离的最后一个WorkerW窗口句柄。
//失败：返回NULL
HWND SetAsWallpaperWindow(HWND hwnd, bool autoFullScreen = false, bool taskViewCompatibility = TASK_VIEW_COMPATIBILITY)
{
    HWND prgmhwnd = FindWindowExW(0, 0, L"Progman", L"Program Manager"); //寻找放置位置
    if (prgmhwnd == 0) return 0; //Explorer关闭，失败

    //分离桌面图标和壁纸
    //我们先进的SendMessageW已经完全取代了落后的幻灯片放映模式
    SendMessageW(prgmhwnd, WM_USER + 300, 0, 0);

    //Win11 24H2实现动态壁纸和更早的版本完全不同
    //Win11 24H2版本，动态壁纸窗口要放在WorkerW里面
    //更早的版本，动态壁纸窗口要放在Program Manager里面，并且隐藏WorkerW
    //Program Manager, SHELLDLL_DefView, WorkerW位置关系也不同，可使用Spy++查看
    // 
    //先假设系统版本是Win11 24H2以上，如果不是则使用else分支

    HWND defviewhwnd = FindWindowExW(prgmhwnd, 0, L"SHELLDLL_DefView", L"");
    if (defviewhwnd != 0)
    {
        //SHELLDLL_DefView在Program Manager里，是Win11 24H2以上
        HWND wpaperhwnd = FindWindowExW(prgmhwnd, 0, L"WorkerW", L"");
        if (wpaperhwnd == 0) return 0; //盗版系统可能失败

        //自动全屏，一点用都没有，但又不能没有
        if (autoFullScreen)
        {
            RECT full;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &full, 0);
            MoveWindow(hwnd, 0, 0, full.right, full.bottom, TRUE);
            //PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }

        //设置壁纸
        SetParent(hwnd, wpaperhwnd);

        //刷新桌面图标
        ShowWindow(defviewhwnd, SW_HIDE);
        Sleep(0);
        ShowWindow(defviewhwnd, SW_SHOW);

        //任务视图兼容模式下持续监听
        if (taskViewCompatibility)
        {
            while (GetAsyncKeyState(RESET_WALLPAPER_HOTKEY) >= 0)
            {
                //如果意外关闭了explorer或动态壁纸窗口，则自动退出
                //如果开了自动换壁纸那么Win11 24H2 WorkerW的句柄是会变的
                //必须不停监测，否则动态壁纸就不见了
                if (!IsWindow(prgmhwnd) || !IsWindow(defviewhwnd)) return 0;
                if (!IsWindow(hwnd)) return wpaperhwnd;

                //系统换壁纸的时候会同时出现2个Worker，新的在上面
                //旧的会销毁，如果拦截不到，动态壁纸就无了!
                HWND checkworkw = FindWindowExW(prgmhwnd, 0, L"WorkerW", L"");
                if (checkworkw != wpaperhwnd)
                {
                    wpaperhwnd = checkworkw;
                    SetParent(hwnd, wpaperhwnd);
                }
                if (wpaperhwnd == 0) return 0; //Explorer关闭，可能发生，有必要写成2个不同分支处理
                else if (!IsWindow(wpaperhwnd)) return 0; //系统自动换壁纸功能导致程序崩溃，只要不卡就不会发生

                Sleep(STATE_CHECK_INTERVAL);
            }
        }
        return wpaperhwnd;
    }
    else
    {
        //defviewhwnd = 0; //defviewhwnd == 0;
        //SHELLDLL_DefView在倒数第二个WorkerW里，是旧版Windows
        HWND iconshwnd = FindWindowExW(0, 0, L"WorkerW", L"");
        while (iconshwnd != 0)
        {
            defviewhwnd = FindWindowExW(iconshwnd, 0, L"SHELLDLL_DefView", L"");
            if (defviewhwnd != 0) break;
            iconshwnd = FindWindowExW(0, iconshwnd, L"WorkerW", L"");
        }
        if (defviewhwnd == 0) return 0; //Explorer关闭，失败

        HWND wpaperhwnd = FindWindowExW(0, iconshwnd, L"WorkerW", L"");
        if (wpaperhwnd == 0) return 0; //盗版系统可能失败

        //自动全屏，一点用都没有，但又不能没有
        if (autoFullScreen)
        {
            RECT full;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &full, 0);
            MoveWindow(hwnd, 0, 0, full.right, full.bottom, TRUE);
            //PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }

        //设置壁纸
        SetParent(hwnd, prgmhwnd);
        ShowWindow(wpaperhwnd, SW_HIDE); //隐藏系统的上层壁纸

        //任务视图兼容模式下持续监听
        if (taskViewCompatibility)
        {
            while (GetAsyncKeyState(RESET_WALLPAPER_HOTKEY) >= 0)
            {
                //如果意外关闭了explorer或动态壁纸窗口，则自动退出

                if (!IsWindow(prgmhwnd)) return 0;
                if (!IsWindow(defviewhwnd))
                {
                    //原窗口还能抢救一下
                    SetParent(hwnd, 0);
                    return 0;
                }
                if (!IsWindow(hwnd))
                {
                    ShowWindow(wpaperhwnd, SW_SHOW);
                    return wpaperhwnd;
                }
                //使用任务视图后，窗口Z序和关系发生变化，需要重置
                //GetParent无法正常使用，返回NULL，必须使用其他方法
                HWND znext = GetWindow(hwnd, GW_HWNDNEXT);
                if (znext == defviewhwnd)
                {
                    SetParent(hwnd, prgmhwnd);
                }
                //旧版Windows11/10里WorkerW的句柄是不变的，但Windows7里WorkerW的句柄是变化的
                if (!IsWindow(wpaperhwnd))
                {
                    wpaperhwnd = FindWindowExW(0, iconshwnd, L"WorkerW", L"");
                }
                if (wpaperhwnd != 0) ShowWindow(wpaperhwnd, SW_HIDE);
                else
                {
                    //原窗口还能抢救一下
                    SetParent(hwnd, 0);
                    return 0;
                }
                Sleep(STATE_CHECK_INTERVAL);
            }
        }
        return wpaperhwnd;
    }
}
void hint()
{
    printf("---------------------------------------------------\n");
    printf("尝试将指定窗口设置为壁纸，如未成功请重新运行本程序\n");
    printf("---------------------------------------------------\n");
    if (RESET_WALLPAPER_HOTKEY == VK_RCONTROL)
        printf("若要还原壁纸，请长按右Ctrl，或关闭动态壁纸播放程序\n");
    else if (RESET_WALLPAPER_HOTKEY == VK_PAUSE)
        printf("若要还原壁纸，请长按键盘右上角的Pause键，或关闭动态壁纸播放程序\n");
    else
        printf("若要还原壁纸，请长按自定义的RESET_WALLPAPER_HOTKEY，或关闭动态壁纸播放程序\n");
}
int main(int argc, char** argv)
{
    HWND hwnd = 0;
    if (argc >= 2)
    {
        //命令行参数指定窗口标题
        printf("命令行参数指定了窗口标题：%s\n", argv[1]);
        for (int i = 0; i < RETRY_TIMES; i++)
        {
            hwnd = FindWindowA(nullptr, argv[1]);
            if (hwnd != 0) break;
            Sleep(RETRY_DELAY_MS);
        }
        hint();
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
        hint();

        hwnd = GetForegroundWindow();
        SetAsWallpaperWindow(hwnd, false);

        //非任务视图兼容模式需要手动阻塞
        if (!TASK_VIEW_COMPATIBILITY)  while (GetAsyncKeyState(RESET_WALLPAPER_HOTKEY) >= 0) Sleep(STATE_CHECK_INTERVAL);
        else printf("---------------------------------------------------\n请将本控制台移到其他任务视图\n");
    }

    //还原窗口，并使用当前目录的壁纸，也可以不还原直接退出。
    SetParent(hwnd, NULL);
    printf("---------------------------------------------------\n");
    printf("已退出，请重新设置壁纸\n");
    return 0;
}