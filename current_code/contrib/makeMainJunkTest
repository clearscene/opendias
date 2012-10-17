rm main main.o 
gcc -I/usr/include/uuid -g -O2 -c -o main.o main.c
g++ -g -O2 -o main main.o -luuid
valgrind  --leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --show-below-main=yes --track-origins=yes --track-fds=yes --gen-suppressions=all ./main
