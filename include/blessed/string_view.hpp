#ifndef BLESSED_STRING_VIEW_HPP_
#define BLESSED_STRING_VIEW_HPP_

#include <blessed/blessed.hpp>

#ifdef BLESSED_CXX17
#include <string_view>

    namespace blessed
    {
        using string_view = std::string_view;
    }
#else
#endif

#endif
