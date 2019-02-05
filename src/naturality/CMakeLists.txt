
add_library(Naturality SHARED
  src/cospan.cpp
  src/natural_transformation_node.cpp
)

target_include_directories(Naturality
  PUBLIC
    inc
  PRIVATE
    ${CMAKE_JS_INC}
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
)

target_link_libraries(Naturality
  PUBLIC
    ${CMAKE_JS_LIB}
  PRIVATE
    PolymorphicTypes
)

set_target_properties(Naturality 
  PROPERTIES 
    PREFIX "" 
    SUFFIX ".node"
)

# add_subdirectory(test)
