
add_library(PolymorphicTypes SHARED
  src/addon.cpp
  src/type_constructor.cpp
  src/type_constructor_node.cpp
  src/type_transformers.cpp
)

target_include_directories(PolymorphicTypes
  PUBLIC
    inc
  PRIVATE
    ${CMAKE_JS_INC}
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
)

target_link_libraries(PolymorphicTypes
  PUBLIC
    ${CMAKE_JS_LIB}
)

set_target_properties(PolymorphicTypes 
  PROPERTIES 
    PREFIX "" 
    SUFFIX ".node"
)

# add_subdirectory(test)
