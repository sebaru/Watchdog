echo "Replace $1 by $2 in every .c .h .js .php .y"
find . -type f -name '*.js' -exec sed -i 's/'$1'/'$2'/g' {} \;
find . -type f -name '*.c' -exec sed -i 's/'$1'/'$2'/g' {} \;
find . -type f -name '*.h' -exec sed -i 's/'$1'/'$2'/g' {} \;
find . -type f -name '*.php' -exec sed -i 's/'$1'/'$2'/g' {} \;
find . -type f -name '*.y' -exec sed -i 's/'$1'/'$2'/g' {} \;

