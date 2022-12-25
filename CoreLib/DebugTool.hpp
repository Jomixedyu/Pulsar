/*
* @Moudule     : DebugTool
* @Date        : 2021/05/20
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++20
*/

#ifndef _CORELIB_DEBUGTOOL
#define _CORELIB_DEBUGTOOL

#include <string>
#include <cassert>

#define DEBUG_INFO(info) std::format("info: {}; line: {}, file: {};", info, __LINE__, __FILE__)


#define nulable$(ptr) if(ptr != nullptr) ptr
#define assert_nulptr(ptr) if(ptr == nullptr) throw jxcorlib::NullPointerException(DEBUG_INFO(#ptr))

#define zero$(num) (num == (decltype(num))(0) ? throw jxcorlib::DivisionByZeroException(DEBUG_INFO(#num)) : num)
#define assert_zero(num) if(num == 0) throw jxcorlib::DivisionByZeroException(DEBUG_INFO(#num));

#endif // !_CORELIB_DEBUGTOOL
