#
#     ./w - mini web-browser
#
#
#
# (>) CC-BY 2021 Jonas S Karlsson
#
#           jsk@yesco.org
#

# Usage:
#  ./w [URL ...]

export UDIR=`pwd`
echo "UDIR=$UDIR"
[[ ! -e $BASH_SOURCE ]] && export WPATH=$(cd `dirname ${BASH_SOURCE:-.}` ; pwd)
[[ -e $BASH_SOURCE ]] && export WPATH=$UDIR

echo "WPATH=$WPATH"

# The script is stupid and assumes being run from the source code directory...

# Build

(cd $WPATH ; ./wbuild) || exit $!

# just to be sure...

cd $UDIR

# -- check internet connection
# (for some reason can't do exactly this from C?)

wget -O /dev/null example.com 2>/dev/null || (\
  printf "\e[41;37m%% ./w: No internet connection!\e[40;37m\n\n\n" ;\
  printf "(press RETURN to continue)\n" ;\
  read foo)



# -- download/refresh in background

stty size >/dev/null

for f in "$@"
do
  
  $WPATH/wdownload -d -l "$f" $LINES $COLUMNS &

done

# wait for wdownload to write to .whistory
sleep 0.2



# -- browse

#debug
#gdb -q $WPATH/wless.x -ex run -ex bt -ex quit

#normal
$WPATH/wless.x

stty sane


# --- reset screen after text

printf "\e[m\e[48;50m\e[38;57m"

# --- wait for HTML downloaded
#
# [ "$CFILE" == "$FILE" ] && \
# while [ $CFILE.ANSI -nt $CFILE ]; do
#      printf ">"
#      sleep 1
# done

#./w.x $CFILE $GO > $CFILE.ANSI \
#  || (printf "\n\n\e[48;5;1m\e[38;5;7m %% FAILED with ERROR $?\e[48;5;0m" && \
#  (printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"; echo "run $GO";echo "where") | gdb ./w.x; exit) || exit

#(stdbuf -i0 -o0 -e0 ./w.x "$GO" 2>.stderr | tee -a .stdout \
#  | perl -0777 -pe 's/(\n#.*?)+\n//g' \
#  | less -Xrf) \
#  && kill -9 $spinpid \

killall spin.x 2>/dev/null

###################################

# --- reset screen after text

printf "\e[m\e[48;50m\e[38;57m"
#clear

# --- show any errors

cat .stderr 2>/dev/null 1>&2

# --- calculate LOCs

cd $WPATH

git show :w.c >.w.c
git show :table.c >.table.c
git show :wless.c >.wless.c
git show :jio.c >.jio.c
git show :graphics.c >.graphics.c

cat .w.c .table.c .wless.c >.before-all.c
cat w.c table.c wless.c >.all.c

cat .w.c .table.c >.before-render.c
cat w.c table.c >.render.c

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

render  - LOC: `./wcode .render.c`	(`./wcode .before-render.c`)
browser - LOC: `./wcode wless.c`	(`./wcode .wless.c`)
---
TOTAL   - LOC: `./wcode .all.c`	(`./wcode .before-all.c`)

[libary - LOC: `./wcode jio.c`	(`./wcode .jio.c`)]
[graphic- LOC: `./wcode graphics.c`	(`./wcode .graphics.c`)]


Usage: ./w
       ./w F.NAME   (tries file first)
       ./w URL      (if no file, wget)
       ./w [URL ...]       (load many)
"
) 1>&2

# -- cleanup

rm .all.c
rm .w.c
rm .table.c
rm .before-all.c
rm .before-render.c
rm .jio.c
rm .graphics.c

cd $UDIR
