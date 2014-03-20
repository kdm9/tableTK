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

    mkdir build && cd build
    cmake ..
    make
    ctest
    make install


Usage
=====

Run each utility with no arguments or with `-h` to receive a help message.
