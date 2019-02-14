
add_library(Naturality SHARED
  src/cospan.cpp
  src/natural_transformation.cpp
  src/natural_transformers.cpp
)

target_include_directories(Naturality
  PUBLIC
    inc
)

target_link_libraries(Naturality
  PRIVATE
    PolymorphicTypes
)

add_subdirectory(interface)
# add_subdirectory(test)
