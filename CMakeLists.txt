cmake_minimum_required(VERSION 3.10)

project(scorpio_zadanie_rekrutacyjne_software)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)


# Exercise library
add_library(backend SHARED
  src/backend/motor.cpp
  src/backend/basic_tester.cpp
  src/backend/ros_interface.cpp
)
target_include_directories(backend PUBLIC
  include/backend_interface/
  include/backend/
)
#ament_target_dependencies(backend geometry_msgs rclcpp rclcpp_components std_msgs)
target_link_libraries(backend PUBLIC
  geometry_msgs::geometry_msgs__rosidl_typesupport_cpp
  std_msgs::std_msgs__rosidl_typesupport_cpp
  rclcpp::rclcpp
  rclcpp_components::component
)
rclcpp_components_register_nodes(backend "backend::RosInterface")


# Solution library
option(REFERENCE_SOLUTION "Build the maintainers' reference solution from reference_solution/ instead of src/solution/" OFF)
if(REFERENCE_SOLUTION)
  file(GLOB solution_SRC CONFIGURE_DEPENDS "reference_solution/*.cpp")
  set(solution_INCLUDE reference_solution/)
else()
  file(GLOB solution_SRC CONFIGURE_DEPENDS "src/solution/*.cpp")
  set(solution_INCLUDE include/solution/)
endif()
add_library(solution SHARED ${solution_SRC})
target_include_directories(solution PRIVATE
  ${solution_INCLUDE}
  include/backend_interface/
)
#ament_target_dependencies(solution geometry_msgs rclcpp rclcpp_components std_msgs)

target_link_libraries(solution PUBLIC
  geometry_msgs::geometry_msgs__rosidl_typesupport_cpp
  std_msgs::std_msgs__rosidl_typesupport_cpp
  rclcpp::rclcpp
  rclcpp_components::component
)

rclcpp_components_register_nodes(solution "solution::Solution")


install(TARGETS backend solution
  DESTINATION lib
)
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)


# Unit tests (run with `colcon test`, skip with `colcon build --cmake-args -DBUILD_TESTING=OFF`)
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  file(GLOB test_SRC CONFIGURE_DEPENDS "unit_tests/test_*.cpp")
  ament_add_gtest(tests ${test_SRC}
    src/backend/motor.cpp
    src/backend/basic_tester.cpp
  )
  target_include_directories(tests PRIVATE
    include/backend_interface/
    include/backend/
  )
endif()

ament_package()
