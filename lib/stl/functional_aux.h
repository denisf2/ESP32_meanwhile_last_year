#ifndef FUNCTIONAL_AUX_H__
#define FUNCTIONAL_AUX_H__

#include <optional>
#include <utility>
#include <type_traits>

namespace Functional
{
    template<typename T, typename F>
    auto and_then(std::optional<T> aOpt, F&& aFunc)
    {
        using Result = std::invoke_result_t<F, T&&>;

        if (aOpt)
            return std::forward(aFunc)(std::move(*aOpt));

        return Result{};
    }

    template<typename T, typename F>
    auto transform(std::optional<T> aOpt, F&& aFunc)
    {
        using Result = std::invoke_result_t<F, T&&>;

        if (aOpt)
            return std::optional<Result>(aFunc(std::move(*aOpt)));

        return std::optional<Result>(std::nullopt);
    }

    template<typename T, typename Predicate>
    auto filter(std::optional<T> aOpt, Predicate&& aPred) -> std::optional<T>
    {
        if (aOpt && std::forward<Predicate>(aPred)(*aOpt))
            return std::move(aOpt);

        return std::nullopt;
    }

template<typename T>
class Optional {
    std::optional<T> m_opt;

public:
    // Constructors
    Optional() = default;
    Optional(std::nullopt_t) {}
    Optional(const T& value)
        : m_opt(value) {}
    Optional(T&& value)
        : m_opt(std::move(value)) {}
    Optional(const std::optional<T>& opt)
        : m_opt(opt) {}
    Optional(std::optional<T>&& opt)
        : m_opt(std::move(opt)) {}

    // Monadic operations
    template<typename F>
    auto and_then(F&& f) &&
    {
        using Result = std::invoke_result_t<F, T&&>;
        if (m_opt) {
            return std::forward<F>(f)(std::move(*m_opt));
        }
        return Result{};
    }

    template<typename F>
    auto transform(F&& f) &&
    {
        using U = std::invoke_result_t<F, T&&>;
        if (m_opt) {
            return Optional<U>(std::forward<F>(f)(std::move(*m_opt)));
        }
        return Optional<U>(std::nullopt);
    }

    template<typename Predicate>
    auto filter(Predicate&& pred) &&
    {
        if (m_opt && std::forward<Predicate>(pred)(*m_opt)) {
            return Optional<T>(std::move(*this));
        }
        return Optional<T>(std::nullopt);
    }

    // Value access
    // explicit operator bool() const
    // {
    //     return m_opt.has_value();
    // }
    // T&& unwrap() &&
    // {
    //     return std::move(*m_opt);
    // }
    // const T& unwrap() const &
    // {
    //     return *m_opt;
    // }

    // Conversion
    // std::optional<T> into_optional() &&
    // {
    //     return std::move(m_opt);
    // }
    operator std::optional<T>() &&
    {
        return std::move(m_opt);
    }
};

// Factory function
template<typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}

} // namespace

#endif