#include <vsomeip_input_port.h>
#include <output_port.h>
#include <component.h>
#include <periodic_timer_port.h>
#include <blocking_queue.h>
#include <behavior.h>
#include <node.h>

#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>


class Message 
{
    public:
        Message() = default;

        Message(char* buffer, int size_bytes)
        {
            (void)size_bytes;
            
            const char* read_pointer = buffer;

            std::memcpy(&abacos_publisher_time_stamp_ns, read_pointer, sizeof(abacos_publisher_time_stamp_ns)); read_pointer+= sizeof(abacos_publisher_time_stamp_ns);
            std::memcpy(&pre_publisher_serialization_time_stamp_ns, read_pointer, sizeof(pre_publisher_serialization_time_stamp_ns)); read_pointer+= sizeof(pre_publisher_serialization_time_stamp_ns);
            std::memcpy(&post_publisher_serialization_time_stamp_ns, read_pointer, sizeof(post_publisher_serialization_time_stamp_ns)); read_pointer+= sizeof(post_publisher_serialization_time_stamp_ns);
            std::memcpy(&vsomeip_publisher_time_stamp_ns, read_pointer, sizeof(vsomeip_publisher_time_stamp_ns)); read_pointer+= sizeof(vsomeip_publisher_time_stamp_ns);
            std::memcpy(&vsomeip_subscriber_time_stamp_ns, read_pointer, sizeof(vsomeip_subscriber_time_stamp_ns)); read_pointer+= sizeof(vsomeip_subscriber_time_stamp_ns);
            std::memcpy(&pre_subscriber_deserialization_time_stamp_ns, read_pointer, sizeof(pre_subscriber_deserialization_time_stamp_ns)); read_pointer+= sizeof(pre_subscriber_deserialization_time_stamp_ns);
            std::memcpy(&post_subscriber_deserialization_time_stamp_ns, read_pointer, sizeof(post_subscriber_deserialization_time_stamp_ns)); read_pointer+= sizeof(post_subscriber_deserialization_time_stamp_ns);
            std::memcpy(&abacos_subscriber_time_stamp_ns, read_pointer, sizeof(abacos_subscriber_time_stamp_ns)); read_pointer+= sizeof(abacos_subscriber_time_stamp_ns);
            std::memcpy(&payload_size_bytes, read_pointer, sizeof(payload_size_bytes)); read_pointer+= sizeof(payload_size_bytes);
            
            payload.assign(read_pointer, payload_size_bytes);

        }

        char* serialize()
        {
            std::stringstream stream;
            stream << abacos_publisher_time_stamp_ns;
            stream << pre_publisher_serialization_time_stamp_ns;
            stream << post_publisher_serialization_time_stamp_ns;
            stream << vsomeip_publisher_time_stamp_ns;
            stream << vsomeip_subscriber_time_stamp_ns;
            stream << pre_subscriber_deserialization_time_stamp_ns;
            stream << post_subscriber_deserialization_time_stamp_ns;
            stream << abacos_subscriber_time_stamp_ns;
            stream << payload_size_bytes;
            stream << payload;

            buffer = stream.str();
            
            return buffer.data();
        }

        int size_bytes()
        {
            return payload.size();
        }

        std::string payload;
        long abacos_publisher_time_stamp_ns;
        long pre_publisher_serialization_time_stamp_ns;
        long post_publisher_serialization_time_stamp_ns;
        long vsomeip_publisher_time_stamp_ns;
        long vsomeip_subscriber_time_stamp_ns;
        long pre_subscriber_deserialization_time_stamp_ns;
        long post_subscriber_deserialization_time_stamp_ns;
        long abacos_subscriber_time_stamp_ns;
        long payload_size_bytes;

        private:
            std::string buffer;
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
                message.abacos_subscriber_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

                std::cout << "message received - payload size: " << message.payload_size_bytes 
                          << " - abacos publisher time stamp [ns]: " << message.abacos_publisher_time_stamp_ns 
                          << " - pre serialization time stamp [ns]: " << message.pre_publisher_serialization_time_stamp_ns
                          << " - post serialization time stamp [ns]: " << message.post_publisher_serialization_time_stamp_ns
                          << " - vsomeip publisher time stamp [ns]: " <<  message.vsomeip_publisher_time_stamp_ns
                          << " - vsomeip subscriber time stamp [ns]: " << message.vsomeip_subscriber_time_stamp_ns
                          << " - pre deserialization time stamp [ns]: " << message.pre_subscriber_deserialization_time_stamp_ns
                          << " - post deserialization time stamp [ns]: " << message.post_subscriber_deserialization_time_stamp_ns
                          << " - abacos subscriber time stamp [ns]: " << message.abacos_subscriber_time_stamp_ns;
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