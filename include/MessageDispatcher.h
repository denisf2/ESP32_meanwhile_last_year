#ifndef MESSAGEDISPATCHER_H_
#define MESSAGEDISPATCHER_H_

#include <functional>
#include <map>
#include <type_traits>
#include <utility>

template <typename T, typename U>
class MessageDispatcher
{
    static_assert(std::is_move_constructible_v<U>
                    , "U must be move-constructible");

    private:
        using HandlerFunction = std::function<auto (T&, U&&) -> void>;

        // std::unordered_map does not fit here couse Arduino's String
        // class lacks a compatible std::hash specialization
        using HandlersMap = std::map<T, HandlerFunction>;

        HandlersMap m_handlers;
        HandlerFunction m_unknownMessageHandler;

    public:
        auto RegisterHandler(const T& aType, HandlerFunction aHandler) -> void
        {
            m_handlers[aType] = std::move(aHandler);
        }

        auto RegisterUnknownMwssageHandler(HandlerFunction aHandler) -> void
        {
            m_unknownMessageHandler = std::move(aHandler);
        }

        auto Dispatch(T aMessageType, U&& aParams)
        {
            if(auto it = m_handlers.find(aMessageType); m_handlers.end() != it)
                it->second(aMessageType, std::move(aParams));
            else if(m_unknownMessageHandler)
                m_unknownMessageHandler(aMessageType, std::move(aParams));
            // else
            //     log_w("%s, No suitable message handler", TAG);
        }
};
#endif