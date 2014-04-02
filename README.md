Table Toolkit
=============

Several fast utilities to process tabular data row-wise. Specifically, these
tools target tables too big to fit into memory

Note - this is all very alpha code, don't rely on it actually working. It will
be tidied up soon enough though.

filterTable
-----------

Filter tabular data quickly by non-zero cells or row medians


tableDist
---------

Calculate Manhattan or Canberra distance matrices from tablular data by building
distance matricies row-wise


Installation
============

We use cmake as the build system, so instead of the customary
`./configure && make && make install`, use the following. Cmake doesn't take a
`--prefix` option, please use the analagous `-DCMAKE_INSTALL_PREFIX=` option to
cmake to install `tableTK` binaries in a non-standard location. Cmake is also
able to create windows-compatible build configurations, although this has not
been attempted ([see here](http://www.cmake.org/cmake/help/runningcmake.html)).

    git clone --recursive https://github.com/kdmurray91/tableTK.git tableTK
    cd tableTK
    mkdir build && cd build
    cmake .. # use -DCMAKE_INSTALL_PREFIX=/path/to/prefix e.g. /usr/local
    make
    ctest
    make install



Usage
=====

Run each utility with no arguments or with `-h` to receive a help message.
