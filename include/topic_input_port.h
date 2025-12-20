#ifndef __topic_input_port_h
#define __topic_input_port_h

#include <string>
#include <vector>
#include <iostream>

#include "delegate.h"
#include "port_event.h"

namespace abacos
{
    class Topic_Input_Port_Interface
    {
        public:
            virtual std::string topic() const = 0;
    };

    template<typename T>
    class Topic_Input_Port
            : public Topic_Input_Port_Interface
    {
        public:
            // std::string topic is TArgs for the communication mechanism
            explicit Topic_Input_Port(int port_identifier, std::string topic)
                    : port_identifier_(port_identifier), topic_(std::move(topic))
            {}

            // Should be degated to another object that implements a particular communication mechanism
            void receive(T data)
            {
                for (Delegate<void(T)> consumer : consumers_)
                    consumer(data);

                for (Delegate<void(Port_Event)> listener : listeners_)
                    listener(Port_Event(port_identifier_));
            }

            void listen(Delegate<void(Port_Event)> listener)
            {
                listeners_.push_back(listener);
            }

            void bind(Delegate<void(T)> consumer)
            {
                consumers_.push_back(consumer);
            }

            int port_identifier() const
            {
                return port_identifier_;
            }

            std::string topic() const override
            {
                return topic_;
            }

        private:
            int port_identifier_;
            std::string topic_;
            std::vector<Delegate<void(Port_Event)>> listeners_;
            std::vector<Delegate<void(T)>> consumers_;
    };
}

#endif
