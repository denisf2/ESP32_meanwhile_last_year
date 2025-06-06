#ifndef FUNCTIONAL_AUX_H__
#define FUNCTIONAL_AUX_H__

namespace Functonal
{
    template<typename T, typename F>
    auto and_then(std::optional<T> aOpt, F&& aFunc) -> decltype(aFunc(std::move(*aOpt)))
    {
        if (aOpt)
            return aFunc(std::move(*aOpt));

        return decltype(aFunc(std::move(*aOpt))){};
    }

    template<typename T, typename F>
    auto transform(std::optional<T> aOpt, F&& aFunc) -> std::optional<decltype(aFunc(std::move(*aOpt)))>
    {
        if (aOpt)
            return aFunc(std::move(*aOpt));

        return std::nullopt;
    }
} // namespace

#endif