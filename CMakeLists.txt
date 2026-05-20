cmake_minimum_required(VERSION 3.20)

project(Orderbook VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# neovim / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ROOT    "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR "${ROOT}/src")
set(LIB_DIR "${ROOT}/libs")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${SRC_DIR}/*.inl"
  "${SRC_DIR}/*.cpp"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
  "${SRC_DIR}/*.hpp"
)

file(GLOB_RECURSE SRC_SUBDIRS LIST_DIRECTORIES true
  "${SRC_DIR}/*"
)

set(ALL_INCLUDE_DIRS
  "${SRC_DIR}"
  "${LIB_DIR}"
)

foreach(p ${SRC_SUBDIRS})
  if(IS_DIRECTORY "${p}")
    list(APPEND ALL_INCLUDE_DIRS "${p}")
  endif()
endforeach()

list(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)

add_subdirectory("${LIB_DIR}/FTXUI")

add_executable(Orderbook ${SOURCES} ${HEADERS})

target_include_directories(Orderbook PRIVATE
  ${ALL_INCLUDE_DIRS}
)

target_link_libraries(Orderbook PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
)

source_group(TREE "${SRC_DIR}" PREFIX "src" FILES
  ${SOURCES} ${HEADERS}
)

# sanitizers
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g3 -O0 -fno-omit-frame-pointer)
add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g3 -O1)
add_link_options(-fsanitize=address,undefined)


#enable_testing()
#file(GLOB_RECURSE TEST_SOURCE CONFIGURE_DEPENDS
#  "${ROOT}/test/*.hpp"
#  "${ROOT}/test/*.cpp"
#)
#
#add_executable(OrderBookTests ${TEST_SOURCE})
#target_link_libraries(OrderBookTests PRIVATE gtest gtest_main)
#add_test(NAME OrderbookTests COMMAND OrderBookTests)

