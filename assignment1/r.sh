gcc grand_child.c -o src

if [ -f ./src ]; then
	./src
	rm src
else
	echo Compile error :\(
fi
