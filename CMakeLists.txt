cmake_minimum_required(VERSION 3.0.0)
project(quick VERSION 0.1.0 LANGUAGES C CXX)


find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")


include(CTest)
enable_testing()

add_executable(quick main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(quick PUBLIC MPI::MPI_CXX)
