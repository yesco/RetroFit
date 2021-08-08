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

GO="${1:-$URL}"

# run it!

clear

(./w.x "$GO" > stdout 2>stderr && less -r -f stdout) || printf "\n\n\e[48;5;1m\e[38;5;7m %% FAILED with ERROR $?\e[48;5;0m"

echo
cat stderr 1>&2

# print to STDERR
(printf "
      __            minimal 
     / /_      __         webbrowser 
    / /\ \ /\ / /   (>) CC-BY 2021    
 _ / /  \ V  V /    Jonas S Karlsson    
(_)_/    \_/\_/     jsk@yesco.org

- https://github.com/yesco/RetroFit -

w.c - total: `cat w.c |wc`
w.c -   LOC: `./wcode w.c`

usage: ./w            (loads test.html)
       ./w FILE.NAME  (tries file first)
       ./w URL        (if no file, wget)
       ./w yesco.org  (my homepage)
"
) 1>&2


