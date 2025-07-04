# GameCG-to-Wallpaper
将全屏游戏CG、视频、网页等任意窗口设置为动态壁纸！
Set any window such as Full Screen Game CG, Video, Web Page, etc. as dynamic Wallpaper!

0. 目前支持的系统 Windows10/11，必须激活，否则无法个性化。Windows7必须先手动开启自动更换壁纸。
1. 本工具适合可全屏的游戏，包括BlueStacks等模拟器，也适用于视频播放器、网页等其他软件。
2. 在使用前，请将本工具放置于原初壁纸所在的目录，再在桌面创建快捷方式。
3. 对以管理员权限运行的游戏或程序，要先以管理员身份运行本工具，否则无法生效。
4. 游戏的反作弊系统可能使本工具无效化。
5. 打开游戏，切换到想要的CG，进入全屏模式，再按左Ctrl，就把CG设为壁纸了，设置后游戏不可操作。
   或者打开视频，设置为循环播放模式，进入全屏模式，再按左Ctrl，就把视频设为壁纸了。
6. 如果关闭控制台，则只能通过任务管理器结束进程退出壁纸，切换任务视图会使设置失灵。
7. 如果未关闭控制台，还可以按右Ctrl（或自定义按键），将游戏、视频、网页等恢复到可操作状态。
8. 可使用命令行或终端开启本程序，接受1个可选参数：设为壁纸的程序的名称（可在任务管理器中查看）。
9. 建议使用任务视图，将控制台移到其他桌面上。
10. 关闭控制台后，再次设置动态壁纸必须重新打开本程序。
11. 修改源代码中的宏SET_WALLPAPER_HOTKEY/RESET_WALLPAPER_HOTKEY，重新编译程序（需VS2019及以上版本新建控制台项目），可自定义设置/还原热键。 
12. 若要开机启动，必须编写cmd或bat批处理脚本，再将该脚本或其快捷方式添加至注册表启动项或开始菜单启动目录。提示：向AI提问“如何用使用命令行全屏播放视频”。   
	
## 开机启动方法：   
将一个视频设置为开机启动循环播放壁纸：   
### PotPlayer播放器（功能不全）
1. 手动设置默认全屏和默认循环播放。   
2. 下载Release或编译程序。   
3. 新建一个文本文件，编写脚本，然后改为.bat文件。   
注意：要修改为正确的路径，若视频打开较慢，则timeout要设置更长一些；使用 start /b 不会出现控制台，不影响热键恢复。
```
	start "" "C:\Program Files\DAUM\PotPlayer\PotPlayerMini64.exe" /filedlg "xxx\YourVideo.mp4" 
	timeout /t 2 
	start /b "" "xxx\setwallpaper_lctrl_pause.exe"  "YourVideo.mp4 - PotPlayer"
```
4. 将脚本添加到系统启动项。提示：向AI提问如何操作。
5. 使用系统音量合成器调节背景音量。   

### VLC播放器（推荐）
不需要手动设置默认全屏和循环播放，可直接使用命令行，参考文档：https://wiki.videolan.org/VLC_command-line_help/   
使用--no-audio选项可使视频永久静音。
```
	start "" "C:\Program Files\VideoLAN\VLC\vlc.exe" --loop --no-osd --fullscreen "xxx\YourVideo.mp4" 
	timeout /t 2 
	start /b "" "xxx\setwallpaper_lctrl_pause.exe"  "YourVideo.mp4 - VLC media player"
```
托盘图标可随时开关壁纸显示、调节音量。若产生Bug，可在VLC偏好设置中禁用。	