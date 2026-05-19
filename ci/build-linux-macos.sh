#!/bin/bash
set -x

cmake --version
mkdir build
cd build

if [ -z "$CXX" ]
then
	echo "Please define the 'CXX' environment variable."
	exit 1
fi

if [ -z "$THREADS" ]
then
	echo "The 'THREADS' environment variable is not set, default value = 1."
	THREADS=1
fi

if [ -z "$NAME" ]
then
	echo "The 'NAME' environment variable is not set, default value = 'build_linux_macos'."
	NAME="build_linux_macos"
fi

# cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX \
#          -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="$CFLAGS" \
#          -DCMAKE_EXE_LINKER_FLAGS="$LFLAGS" \
#          $CMAKE_OPT -DCMAKE_INSTALL_PREFIX="$NAME"
# rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# make -j $THREADS -k
# rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
# rm -rf $NAME
# make install > /dev/null
# rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
# mv $NAME ../

pwd;
ls -lia;

cd lib/aff3ct;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

mkdir build;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd build;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_CXX_FLAGS="$CFLAGS" \
         -DCMAKE_EXE_LINKER_FLAGS="$LFLAGS" \
         -DAFF3CT_COMPILE_EXE="OFF" \
         -DAFF3CT_COMPILE_STATIC_LIB="ON" \
         -DAFF3CT_COMPILE_SHARED_LIB="ON" \
         $CMAKE_OPT -DCMAKE_INSTALL_PREFIX="$NAME";
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --build . -j $THREADS;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --install . --prefix ../../../aff3ct_install;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

ln -sfn ../../../aff3ct_install/lib/cmake/aff3ct-* ../../../aff3ct_install/lib/cmake/aff3ct;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd ../../../;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

mkdir $NAME;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd $NAME;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_CXX_FLAGS="$CFLAGS" \
         -DCMAKE_EXE_LINKER_FLAGS="$LFLAGS" \
         -DAFF3CT_DIR=$(pwd)/../aff3ct_install/lib/cmake/aff3ct \
         -Dcpptrace_DIR=$(pwd)/../aff3ct_install/lib/cmake/cpptrace \
         $CMAKE_OPT -DCMAKE_INSTALL_PREFIX="$NAME";
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --build . -j $THREADS;
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
