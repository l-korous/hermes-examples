project(1d-layer-boundary) 
add_executable(${PROJECT_NAME} main.cpp definitions.cpp)
set(COMPILE_FLAGS   "-g")
set_common_target_properties(${PROJECT_NAME} "HERMES2D")  


