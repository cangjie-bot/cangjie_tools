clang++ -c hello.cpp -o hello.o -fPIC -std=c++20 -O2 -DNDEBUG -I"/Users/liuzheng/wnw/2025-12/cangjie_tools/cjpm/cpp/include"
# clang++ -c hello.cpp -o hello.o -std=c++20 -O2 -DNDEBUG
# clang++ -c hello.cpp -o hello.o -std=c++20 -O2 -DNDEBUG -I"C:\code\2025-12\liuzheng\cangjie_tools\cjpm\cpp\include"
ar rcs libhello.a hello.o

# /Users/liuzheng/wnw/2025-12/darwin_aarch64_cjnative/static/stdx
# export CANGJIE_STDX_PATH=/Users/liuzheng/wnw/2025-12/darwin_aarch64_cjnative/static/stdx
# -L {os.path.join(CURRENT_DIR, '..', 'cpp')} -lhello -luv 
# -lstdc++ -lgcc_s -lpthread -ldl
# cp ../dist/cjpm ../../../cangjie/tools/bin/