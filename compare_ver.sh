function version_gt() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1"; }
VERSION2=4.2.0
VERSION=4.2.0a
if version_gt $VERSION $VERSION2; then
   echo "$VERSION is greater than $VERSION2"
fi
