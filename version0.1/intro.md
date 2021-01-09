参照CSAPP开发一个tiny web服务器。
并将在之后的版本完善。

编译 g++ server.cc util.cc -o a
运行 sudo ./a 80

静态页面
浏览器访问 127.0.0.1/index.html

cgi
cd cgi-bin
g++ adder.c ../util.cc -0 add
浏览器访问 127.0.0.1/cgi-bin/add?3&4
