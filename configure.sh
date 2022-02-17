#!/bin/bash

#
# Copyright 2022, Elliot Kohlmyer
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.
#

echo -- configure --

arg=$1

# stuff

## c and c++ compilers
CC=cc
CXX=c++
## cross compilation prefix and installation prefix
CROSS_COMPILE=
DESTDIR=/
## headers and other files to check
CHECK_HEADERS=

# for config stuff
touch "config.source"

echo -n

# options
while [ "$arg" != "" ]; do
	
	# c compiler
	if [ "$arg" == "-cc" ]; then
		shift
		arg=$1
		
		if [ "$arg" == "" ]; then
			echo $0: unspecified value for C compiler
			exit 1
		fi
		
		# set variable in config.source
		useless=$(echo "CC=$arg" | tee -a "config.source")
		unset useless
	fi
	
	# c++ compiler
	if [ "$arg" == "-cxx" ]; then
		shift
		arg=$1
		
		if [ "$arg" == "" ]; then
			echo $0: unspecified value for C++ compiler
			exit 1
		fi
	
		# config.source
		useless=$(echo "CXX=$arg" | tee -a "config.source")
		unset useless
	fi
	
	# installation destination prefix
	if [ "$arg" == "-prefix" ]; then
		shift
		arg=$1
		
		if [ "$arg" == "" ]; then
			echo $0: unspecified value for installation destination prefix
			exit 1
		fi
		
		# config.source
		useless=$(echo "DESTDIR=$arg" | tee -a "config.source")
		unset useless
	fi
	
	# cross compile prefix
	if [ "$arg" == "-cross" ]; then
		shift
		arg=$1
	
		if [ "$arg" == "" ]; then
			echo $0: unspecified value for cross compilation prefix
			exit 1
		fi
		
		# config.source
		useless=$(echo "CROSS_COMPILE=$arg" | tee -a "config.source")
		unset useless
	fi
	
	# help
	if [ "$arg" == "-help" ]; then
	
		echo usage: $0 [options]
		echo -e \\t-cc\\tspecify C compiler \(i.e. gcc\)
		echo -e \\t-cxx\\tspecify C++ compiler \(i.e. g++\)
		echo -e \\t-cross\\tspecify cross compilation prefix \(i.e. x86_64-linux-gnu-\)
		echo -e \\t-prefix\\tspecify installation prefix \(i.e /usr\)
		echo -e \\t-help\\tdisplay this help message
		echo
		exit 0
	fi
	
	shift
	arg=$1
done

# use config.source and config.defaults
source "config.defaults" 1>/dev/null 2>/dev/null
source "config.source" 1>/dev/null 2>/dev/null

# set values to use CROSS_COMPILE prefix
CC="$CROSS_COMPILE$CC"
CXX="$CROSS_COMPILE$CXX"

echo 'int main(int argc, char **argv) { printf("%s okay\n", argv[1]); }' > "conf.test.c"
echo -e '#include <iostream>\nint main(int argc, char **argv) { cout << argv[1] << " okay" << endl; }' > "conf.test.cpp"

# print info
echo "C compiler is $CC"
echo "C++ compiler is $CXX"

# test c compiler
echo -n "testing C compiler... "
"$CC" "conf.test.c" -o ./a.out 1>/dev/null 2>/dev/null

# failed
if [ $? -ne 0 ]; then
	echo failed to test C compiler
	rm "conf.test.c"
	if [ "$TESTCXX" == "YES" ]; then rm "conf.test.cpp"; fi
	exit 1
fi

# test if file ./a.out exists
stat "./a.out" 1>/dev/null 2>/dev/null

if [ $? -ne 0 ]; then
	echo failed to test C compiler
	rm "conf.test.c"
	if [ "$TESTCXX" == "YES" ]; then rm "conf.test.cpp"; fi
	exit 1
fi

# okay
./a.out $CC
rm "conf.test.c"

# test c++ compiler
if [ "$TESTCXX" == "YES" ]; then
	
	# test for compiler
	echo -n "testing C++ compiler... "
	"$CXX" "conf.test.cpp" -o ./b.out 1>/dev/null 2>/dev/null
	
	# failed
	if [ $? -ne 0 ]; then
		echo failed to test C++ compiler
		rm "conf.test.cpp"
		exit 1
	fi
	
	# file
	stat ./b.out 1>/dev/null 2>/dev/null
	
	# failed
	if [ $? -ne 0 ]; then
		echo failed to test C++ compiler
		rm "conf.test.cpp"
		exit 1
	fi
	
	# okay
	./b.out $CXX
fi

rm "conf.test.cpp"

# search for headers
if [ "$CHECK_HEADERS" != "" ]; then
	for header in $CHECK_HEADERS; do
		
		# search for file
		echo -n "looking for $header... "
		
		# stat file
		stat /usr/include/$header 1>/dev/null 2>/dev/null
		
		# error
		if [ $? -ne 0 ]; then
	
			echo not found
			exit 1
		else
		
			echo found
		fi
	done
fi

# generate makefile
echo -n "generating Makefile... "
cp Makefile.orig Makefile

sed -i -- "s#_configure_CC#$CC#g" Makefile
sed -i -- "s#_configure_CXX#$CXX#g" Makefile
sed -i -- "s#_configure_DESTDIR#$DESTDIR#g" Makefile
echo

echo "cleaning up..."
rm a.out "config.source"
if [ "$TESTCXX" == "YES" ]; then
	rm b.out
fi

echo "configuration done"
