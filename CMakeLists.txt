cmake_minimum_required(VERSION 3.14)
project(half_fast_and_half_furious)

include(FetchContent)

set(TINY_ENGINE_NO_EXAMPLES ON CACHE BOOL "")

FetchContent_Declare(
   Tiny_engine
   GIT_REPOSITORY https://github.com/zen4xx/tiny_engine.git
   GIT_TAG        master
   )

FetchContent_MakeAvailable(Tiny_engine)

find_package(OpenAL QUIET)

if(OpenAL_FOUND)
   message(STATUS "Using system OpenAL")
else()
   message(STATUS "OpenAL not found, downloading OpenAL Soft")

   set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
   set(ALSOFT_UTILS    OFF CACHE BOOL "" FORCE)
   set(ALSOFT_TESTS    OFF CACHE BOOL "" FORCE)
   set(ALSOFT_INSTALL  OFF CACHE BOOL "" FORCE)

   FetchContent_Declare(
      openal
      GIT_REPOSITORY https://github.com/kcat/openal-soft.git
      master
      GIT_SHALLOW TRUE
   )

FetchContent_MakeAvailable(openal)
endif()

set(CMAKE_DIR "${tiny_engine_SOURCE_DIR}/tiny_engine_assets")
set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(COPY ${CMAKE_DIR} DESTINATION ${TARGET_DIR}) # for nessasery tiny_engine assets

add_executable(half_fast_and_half_furious main.cpp Car.cpp)

target_include_directories(half_fast_and_half_furious PRIVATE ${tiny_engine_SOURCE_DIR} ${OPENAL_INCLUDE_DIR})
target_link_libraries(half_fast_and_half_furious PRIVATE Tiny_engine ${OPENAL_LIBRARY})
