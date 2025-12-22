#include <vsomeip_output_port.h>
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
    template<typename TOutput_Port>
    requires Output_Port_Concept<TOutput_Port, Message>
    class Simple_Publisher
            : public Component
    {
        public:
            explicit Simple_Publisher(long period_ms)
                    : periodic_timer_port_(0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::milliseconds(period_ms)).count()),
                    output_port_(1, "vsomeip_topic")
            {
                register_output_port(&output_port_);

                bind_behavior_to_input_port(&periodic_timer_port_,
                                            abacos::Behavior<Simple_Publisher>:: template create_behavior_delegate<&Simple_Publisher::publish>(
                                                    this));
            }

        private:
            void publish()
            {
                std::cout << "writting data" << std::endl;

                Message message;
                message.payload = "Hello from Cyclone DDS";
                message.payload_size_bytes = message.payload.size();

                message.abacos_publisher_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();
                
                output_port_.write(message);
            }

            abacos::Periodic_Timer_Port periodic_timer_port_;
            TOutput_Port output_port_;
            int current_id_ = 0;
    };
}



int main()
{
    abacos::Simple_Publisher<abacos::VSOMEIP_Output_Port<Message>> publisher(500);

    publisher.start();

    while(true);

    return 0;
}