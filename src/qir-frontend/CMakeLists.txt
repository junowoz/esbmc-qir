add_library(qirfrontend qir_language.cpp)

target_include_directories(qirfrontend
        PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(qirfrontend clibs ${Boost_LIBRARIES} fmt::fmt)
