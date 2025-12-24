#include <sstream>
#include <cstring>
#include <string>
#include <iostream>

class Message 
{
    public:
        Message() = default;

        Message(char* buffer, int size_bytes)
        {
            (void)size_bytes;

            char* read_pointer = buffer;

            std::memcpy(&abacos_publisher_time_stamp_ns, read_pointer, sizeof(abacos_publisher_time_stamp_ns)); read_pointer+= sizeof(abacos_publisher_time_stamp_ns);
            std::memcpy(&vsomeip_publisher_time_stamp_ns, read_pointer, sizeof(vsomeip_publisher_time_stamp_ns)); read_pointer+= sizeof(vsomeip_publisher_time_stamp_ns);
            std::memcpy(&vsomeip_subscriber_time_stamp_ns, read_pointer, sizeof(vsomeip_subscriber_time_stamp_ns)); read_pointer+= sizeof(vsomeip_subscriber_time_stamp_ns);
            std::memcpy(&abacos_subscriber_time_stamp_ns, read_pointer, sizeof(abacos_subscriber_time_stamp_ns)); read_pointer+= sizeof(abacos_subscriber_time_stamp_ns);
            std::memcpy(&payload_size_bytes, read_pointer, sizeof(payload_size_bytes)); read_pointer+= sizeof(payload_size_bytes);
            
            payload.assign(read_pointer, payload_size_bytes);
        }

        char* serialize()
        {
            buffer.resize(payload.size() + 5*sizeof(long));

            char* write_pointer = buffer.data();

            payload_size_bytes = payload.size();

            std::memcpy(write_pointer, &abacos_publisher_time_stamp_ns, sizeof(abacos_publisher_time_stamp_ns)); write_pointer+= sizeof(abacos_publisher_time_stamp_ns);
            std::memcpy(write_pointer, &vsomeip_publisher_time_stamp_ns,  sizeof(vsomeip_publisher_time_stamp_ns)); write_pointer+= sizeof(vsomeip_publisher_time_stamp_ns);
            std::memcpy(write_pointer, &vsomeip_subscriber_time_stamp_ns,  sizeof(vsomeip_subscriber_time_stamp_ns)); write_pointer+= sizeof(vsomeip_subscriber_time_stamp_ns);
            std::memcpy(write_pointer, &abacos_subscriber_time_stamp_ns,  sizeof(abacos_subscriber_time_stamp_ns)); write_pointer+= sizeof(abacos_subscriber_time_stamp_ns);
            std::memcpy(write_pointer, &payload_size_bytes,  sizeof(payload_size_bytes)); write_pointer+= sizeof(payload_size_bytes);
            std::memcpy(write_pointer, payload.data(),  payload_size_bytes); write_pointer+= payload_size_bytes;
            
            return buffer.data();
        }

        int size_bytes()
        {
           return buffer.size();
        }

        std::string payload;
        long abacos_publisher_time_stamp_ns;
        long vsomeip_publisher_time_stamp_ns;
        long vsomeip_subscriber_time_stamp_ns;
        long abacos_subscriber_time_stamp_ns;
        long payload_size_bytes;

        private:
            std::string buffer;
};