#ifndef MESSAGEDISPATCHER_H_
#define MESSAGEDISPATCHER_H_

#include <functional>
#include <map>
#include <type_traits>
#include <utility>

template <typename MessageType, typename ParamType>
class MessageDispatcher
{
    static_assert(std::is_move_constructible_v<ParamType>
                    , "ParamType must be move-constructible");

    private:
        using HandlerFunction = std::function<auto (MessageType&, ParamType&&) -> void>;

        // std::unordered_map does not fit here couse Arduino's String
        // class lacks a compatible std::hash specialization
        using HandlersMap = std::map<MessageType, HandlerFunction>;

        HandlersMap m_handlers;
        HandlerFunction m_unknownMessageHandler;

    public:
        auto RegisterHandler(const MessageType& aType, HandlerFunction aHandler) -> void
        {
            m_handlers[aType] = std::move(aHandler);
        }

        auto RegisterUnknownMessageHandler(HandlerFunction aHandler) -> void
        {
            m_unknownMessageHandler = std::move(aHandler);
        }

        auto Dispatch(MessageType aMessageType, ParamType&& aParams)
        {
            if(const auto it = m_handlers.find(aMessageType); m_handlers.end() != it)
                it->second(aMessageType, std::move(aParams));
            else if(m_unknownMessageHandler)
                m_unknownMessageHandler(aMessageType, std::move(aParams));
            // else
            //     log_w("%s, No suitable message handler", TAG);
        }
};
#endif