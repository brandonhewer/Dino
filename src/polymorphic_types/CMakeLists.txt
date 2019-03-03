
add_library(PolymorphicTypes
  src/substitution.cpp
  src/type_constructor.cpp
  src/type_errors.cpp
  src/type_equality.cpp
  src/type_replacement.cpp
  src/type_to_string.cpp
  src/unification.cpp
)

target_include_directories(PolymorphicTypes
  PUBLIC
    inc
)

target_link_libraries(PolymorphicTypes)

add_subdirectory(test)
