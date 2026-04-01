cmake_minimum_required(VERSION 3.15)

project(SequentialRunner)

find_package(Python3 REQUIRED COMPONENTS Interpreter)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# generowanie danych
add_custom_target(run_data_gen
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/data_gen.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Step 1: Running data_gen.py"
)

#gmp mnożenie
add_executable(main_gmp ${CMAKE_SOURCE_DIR}/main_GMP.cpp)

target_link_libraries(main_gmp gmp)

add_custom_target(run_main_gmp
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/main_gmp.exe
    DEPENDS main_gmp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Step 2: Running main_GMP.cpp"
)

#Python mnożenie
add_custom_target(run_main_python
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/main_PYTHON.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Step 3: Running main_PYTHON.py"
)

#wykresy
add_custom_target(run_analysis
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/analysis.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Step 4: Running analysis.py"
)
#kolejność
add_dependencies(run_main_gmp run_data_gen)
add_dependencies(run_main_python run_main_gmp)
add_dependencies(run_analysis run_main_python)


add_custom_target(run_all
    DEPENDS run_analysis
)