
add_library(PolymorphicTypes
  src/type_constructor.cpp
  src/type_errors.cpp
  src/type_transformers.cpp
)

target_include_directories(PolymorphicTypes
  PUBLIC
    inc
)

target_link_libraries(PolymorphicTypes)

add_subdirectory(interface)
# add_subdirectory(test)
