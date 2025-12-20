#ifndef __periodic_timer_port_h
#define __periodic_timer_port_h

#include <thread>
#include <vector>
#include "port_event.h"
#include "delegate.h"

#include <iostream>

namespace abacos
{
    class Periodic_Timer_Port
    {
        public:
            explicit Periodic_Timer_Port(int port_identifier, long period_ns)
                    : period_ns_(period_ns), port_identifier_(port_identifier)
            {
            }

            void listen(Delegate<void(Port_Event)> callback)
            {
                callbacks_.emplace_back(callback);
                runner_ = std::thread(&Periodic_Timer_Port::run, this);
            }

            int port_identifier() const
            {
                return port_identifier_;
            }

        private:
            std::thread runner_;
            long period_ns_;
            std::vector<Delegate<void(Port_Event)>> callbacks_;
            int port_identifier_;

            void run()
            {
                while (true)
                {
                    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

                    for (Delegate<void(Port_Event)> callback: callbacks_)
                        callback(Port_Event(port_identifier_));

                    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

                    long elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

                    long sleep_time_ns = period_ns_ - elapsed_ns;

                    std::cout << "Periodic_Timer_Port::run - sleeping for " << sleep_time_ns << " [ns]" << std::endl;

                    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time_ns));
                }
            }
    };
}

#endif
