
add_library(TypeParsers
  src/cospan_parser.cpp
  src/transformation_parser.cpp
)

target_include_directories(TypeParsers
  PUBLIC
    inc
  PRIVATE
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(TypeParsers
  PUBLIC
    Naturality
    PolymorphicTypes
  PRIVATE
    ${Boost_LIBRARIES}
)

add_subdirectory(test)
