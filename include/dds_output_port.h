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
                    : port_identifier_(port_identifier), topic_(std::move(topic)),
                                        participant_(0),
                    dds_topic_(participant_, topic_),
                    publisher_(participant_),
                    writer_qos_(publisher_.default_datawriter_qos()
                                    << dds::core::policy::Reliability::Reliable()
                                    << dds::core::policy::History::KeepLast(1)),
                    writter_(publisher_, dds_topic_, writer_qos_)
            {}

            void write(T data)
            {
                std::cout << "writing to topic: " << topic_ << std::endl;
                writter_.write(data);
            }

        private:
            int port_identifier_;
            std::string topic_;
            std::vector<Delegate<void(T)>> subscriber_callbacks_;

            dds::domain::DomainParticipant participant_;
            dds::topic::Topic<T> dds_topic_;
            dds::pub::Publisher publisher_;
            dds::pub::qos::DataWriterQos writer_qos_;
            dds::pub::DataWriter<T> writter_;
    };
}

#endif
