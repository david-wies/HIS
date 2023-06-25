cmake_minimum_required(VERSION 3.18)

# Set some basic project attributes
project (Replanner
	VERSION 0.1
	DESCRIPTION "A Hello World Project")

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS program_options filesystem)

include_directories(${Boost_INCLUDE_DIR})

# This project will output an executable file
add_executable(${PROJECT_NAME} k_replanner.cpp utils.h utils.cpp replanner.h replanner.cpp CompiledProblem.h CompiledProblem.cpp
planners/Planner.h planners/FastDownward.h planners/FastDownward.cpp planners/FastForward.cpp planners/FastForward.cpp planners/planner_factory.h
problem/Action.h problem/Action.cpp problem/Budget.h problem/Budget.cpp problem/Fact.h problem/Fact.cpp problem/Formula.h problem/Formula.cpp problem/Invariant.h problem/Invariant.cpp problem/Parameter.h problem/Parameter.cpp problem/PDDLObject.h problem/PDDLObject.cpp problem/Predicate.h problem/Predicate.cpp problem/Problem.h problem/Problem.cpp problem/Sensor.h problem/Sensor.cpp problem/Term.h problem/Term.cpp problem/Type.h problem/Type.cpp problem/Variable.h problem/Variable.cpp problem/Function.h problem/Function.cpp problem/FunctionFormula.h problem/FunctionFormula.cpp
compilations/limited_kp.h compilations/limited_kp.cpp compilations/kp.h compilations/kp.cpp compilations/kat.h compilations/kat.cpp compilations/kminrob.h compilations/kminrob.cpp compilations/compiler.h)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
