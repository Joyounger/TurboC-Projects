# TurboC-Projects

开发环境搭建

1. 使用新立得软件包管理器安装dosbox。

2. 下载Turbo C++ 3.0，这里假设用本贴提供的（不是我改过的）。
地址：tcpp3.part1.rar tcpp3.part2.rar

3. 将本帖提供的Turbo C++ 3.0解压到~/tmp/tc3inst。（~是指“主文件夹”）

4. 为了使以后使用dir命令的时候不会列出一大堆不相关的东西，建立一个目录/文件夹专门用来存放DOS程序：~/dosapp（用“位置->主文件夹”来打开文件管理器，空白处点右键->“创建文件夹”），等下我们就把Turbo C++ 3.0安装到这里来。

5. 启动dosbox：Alt+F2，然后输入dosbox，单击“运行”。完了会启动dosbox窗口，如图所示：
<!—->


6. 做好心里准备：从现在起要输入命令了，DOS本来就是命令操作的。下面的命令都在dosbox里面输入。

7. 挂载Turbo C++ 3.0安装盘到D盘。

代码:
mount d ~/tmp/tc3inst
8. 挂载~/dosapp到C盘，以后每次运行dosbox，也要将~/dosapp挂载到C盘。例如，Turbo C++是将盘符和配置挂钩的，本来在C盘的，你放到D盘即使运行了也会出错（假如不做任何修改的话）。

代码:
mount c ~/dosapp
9. 切换到D盘，然后运行安装程序。

代码:
D:
install
如图所示：
<!—->

10. 按回车继续，第二个画面选择安装源，直接按下键盘上的D。完成后如图。
<!—->

11. 按回车继续，这个画面要求选择安装源目录，如图所示。由于默认的安装源目录是根目录，所以直接按回车继续。
<!—->

12. 这里的默认安装选项是要将Turbo C++安装到D:TC，这里将其安装到C:TC比较合适。
此时（光标选中第一项）直接按回车，在Turbo C++ Directory（默认就是了）处按回车，输入C:TC，如图所示。
<!—->
回车后，其他目录也根着改成在C:TC下了。此时按Esc，回到主界面，如图所示。
<!—->

13. 由于默认安装选项已经选择安装所有的功能，此时用上下箭头将选择移动到Start Installation，按回车后开始安装。等一下然后安装就完成了，如图所示。
<!—->

14. 按任意键继续。此时会打开一篇说明文档，有兴趣的可以看看。看完了按Esc退出。

15. 安装完成了，Turbo C++可以使用了。现在切换到C盘Turbo C++的程序目录然后启动Turbo C++。

代码:
c:
cd tcbin
tc
如图所示。
<!—->

———————========= 华丽的分割线 =========———————

16. 以后的使用方法：启动dosbox，挂载~/dosapp到C盘，然后启动TC。
挂载~/dosapp到C盘：

代码:
mount c ~/dosapp
启动TC：

代码:
c:
cd tcbin
tc
退出dosbox：

代码:
exit


编译执行程序
在dosbox中启动tc后，按F3打开每章源程序并编译运行

