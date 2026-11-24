# CMake generated Testfile for 
# Source directory: /mnt
# Build directory: /mnt/build-no-cancel
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(student_tests "/mnt/build-no-cancel/student_tests")
set_tests_properties(student_tests PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT "60" _BACKTRACE_TRIPLES "/mnt/CMakeLists.txt;65;add_test;/mnt/CMakeLists.txt;0;")
