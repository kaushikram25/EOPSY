#!/bin/bash
filename=modify

error_msg() 
{ 
    echo "$filename: error is : $1" 1>&2 
}

show_help()
{
    echo "
    usage:
        $filename [-r] [-l|-u] | directory or file filenames
        $filename [-r] | sed pattern | directory or file filenames
        $filename [-h]

    options:
        -l          Lower Casing file filenames
        -u          Upper Casing file filenames
        -r          Use Recursion
        -h          Show Help

    $filename syntax is correct examples: 
        $filename -l FILE.c
        $filename -u File.c
        $filename -r -l directory/
        $filename -r |sed pattern | directory1 directory2
        $filename -h

    $filename syntax is incorrect example: 
        $filename File.c
    "
}

# <RECURSIVE>:
#    0 : Not-Recursive
#    1 : Recursive
RECURSIVE=0
# <CASE>:
#   -1 : Sed_Pattern
#    0 : Lowercase
#    1 : Uppercase
CASE=-1

while test "x$1" != "x"
do
    case "$1" in
        -h) show_help; exit 0;;
        -r) RECURSIVE=1; shift;;
        -l) 
            if test "x$2" == "x"
            then
                error_msg "you should provide <directory or file filenames>"; exit 1
            fi
            CASE=0; shift; break;;
        -u)
            CASE=1; shift; break;;
        -*) error_msg "bad option is $1"; exit 1;;
        *) 
            if test "$CASE" == "-1" && test "x$1" != "x" && test "x$2" != "x"
            then
                PATTERN=$1; shift; break
            else
                error_msg "you should provide <sed patttern> and <directory or file filenames>"; exit 1
            fi;;
    esac
done

handleDirectory()
{
    DIR_filename=`basename "$1"`
    filepath=`dirname "$1"`
    if test "$CASE" == "-1"
    then
        new_file_name=`echo "$DIR_filename" | sed --regexp-extended "$PATTERN"`
        if test $? != "0"
        then
            error_msg "incorrect sed, you should provide correct <sed patttern> "; exit 1
        fi
    elif test "$CASE" == "0"
    then
        new_file_name=`echo "$DIR_filename" | tr "[:upper:]" "[:lower:]"`
    else
        new_file_name=`echo "$DIR_filename" | tr "[:lower:]" "[:upper:]"`
    fi

    if test "$filepath/$DIR_filename" != "$filepath/$new_file_name"
    then
        mv "$filepath/$DIR_filename" "$filepath/$new_file_name"
    fi
    
    if test $RECURSIVE == "1"
    then
        for entry in "$filepath/$new_file_name"/*
        do
            handleInstance "$entry"
        done
    fi
}

handleFile()
{
    EXTENSION=`echo "$1" | grep -o "[^.]*$"`
    DIR_filename=`basename "$1" ".$EXTENSION"`
    filepath=`dirname "$1"`
    if test "$CASE" == "-1"
    then
        echo "$PATTERN"
        new_file_name=`echo $DIR_filename | sed --regexp-extended "$PATTERN"`       
    elif test "$CASE" == "0"
    then
        new_file_name=`echo $DIR_filename | tr "[:upper:]" "[:lower:]"`
    else
        new_file_name=`echo $DIR_filename | tr "[:lower:]" "[:upper:]"`
    fi

    if test "$filepath/$DIR_filename.$EXTENSION" != "$filepath/$new_file_name.$EXTENSION"
    then
        mv "$filepath/$DIR_filename.$EXTENSION" "$filepath/$new_file_name.$EXTENSION"
    fi
}

handleInstance()
{
    if [ -d "$1" ]
    then
        handleDirectory "$1"
    elif [ -f "$1" ]
    then
        handleFile "$1"
    else 
        error_msg "$1 Please try this is not valid"
    fi
}

while test "x$1" != "x"
do
    handleInstance "$1"; shift
done

exit 0