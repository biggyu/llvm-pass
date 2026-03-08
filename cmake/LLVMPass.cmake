function(add_llvm_pass PASS_NAME)
  add_llvm_pass_plugin(${PASS_NAME}
    ${ARGN}
  )
  target_compile_features(${PASS_NAME} PRIVATE cxx_std_17)
  target_include_directories(${PASS_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()