
add_library(Naturality SHARED
  src/cospan.cpp
  src/cospan_equality.cpp
  src/cospan_renaming.cpp
  src/cospan_to_string.cpp
  src/natural_composition.cpp
  src/natural_transformation.cpp
  src/natural_transformers.cpp
)

target_include_directories(Naturality
  PUBLIC
    inc
)

target_link_libraries(Naturality
  PUBLIC
    PolymorphicTypes
)

add_subdirectory(interface)
add_subdirectory(test)
