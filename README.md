# miniShell

使用Unix系统接口实现的简易shell，支持作业控制、多重管道、多重重定位等基本功能，主要参考csapp shell实验和xv6的shell实现，主要涉及的技术包括进程控制、信号处理、IO读写、进程间通信等。依赖库unix_lib参考csapp和apue中的函数，进行封装和改进后实现。

## 运行

首先安装unix_lib库，里面继承了常用的系统调用接口和对复杂函数的封装。

    git clone git@github.com:csfenghan/unix_lib.git
    cd unix_lib && make
    make install

安装完成后，clone并make即可

    git clone git@github.com:csfenghan/miniShell.git
    cd miniShell && make
    make run


