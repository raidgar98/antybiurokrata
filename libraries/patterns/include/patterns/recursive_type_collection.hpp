#pragma once

namespace patterns
{
    namespace RCNS // recursive concentrator namespace
    {
        template <typename T>
        concept recursive_concentrator_c =
            requires
        {
            typename T::Prev;
        };

        template <template <typename...> class _Storage /* Ex: std::variant, std::tuple */, typename _First /* can be empty struct */, recursive_concentrator_c LastElement>
        struct recursive_concentrator
        {
            // inspiration: https://stackoverflow.com/a/52393977
            template <typename T, typename... Args>
            struct concatenator;

            template <recursive_concentrator_c FArg, typename... Args>
            struct concatenator<_Storage<FArg, Args...>>
            {
                using type = typename concatenator<_Storage<typename FArg::Prev, FArg, Args...>>::type;
            };

            template <typename... Args>
            struct concatenator<_Storage<_First, Args...>>
            {
                using type = _Storage<_First, Args...>;
            };

            using result = concatenator<_Storage<LastElement>>::type;
        };

    } // namespace RCNS

} // namespace patterns