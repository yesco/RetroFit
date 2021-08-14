# sets LINES/COLUMNS
stty size >/dev/null
clear # this too!

# --- dependencies
if [[ jio.c -nt w.x ]]; then
  rm w.x wless.x 2>/dev/null
fi
if [[ table.c -nt w.x ]]; then
  rm w.x 2>/dev/null
fi

# --- compile w.c
if [[ w.c -nt w.x ]]; then
  echo "--- w.c changed, recompiling w.x..."
  rm w.x 2>/dev/null
  cc -g -w -lunistring  w.c -o w.x || (printf "maybe need to install libunistring?\napt install libunistring\nbrew install libunistring\nyum install libunistring-devel\n" && exit 4711) || exit
fi

# --- compile spinner.c
# - if fails, ignore

if [[ Play/spin.c -nt spin.x ]]; then
  echo "--- spin.c changed, recompiling w.x..."
  rm spin.x 2>/dev/null
  cc -g -w Play/spin.c -o spin.x # || exit
fi

# --- compile wless.c
# - if fails, ignore

if [[ wless.c -nt wless.x ]]; then
  echo "--- wless.c changed, recompiling wless.x..."
  rm wless.x 2>/dev/null
  cc -g -w wless.c -o wless.x || exit
fi

# 
# cc -g -w w.c && ((echo "run http://yesco.org/";echo "where") | gdb ./a.out )

# --- URLS
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

# --- Select actual URL to view
GO="${1:-$URL}"

# less
# - http://www.greenwoodsoftware.com/less/faq.html#tricks
# less -F        # exit at end of file
# less +F        # follow at end of file as it grows "tail -f"
# less +150
# less +/patter  # go to first pattern
# less +G        # go to end

# tail +15 | head -20

# --- display preview

clear

# predisplay last file
# (will appear to be very fast!)
# TODO: use correct cached file
# (detect change of screen width?)
# NOTE: diplay ok as show whole lines!

./wdisplay .stdout 1 $((LINES-1))

# --- display spinning globe!

./spin.x 2>/dev/null & spinpid=$!

# --- load web-page
# - display the real thing
# - or give error code
# - and gdb where stacktrace!

# cursor home
printf "\e[1;4H\e[48;5;0m"

# less -X # scroll after last shown page
# == no use -C = see all that scrolled
# less -fr # (force) show raw ANSI codes
# TODO: >(head ...) if it loads slowly,
#   otherwise needs to wait till all loaded

(stdbuf -i0 -o0 -e0 ./w.x "$GO" > .stdout 2>.stderr \
  && (kill -9 $spinpid; \
      ./wdisplay \
      | less -Xrf)) \
|| (printf "\n\n\e[48;5;1m\e[38;5;7m %% FAILED with ERROR $?\e[48;5;0m" && \
(kill -9 $spinpid 2>/dev/null; printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"; echo "run $GO";echo "where") | gdb ./w.x; exit) || exit

# --- kill spinner for sure

kill -9 $spinpid 2>/dev/null

###################################

# --- reset screen after text

printf "\e[m\e[48;50m\e[38;57m"
#clear

# --- show any errors

cat .stderr 1>&2

# --- calculate LOCs

git show :w.c >.w.c
git show :table.c >.table.c
git show :wless.c >.wless.c
cat .w.c .table.c .wless.c >.before-all.c

cat w.c table.c jio.c wless.c >.all.c

# --- print stats and USAGE

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
  table.c LOC: `./wcode table.c`
wless.c - LOC: `./wcode wless.c`
jio.c   - LOC: `./wcode jio.c`
---
TOTAL   - LOC: `./wcode .all.c`
   (old - LOC: `./wcode .before-all.c`)


Usage: ./w            (loads test.html)
       ./w FILE.NAME  (tries file first)
       ./w URL        (if no file, wget)
       ./w yesco.org  (my homepage)
"
) 1>&2

# -- cleanup

rm .all.c
rm .w.c
rm .table.c
rm .before-all.c
