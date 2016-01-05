set -e

NAME=$1
TESTLIB=$2
TESTFIX=$3

export PATH=$TESTLIB:$PATH
source assert.sh
source helpers.sh
