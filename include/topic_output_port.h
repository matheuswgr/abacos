#ifndef __topic_output_port_h
#define __topic_output_port_h

#include <string>
#include <vector>

#include "delegate.h"
#include "topic_input_port.h"

namespace abacos
{
    class Topic_Output_Port_Interface
    {
        public:
            virtual std::string topic() const = 0;
            virtual void subscribe(Topic_Input_Port_Interface* input_port_interface) = 0;
    };

    template<typename T>
    class Topic_Output_Port
            : public Topic_Output_Port_Interface
    {
        public:
            // std::string topic is TArgs for the communication mechanism
            explicit Topic_Output_Port(int port_identifier, std::string topic)
                    : port_identifier_(port_identifier), topic_(std::move(topic))
            {}

            void write(T data)
            {
                // Should be degated to another object that implements a particular communication mechanism
                for (Delegate<void(T)>  callback : subscriber_callbacks_)
                    callback(data);
            }

            void subscribe(Topic_Input_Port_Interface* input_port_interface) override
            {
                Topic_Input_Port<T>* input_port = static_cast<Topic_Input_Port<T>*>(input_port_interface);
                subscriber_callbacks_.push_back(Delegate<void(T)>::template create_delegate<Topic_Input_Port<T>, &Topic_Input_Port<T>::receive>(input_port));
            }

            std::string topic() const override
            {
                return topic_;
            }

        private:
            int port_identifier_;
            std::string topic_;
            std::vector<Delegate<void(T)>> subscriber_callbacks_;
    };
}

#endif
