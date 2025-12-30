# clang++ -c hello.cpp -o hello.o -fPIC -std=c++20 -O2 -DNDEBUG
clang++ -c hello.cpp -o hello.o -std=c++20 -O2 -DNDEBUG
clang++ -c hello.cpp -o hello.o -std=c++20 -O2 -DNDEBUG -I"C:\code\2025-12\liuzheng\cangjie_tools\cjpm\cpp\include"
ar rcs libhello.a hello.o