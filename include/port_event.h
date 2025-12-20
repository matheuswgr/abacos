#ifndef __port_event_h
#define __port_event_h

namespace abacos
{
    class Port_Event
    {
        private:
            int port_identifier_;

        public:
            explicit Port_Event(int port_identifier)
                    : port_identifier_(port_identifier)
            {}

            int port_identifier() const
            {
                return port_identifier_;
            }
    };
}

#endif
