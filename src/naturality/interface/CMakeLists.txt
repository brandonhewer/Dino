add_library(NaturalityNode SHARED
  src/addon.cpp
  src/graph_builder.cpp
  src/natural_transformation_node.cpp
)

target_include_directories(NaturalityNode
  PUBLIC
    inc
  PRIVATE
    ${CMAKE_JS_INC}
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
)

target_link_libraries(NaturalityNode
  PUBLIC
    ${CMAKE_JS_LIB}
  PRIVATE
    Naturality
    PolymorphicTypes
    TypeParsers
)

set_target_properties(NaturalityNode 
  PROPERTIES 
    OUTPUT_NAME "Naturality"
    PREFIX "" 
    SUFFIX ".node"
)
