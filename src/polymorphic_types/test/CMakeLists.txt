add_executable(PolymorphicTypesTest
  src/main.cpp
  src/unification_test.cpp
)

target_include_directories(PolymorphicTypesTest PRIVATE inc)

target_link_libraries(PolymorphicTypesTest
  PUBLIC
    PolymorphicTypes
    gtest_main
)

add_test(NAME PolymorphicTypesTest COMMAND PolymorphicTypesTest)
