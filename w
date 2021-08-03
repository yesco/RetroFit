# ./spin & # how to stop!!
# clang -g -w w.c && ((echo "run http://yesco.org/";echo "where") | gdb ./a.out )

#clang -g -w w.c && ./a.out https://example.com/

clang -g -w w.c && ./a.out http://yesco.org/

# So ugly!
#clang -g -w w.c && ./a.out http://ibm.com/

# TODO: &#53535;
#clang -g -w w.c && ./a.out http://google.com

# TODO: black background?
#clang -g -w w.c && ./a.out https://en.m.wikipedia.org/wiki/UTF-8

# possibly pipe to "less -R" for ansi
