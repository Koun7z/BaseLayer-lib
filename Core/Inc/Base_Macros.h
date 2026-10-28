#ifndef BASE_MACROS_H__
#define BASE_MACROS_H__

#define SELECT_PARAM_SET_1_TO_3(_1, _2, _3, _NAME, ...) _NAME

#if defined(__GNUC__)
#  define SELECT_PARAM_SET_0_TO_3(_0, _1, _2, _3, _NAME, ...) _NAME
#endif

#endif /* BASE_MACROS_H__ */