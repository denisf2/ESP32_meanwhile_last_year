#ifndef MESSAGEDISPATCHER_H_
#define MESSAGEDISPATCHER_H_

#include <functional>
#include <map>
#include <type_traits>
#include <utility>

template <typename T>
using MoveConstructible = std::enable_if_t<std::is_move_constructible_v<T> >;

template <typename T, typename U, typename = MoveConstructible<U> >
class MessageDispatcher
{
    private:
        using MessageHandler = std::function<auto (T&, U&&) -> void>;

        // std::unordered_map does not fit here couse Arduino's String
        // class lacks a compatible std::hash specialization
        std::map<T, MessageHandler> m_handlers;
        MessageHandler m_unknownMessageHandler;

    public:
        auto RegisterHandler(const T& aType, MessageHandler aHandler) -> void
        {
            m_handlers[aType] = aHandler;
        }

        auto RegisterUnknownMwssageHandler(MessageHandler aHandler) -> void
        {
            m_unknownMessageHandler = aHandler;
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