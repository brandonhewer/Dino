
add_library(Naturality SHARED
  src/cospan.cpp
  src/cospan_composition.cpp
  src/cospan_equality.cpp
  src/cospan_shared_count.cpp
  src/cospan_substitution.cpp
  src/cospan_to_string.cpp
  src/cospan_zip.cpp
  src/natural_composition.cpp
  src/natural_transformation.cpp
  src/unify_cospan_with_type.cpp
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
