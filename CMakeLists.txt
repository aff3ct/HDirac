cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

project (virtual_channel)

# Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify bin path
set (EXECUTABLE_OUTPUT_PATH bin/)

# --------------------------------------------------------------------------#
#                               COMMON SOURCES                              #
# --------------------------------------------------------------------------#

# List all files used by BOTH executables here
set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Alice_quantum_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Quantum_channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Norms2_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Rotation_matrices_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Rotation_vectors_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/LLRs_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/LLRs_formatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/LLRs_computer_low_dimensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Real_operator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Complex_operator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Quaternion_operator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Octonion_operator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Decoder/LDPC/SPA/Decoder_LDPC_BP_flooding_SPA_coset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Decoder/LDPC/Syndrome_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/Cross_rotation_computer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Module/LLRs_computer_cross_rot.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tools/Helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tools/Reporter/Noise/Reporter_noise_CVQKD.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tools/LDPC/LDPC_build_tools.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Code/Code_params_LDPC_synd_cnct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Code/Code_params.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Code/Code_params_LDPC.cpp
)

# --------------------------------------------------------------------------#
#                               FIND PACKAGES                               #
# --------------------------------------------------------------------------#

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(cpptrace CONFIG 0.4.1)
find_package(AFF3CT CONFIG 4.2.0 REQUIRED)

# --------------------------------------------------------------------------#
#                               DEFINE TARGETS                              #
# --------------------------------------------------------------------------#

add_library(CVQKD-obj OBJECT ${COMMON_SOURCES})
set_target_properties(CVQKD-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Binary
add_executable(HDirac
    $<TARGET_OBJECTS:CVQKD-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HDirac.cpp
)

# Tool to create .bsparse from Cil matrices
add_executable(create_bsparse_from_cil
    $<TARGET_OBJECTS:CVQKD-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Create_bsparse_from_cil.cpp
)


# --------------------------------------------------------------------------#
#                          INCLUDE PATHS & LINKING                          #
# --------------------------------------------------------------------------#

# Create a list of all targets to apply settings to all of them at once
set(ALL_TARGETS
    HDirac
    create_bsparse_from_cil
)

target_include_directories(CVQKD-obj PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/eigen
)
    
target_link_libraries(CVQKD-obj PRIVATE 
    aff3ct::aff3ct-static-lib
    Threads::Threads
    cpptrace::cpptrace
)


foreach(target ${ALL_TARGETS})
    target_include_directories(${target} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    target_link_libraries(${target} PRIVATE 
        aff3ct::aff3ct-static-lib
        Threads::Threads
        cpptrace::cpptrace
    )
endforeach()
