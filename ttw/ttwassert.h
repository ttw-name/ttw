
#ifndef __TTW__TTWASSERT_H
#define __TTW__TTWASSERT_H

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

static auto ass_logger_ = ttw::LogManage::GetLogManage()->getLogger("root");

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define LIKELY(x)      (x)
#   define UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define ASSERT(x) \
    if(UNLIKELY(!(x))) { \
        LOG_ERROR(ass_logger_) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << ttw::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define ASSERT2(x, w) \
    if(UNLIKELY(!(x))) { \
        LOG_ERROR(ass_logger_) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << ttw::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif