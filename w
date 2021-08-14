# --- Setup cache structure
# TODO: in the home directory?

WDIR=.w

[ ! -d $WDIR ] \
  && mkdir $WDIR
  # TODO: could be shared globally

[ ! -d $WDIR/Cache ] \
  && mkdir $WDIR/Cache
  # TODO: could be several?
[ ! -d $WDIR/Session ] \
  && mkdir $WDIR/Session

# --- sets LINES/COLUMNS
stty size >/dev/null
clear # this too!

./wbuild

# --- URLs

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

SAFE=`./wquote "$GO"`
FILE=$WDIR/Cache/$SAFE

# --- display preview

clear

# TODO: use correct cached file
# (detect change of screen width?)

[ ! -f $FILE.ansi ] && (./w.x $FILE > $FILE.ansi)

# show a page-ful of last page viewed

./wdisplay $FILE.ansi 0 $((LINES-2))

# --- Log it
echo "`date --iso=s` #=W $GO" >> .wlog

# --- WGET in background
# Exit:
#   1   Generic error code.
#   2   Parse error (options).
#   3   File I/O error.
#   4   Network failure.
#   5   SSL verification failure.
#   6   Username/password auth fail.
#   7   Protocol errors.
#   8   Server gave error.

# TODO: always update?
[ ! -f "$GO" ] && (wget --backups=3 -q -O $FILE -a $FILE.LOG "$GO" >/dev/null) &
echo "`date --iso=s` #=W $GO" >> $FILE.WLOG

# --- less options help
# - http://www.greenwoodsoftware.com/less/faq.html#tricks
# less -F        # exit at end of file
# less +F        # follow at end of file as it grows "tail -f"
# less +150
# less +/patter  # go to first pattern
# less +G        # go to end

# tail +15 | head -20

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

(stdbuf -i0 -o0 -e0 ./w.x "$GO" 2>.stderr | tee .stdout \
  | perl -0777 -pe 's/(\n#.*?)+\n//g' \
  | less -Xrf) \
  && kill -9 $spinpid \
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
