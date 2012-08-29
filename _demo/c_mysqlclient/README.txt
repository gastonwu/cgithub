You might have to edit the Makefile somewhat to get the programs in
this directory to build on your system.

The Makefile assumes that your C and C++ compilers are named gcc and g++.
On systems where that is not true, you can invoke make with CC and CXX
arguments that override the names.  For example, if your compilers are named
cc and c++, you can invoke make like this:

    % make CC=cc CXX=c++ all
