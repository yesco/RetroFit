if [[ w.c -nt w.x ]]; then
  echo "--- w.c changed, recompiling w.x..."
  clang -g -w -lunistring  w.c -o w.x || exit
fi

# ./spin & # how to stop!!
# clang -g -w w.c && ((echo "run http://yesco.org/";echo "where") | gdb ./a.out )

URL=https://example.com/

# So ugly!
URL=http://ibm.com/

# TODO: &#53535;
URL=http://google.com

# TODO: black background?
URL=https://en.m.wikipedia.org/wiki/UTF-8

# [image] alt tag!
URL=http://lysator.liu.se/

# maybe not detecting tag correctly?
URL=http://runeberg.org

URL=https://github.com/fractalide/copernica


# code/pre
URL=http://www.columbia.edu/~fdc/sample.html

URL=http://yesco.org/
URL=test.html

# run it!

./w.x "${1:-$URL}"

# possibly pipe to "less -R" for ansi

echo
echo "---"
echo "(w.c: total lines    : `cat w.c |wc`)"
echo "(w.c: pure code lines: `grep -iPv '^\s*//' w.c | grep -v '^\s*$' | wc`)"
echo
echo "usage: ./w            (loads test.html)"
echo "       ./w FILE.NAME  (tries file first)"
echo "       ./w URL        (if no file, wget)"
echo "       ./w yesco.org  (my homepage)"
