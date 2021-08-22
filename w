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

./wbuild

for f in "$@"
do
  ./wdownload "$f"
done

./wless.x

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

cat .stderr 1>&2

# --- calculate LOCs

git show :w.c >.w.c
git show :table.c >.table.c
git show :wless.c >.wless.c
git show :jio.c >.jio.c
cat .w.c .table.c .wless.c jio.c >.before-all.c

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
 +table.c LOC: `./wcode table.c`
wless.c - LOC: `./wcode wless.c`
jio.c   - LOC: `./wcode jio.c`
---
TOTAL   - LOC: `./wcode .all.c`
   (old - LOC: `./wcode .before-all.c`)


Usage: ./w
       ./w F.NAME     (tries file first)
       ./w URL        (if no file, wget)
       ./w [URL ...]         (load many)
"
) 1>&2

# -- cleanup

rm .all.c
rm .w.c
rm .table.c
rm .before-all.c
rm .jio.c
