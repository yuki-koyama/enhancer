cmake_minimum_required(VERSION 3.16)

project(enhancer CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENHANCER_USE_QT_FEATURES "Build Qt features" OFF)
option(ENHANCER_BUILD_QT_TESTS "Build Qt-based tests" OFF)
option(ENHANCER_USE_ADVANCED_PARAMETERS "Use additional advanced parameters" OFF)

set(ENHANCER_VERT_SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders/enhancer.vs" CACHE INTERNAL "")
set(ENHANCER_FRAG_SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders/enhancer.fs" CACHE INTERNAL "")

if(ENHANCER_USE_QT_FEATURES)
  find_package(Qt6 COMPONENTS OpenGL OpenGLWidgets Widgets Gui REQUIRED)

  find_package(Eigen3 REQUIRED)
  if((NOT TARGET Eigen3::Eigen) AND (DEFINED EIGEN3_INCLUDE_DIR))
    add_library(AliasEigen3 INTERFACE)
    target_include_directories(AliasEigen3 INTERFACE ${EIGEN3_INCLUDE_DIR})
    add_library(Eigen3::Eigen ALIAS AliasEigen3)
  endif()

  file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/include/enhancer/*.hpp)
  file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
  file(GLOB shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.fs ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vs)

  set(CMAKE_AUTORCC ON)
  qt_add_resources(resources ${CMAKE_CURRENT_SOURCE_DIR}/enhancer-resources.qrc)

  add_library(enhancer STATIC ${headers} ${sources} ${resources} ${shaders})
  target_link_libraries(enhancer Eigen3::Eigen Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Widgets Qt6::Gui)
  target_include_directories(enhancer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
  if(ENHANCER_USE_ADVANCED_PARAMETERS)
    target_compile_definitions(enhancer PUBLIC ENHANCER_WITH_LIFT_GAMMA_GAIN)
  endif()

  install(FILES ${headers} DESTINATION include/enhancer)
  install(TARGETS enhancer ARCHIVE DESTINATION lib)

  if(ENHANCER_BUILD_QT_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/simple-widget-test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/cpp-export-test)
  endif()
else()
  set(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/enhancer/enhancer.hpp)

  add_library(enhancer INTERFACE)
  target_sources(enhancer INTERFACE ${headers})
  target_include_directories(enhancer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  if(ENHANCER_USE_ADVANCED_PARAMETERS)
    target_compile_definitions(enhancer INTERFACE ENHANCER_WITH_LIFT_GAMMA_GAIN)
  endif()

  install(FILES ${headers} DESTINATION include/enhancer)
endif()
