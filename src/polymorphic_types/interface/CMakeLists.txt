add_library(PolymorphicTypesNode SHARED
  src/addon.cpp
  src/atomic_type_reference.cpp
  src/type_constructor_node.cpp
)

target_include_directories(PolymorphicTypesNode
  PUBLIC
    inc
  PRIVATE
    ${CMAKE_JS_INC}
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
)

target_link_libraries(PolymorphicTypesNode
  PUBLIC
    PolymorphicTypes
    ${CMAKE_JS_LIB}
  PRIVATE
    ${Boost_LIBRARIES}
)

set_target_properties(PolymorphicTypesNode 
  PROPERTIES 
    OUTPUT_NAME "PolymorphicTypes"
    PREFIX "" 
    SUFFIX ".node"
)
