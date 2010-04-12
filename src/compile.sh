#!/bin/sh

CCFLAGS="-Wall -s"
OPTIMIZE="-O2"

GAME_OBJS="main.o frame.o isready.o loadmap.o misc.o gplot.o splash.o chk.o"
EDIT_OBJS="editor.o frame.o loadmap.o misc.o"

for SOURCE in `ls *.c` ; do
  echo "+COMPILE >  $SOURCE"
  gcc -c $SOURCE $CCFLAGS
  if [ $? == 1 ] ; then
    echo "-WARNING >  $SOURCE failed -- aborting!"
    exit 1
  fi
done

echo "+LINK    >  $GAME_OBJS"
gcc $GAME_OBJS -o phear -lncurses $CCFLAGS $OPTIMIZE

echo "+LINK    >  $EDIT_OBJS"
gcc $EDIT_OBJS -o editor -lncurses $CCFLAGS $OPTIMIZE

echo "+CLEANUP >  rm *.o"
rm *.o

ls -al phear 
ls -al editor

exit 0
