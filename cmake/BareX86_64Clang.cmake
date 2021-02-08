set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_ASM_COMPILER_TARGET "x86_64-none-elf")
set(CMAKE_CXX_COMPILER_TARGET "x86_64-none-elf")
set(CMAKE_C_COMPILER_TARGET "x86_64-none-elf")

include(FindToolchain)
find_llvm_toolchain(TRIPLE x86_64-none-elf SET)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOS_STM32_FLAGS "-target x86_64-none-elf -D__ELF__ -mno-red-zone -fno-stack-protector -fomit-frame-pointer -mno-avx")

set(CMAKE_LINKER ${TOOLCHAIN_LD})
message(STATUS ${CMAKE_LINKER})
set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> -o <TARGET> --gc-sections -znorelro --warn-common --no-undefined --fatal-warnings --build-id=sha1 -Bsymbolic -shared <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> -o <TARGET> --lto-O3 --icf=all --gc-sections <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ${TOS_STM32_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_STM32_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${TOS_STM32_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOS_STM32_FLAGS}")

SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")

set(TOS_PROVIDE_LIBCXX ON)