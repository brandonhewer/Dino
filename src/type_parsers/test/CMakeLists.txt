add_executable(TypeParsersTest
  src/main.cpp
  src/cospan_parser_test.cpp
  src/type_parser_test.cpp
)

target_include_directories(TypeParsersTest PRIVATE inc)

target_link_libraries(TypeParsersTest
  PUBLIC
    TypeParsers
    gtest_main
)

add_test(NAME TypeParsersTest COMMAND TypeParsersTest)
