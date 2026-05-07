# Create a Custom Channel

The simulator is designed for black-box use, but it is possible to implement your own channel.

If you want to create your own decoder without dealing with the complex, factory-heavy binary, example CVQKD chains are provided in `src/examples/`.

---

## Understanding the Examples

The example files illustrate how AFF3CT tasks are connected depending on the channel, simulation mode, and coding scheme. Their filenames follow this convention:

    <channel>_<coding scheme>[_pip|_seq]

- The channel name comes first, then the coding scheme type.
- The suffix `_pip` indicates a **pipeline**, and `_seq` indicates a **sequence**.

!!! info
    When a coding scheme is described in the filename as **syndrome concatenation** <u>WITHOUT</u> **coset**, it uses the original AFF3CT decoder. Such an example is provided for educational purposes only as their $\beta$/FER performance will be lower than with the modified decoder.

---

## Creating a Custom File

Once you have created your own `.cpp` file (or adapted an example) in `src/`, register it in `CMakeLists.txt`:

### 1. Add the executable:

```cmake
add_executable(new_cpp_file_binary_name
    $<TARGET_OBJECTS:CVQKD-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/new_cpp_file.cpp
)
```

### 2. Add it to the target list:

```cmake
set(ALL_TARGETS
    new_cpp_file_binary_name

    create_bsparse_from_cil
    HDirac
)
```
