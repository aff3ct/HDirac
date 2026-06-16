# Build & Install

## Get the HDirac repository

=== "SSH"

    ```bash
    git clone git@github.com:aff3ct/HDirac.git
    ```

=== "HTTPS"

    ```bash
    git clone https://github.com/aff3ct/HDirac.git
    ```



## Compile AFF3CT as a Library

Get the AFF3CT library:

```bash
cd HDirac
git submodule update --init --recursive
```

Compile the library on Linux/MacOS/MinGW/WSL:

```bash
cd lib/aff3ct
mkdir build
cd build
cmake .. -G"Unix Makefiles" \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native" \
         -DAFF3CT_COMPILE_EXE="OFF" \
         -DAFF3CT_COMPILE_STATIC_LIB="ON" \
         -DAFF3CT_COMPILE_SHARED_LIB="ON"
cmake --build . -j 12
cmake --install . --prefix ../../../aff3ct_install
ln -sfn ../../../aff3ct_install/lib/cmake/aff3ct-* ../../../aff3ct_install/lib/cmake/aff3ct
cd ../../../
```

Now the AFF3CT library has been built in the `lib/aff3ct/build` folder.

!!! warning
    If you are encountering some problems with
    [`cpptrace`](https://github.com/jeremy-rifkin/cpptrace), you can simply disable it by adding
    `-DSPU_STACKTRACE="OFF"` in the previous `cmake` command line as follow:
    
    ```bash
    cmake .. -G"Unix Makefiles" \
             -DCMAKE_BUILD_TYPE=RelWithDebInfo \
             -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native" \
             -DAFF3CT_COMPILE_EXE="OFF" \
             -DAFF3CT_COMPILE_STATIC_LIB="ON" \
             -DAFF3CT_COMPILE_SHARED_LIB="ON" \
             -DSPU_STACKTRACE="OFF"
    ```

## Compile HDirac Simulator

Make sure to have done the instructions above, **before** doing this. AFF3CT should have been compiled **before** doing
the following.

Compile the code on Linux/MacOS/MinGW/WSL:

```bash
mkdir build
cd build
cmake .. -G"Unix Makefiles" \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native" \
         -DAFF3CT_DIR=$(pwd)/../aff3ct_install/lib/cmake/aff3ct \
         -Dcpptrace_DIR=$(pwd)/../aff3ct_install/lib/cmake/cpptrace
cmake --build . -j 12
```

The source code of this project is in `src/HDirac.cpp`.
The compiled binary is in `build/bin/HDirac`.

!!! warning
    If you are encountering some problems with
    [`cpptrace`](https://github.com/jeremy-rifkin/cpptrace), try to compile AFF3CT
    and StreamPU without linking with it (see the `-DSPU_STACKTRACE="OFF"`
    option). Please refer to the AFF3CT compilation section above.
