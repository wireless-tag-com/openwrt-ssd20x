#!/bin/sh

num=1
clean_num=0
tanted_num=0

# retreive initial commit id
first_commit=$( git log --reverse --pretty=format:'%h' | tr '\n' ':' | cut -d':' -f1 )

# make sure the new line is unix format
dos2unix $1

while read -r line
do
    git_log=$( git log --color --graph --pretty=format:'%h -%d %s (%cr) <%an>' --abbrev-commit -- $line | grep -v $first_commit )
    if [ "$git_log" == "" ]
    then
        echo $num:$line:'CLEAN'
        clean_num=$(( clean_num+1 ))
    else
        echo $num:$line:'TANTED'
        git log --color --graph --pretty=format:'%h -%d %s (%cr) <%an>' --abbrev-commit -- $line
        echo ''
        tanted_num=$(( tanted_num+1 ))
    fi
    #echo ''
    num=$(( num+1 ))
done < $1

echo ======================================
echo CLEAN: $clean_num, TANTED: $tanted_num
echo ======================================

