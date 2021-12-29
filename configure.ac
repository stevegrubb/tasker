dnl
define([AC_INIT_NOTICE],
[### Generated automatically using autoconf version] AC_ACVERSION [
### Copyright 2022 Steve Grubb <sgrubb@redhat.com>
###
### Permission is hereby granted, free of charge, to any person obtaining a
### copy of this software and associated documentation files (the "Software"),
### to deal in the Software without restriction, including without limitation
### the rights to use, copy, modify, merge, publish, distribute, sublicense,
### and/or sell copies of the Software, and to permit persons to whom the
### Software is furnished to do so, subject to the following conditions:
###
### The above copyright notice and this permission notice shall be included
### in all copies or substantial portions of the Software.
###
### THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
### IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
### FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
### THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
### OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
### ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
### OTHER DEALINGS IN THE SOFTWARE.
###
### For usage, run `./configure --help'
### For more detailed information on installation, read the file `INSTALL'.
###
### If configuration succeeds, status is in the file `config.status'.
### A log of configuration tests is in `config.log'.
])

AC_REVISION($Revision: 1.0 $)dnl
AC_INIT(tasker,0.9)
AC_PREREQ(2.65)dnl
AM_CONFIG_HEADER(config.h)

echo Configuring tasker $VERSION
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

AC_CHECK_HEADER(omp.h, omp_headers=yes, omp_headers=no)
if test $omp_headers = no ; then
	AC_MSG_ERROR(openmp headers are missing)
fi

AC_OUTPUT(Makefile)
echo .
echo "

  tasker Version:         $VERSION
  Target:                 $target
  Installation prefix:    $prefix
  Compiler:               $CC
  Compiler flags:
`echo $CFLAGS | fmt -w 50 | sed 's,^,                          ,'`
"

