cmake_minimum_required(VERSION 3.6)
find_program(GLSLC glslc)
set(${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert
                  ${SHADER_DIR}/*.frag
                  ${SHADER_DIR}/*.comp
                  ${SHADER_DIR}/*.geom
                  ${SHADER_DIR}/*.tesc
                  ${SHADER_DIR}/*.tese
                  ${SHADER_DIR}/*.mesh
                  ${SHADER_DIR}/*.task
                  ${SHADER_DIR}/*.rgen
                  ${SHADER_DIR}/*.rchit
                  ${SHADER_DIR}/*.rmiss)
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
file(MAKE_DIRECTORY ${OUTPUT_DIR})
foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    set(INPUT_PATH "${SHADER_DIR}/${FILENAME}")
    set(OUTPUT_PATH "${OUTPUT_DIR}/${FILENAME}.spv")
    add_custom_command(OUTPUT ${OUTPUT_PATH}
        COMMAND ${GLSLC} ${INPUT_PATH} -o ${OUTPUT_PATH} 
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}"
        VERBATIM)
    list(APPEND SPV_SHADERS ${OUTPUT_PATH})
endForeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})