add_executable(fe_tests
    test_Config.cpp
    test_MathUtils.cpp
    test_Image.cpp
    test_DNA.cpp
    test_Flower.cpp
    test_Petals.cpp
    test_Stats.cpp
    test_NativeAPI.cpp
)
target_link_libraries(fe_tests PRIVATE FlowerEvolver Catch2::Catch2WithMain)
target_compile_features(fe_tests PRIVATE cxx_std_17)

include(Catch)
catch_discover_tests(fe_tests)
