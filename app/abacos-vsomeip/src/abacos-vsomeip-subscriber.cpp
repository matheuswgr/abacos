#include <vsomeip_input_port.h>
#include <output_port.h>
#include <component.h>
#include <periodic_timer_port.h>
#include <blocking_queue.h>
#include <behavior.h>
#include <node.h>

#include <iostream>

class Message 
{
    public:
        Message() = default;

        Message(char* buffer, int size_bytes)
        {
            (void)size_bytes;
            payload = buffer;
        }

        std::string payload;

        char* serialize()
        {
            return payload.data();
        }

        int size_bytes()
        {
            return payload.size();
        }
};

namespace abacos
{
    template<typename TInput_Port, typename TData>
    requires Input_Port_Concept<TInput_Port, TData>
    class Simple_Subscriber
            : public Component
    {
        public:
            explicit Simple_Subscriber()
                    : input_port_(1, "dds_topic")
            {
                bind_behavior_to_input_port(&input_port_,
                                            Behavior<Simple_Subscriber, TData, TInput_Port>:: template create_behavior_delegate<&Simple_Subscriber::receive>(
                                                    this, &input_port_));
            }

            void receive(Message message)
            {

                std::cout << ", payload=\"" << message.payload << "\""
                        << std::endl;
            }

        private:
            TInput_Port input_port_;
    };
}



int main()
{
    abacos::Simple_Subscriber<abacos::VSOMEIP_Input_Port<Message>, Message> subscriber;

    subscriber.start();


    while(true);

    return 0;
}