add_executable(TypeParsersTest
  src/main.cpp
  src/CospanParserTest.cpp
  src/TypeParserTest.cpp
)

target_include_directories(TypeParsersTest PRIVATE inc)

target_link_libraries(TypeParsersTest
  PUBLIC
    TypeParsers
    gtest_main
)

add_test(NAME TypeParsersTest COMMAND TypeParsersTest)
