#!/bin/sh

EXE=$1

if [ ! -x "$EXE" ]; then
    echo "Bail out! Executable '$EXE' missing"
    exit 1
fi

FILES=$(find ./instances -type f -name "*.SAT.cnf" -o -name "*.UNSAT.cnf")
NFILES=$(echo "$FILES" | wc -l)

# Output is TAPish (http://testanything.org/), to let `prove` handle reporting.
echo "1..$NFILES"

for f in $FILES; do
    # Extract "SAT" or "UNSAT" from filename.
    # Needed until we can output (and verify!) UNSAT proofs.
    ANSWER=${f%.cnf}
    ANSWER=${ANSWER##*.}

    case $ANSWER in
        "SAT")
            $EXE "$f" | $EXE -V- "$f" > /dev/null;;
        "UNSAT")
            $EXE "$f" | grep "^s UNSATISFIABLE" > /dev/null;;
        *) false;;
    esac

    if [ $? -eq 0 ]; then
        echo "ok $f"
    else
        echo "not ok $f"
    fi
done
