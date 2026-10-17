@echo off
REM Compila a biblioteca, o exemplo e os testes no Windows (MinGW gcc), sem make.
gcc -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude -c src\skew_heap.c -o skew_heap.o || exit /b 1
ar rcs libskewheap.a skew_heap.o || exit /b 1
gcc -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude examples\exemplo.c -L. -lskewheap -o exemplo.exe || exit /b 1
gcc -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude tests\teste.c -L. -lskewheap -o teste_skew.exe || exit /b 1
echo Compilado: libskewheap.a, exemplo.exe, teste_skew.exe
