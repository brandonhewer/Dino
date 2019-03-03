add_executable(NaturalityTest
  src/main.cpp
  src/composition_test.cpp
)

target_include_directories(NaturalityTest PRIVATE inc)

target_link_libraries(NaturalityTest
  PUBLIC
    Naturality
    gtest_main
)

add_test(NAME NaturalityTest COMMAND NaturalityTest)
