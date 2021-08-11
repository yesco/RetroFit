if [[ w.c -nt w.x ]]; then
  echo "--- w.c changed, recompiling w.x..."
  clang -g -w -lunistring  w.c -o w.x || exit
fi

if [[ Play/spin.c -nt spin.x ]]; then
  echo "--- spin.c changed, recompiling w.x..."
  clang -g -w Play/spin.c -o spin.x || exit
fi

# 
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

# less
# - http://www.greenwoodsoftware.com/less/faq.html#tricks
# less -F        # exit at end of file
# less +F        # follow at end of file as it grows "tail -f"
# less +150
# less +/patter  # go to first pattern
# less +G        # go to end

# tail +15 | head -20

###################################
# run it!

clear

# predisplay last file
# (will appear to be very fast!)
# TODO: use correct cached file
# (detect change of screen width?)
# NOTE: diplay ok as show whole lines!

(cat .stdout | perl -0777 -pe 's/(\n#.*?)+\n//g') \
| head -$((LINES-1))

# cursor home
printf "\e[1;4H\e[48;5;0m"

./spin.x 2>/dev/null & spinpid=$!

# less -X # scroll after last shown page
# == no use -C = see all that scrolled
# less -fr # (force) show raw ANSI codes
# TODO: >(head ...) if it loads slowly,
#   otherwise needs to wait till all loaded

(stdbuf -i0 -o0 -e0 ./w.x "$GO" > .stdout 2>.stderr \
  && (kill -9 $spinpid ; \
      cat .stdout \
      | perl -0777 -pe 's/(\n#.*?)+\n//g' \
      | less -XFrf)) \
|| printf "\n\n\e[48;5;1m\e[38;5;7m %% FAILED with ERROR $?\e[48;5;0m"

# just to make sure
kill -9 $spinpid 2>/dev/null

###################################
# TODO: write my own "less" w commands
# read - https://www.computerhope.com/unix/bash/read.htm
# read -sn1 key ; echo "KEY: $key"

###################################
printf "\e[m\e[48;50m\e[38;57m"
#clear

cat .stderr 1>&2

git show :w.c >.w.c
git show :table.c >.table.c
cat .w.c .table.c >.before-all.c

cat w.c table.c >.all.c

# print to STDERR
(printf "
      __            minimal 
     / /_      __         webbrowser 
    / /\ \ /\ / /   (>) CC-BY 2021    
 _ / /  \ V  V /    Jonas S Karlsson    
(_)_/    \_/\_/     jsk@yesco.org

- https://github.com/yesco/RetroFit -


TOTAL   Lines: `cat .all.c | wc`

w.c     - LOC: `./wcode w.c`
table.c - LOC: `./wcode table.c`
---
TOTAL   - LOC: `./wcode .all.c`
   (old - LOC: `./wcode .before-all.c`)



Usage: ./w            (loads test.html)
       ./w FILE.NAME  (tries file first)
       ./w URL        (if no file, wget)
       ./w yesco.org  (my homepage)
"
) 1>&2

rm .all.c
rm .w.c
rm .table.c
rm .before-all.c
