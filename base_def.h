#ifndef SRC_BASEDEF_H
#define SRC_BASEDEF_H

typedef unsigned int u_int;
typedef unsigned long u_long;

#if __cplusplus >= 201103L
#define NOEXCEPT_AVAL noexcept
#define CONSTEXPR_AVAL constexpr
#endif

#endif

#ifndef BASE_DEF_H
#define BASE_DEF_H

// Гарантируем определение макроса для Windows
#ifndef NOEXCEPT_AVAL
#if defined(_MSC_VER) || defined(__cplusplus) && __cplusplus >= 201103L
#define NOEXCEPT_AVAL noexcept
#else
#define NOEXCEPT_AVAL
#endif
#endif

#endif // BASE_DEF_H