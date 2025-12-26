#ifndef __dds_output_port_h
#define __dds_output_port_h

#include <string>
#include <vector>

#include <dds/dds.hpp>

#include "delegate.h"

namespace abacos
{
    template<typename T>
    class DDS_Output_Port
    {
    public:
        explicit DDS_Output_Port(int port_identifier, std::string topic)
            : port_identifier_(port_identifier),
              topic_(std::move(topic)),
              participant_(0),
              dds_topic_(participant_, topic_),
              publisher_(participant_),
              writer_qos_(
                  publisher_.default_datawriter_qos()
                      << dds::core::policy::Reliability::Reliable()
                      << dds::core::policy::History::KeepLast(1)),
              writer_(publisher_, dds_topic_, writer_qos_)
        {}

        void write(T& data)
        {
            data.dds_publisher_time_stamp_ns(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());

            writer_.write(data);
        }

        int port_identifier()
        {
            return port_identifier_;
        }

    private:
        int port_identifier_;
        std::string topic_;

        dds::domain::DomainParticipant participant_;
        dds::topic::Topic<T> dds_topic_;
        dds::pub::Publisher publisher_;
        dds::pub::qos::DataWriterQos writer_qos_;
        dds::pub::DataWriter<T> writer_;
    };
}


#endif
