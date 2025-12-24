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
            : period_ns_(period_ns),
              port_identifier_(port_identifier),
              event_(port_identifier)
        {
        }

        void listen(Delegate<void(const Port_Event &)> callback)
        {
            callbacks_.emplace_back(callback);

            if (!running_)
            {
                running_ = true;
                runner_ = std::thread(&Periodic_Timer_Port::run, this);
            }
        }

        int port_identifier() const
        {
            return port_identifier_;
        }

    private:
        std::thread runner_;
        long period_ns_;
        int port_identifier_;

        Port_Event event_;

        std::vector<Delegate<void(const Port_Event &)>> callbacks_;

        bool running_ = false;

        void run()
        {
            using clock = std::chrono::steady_clock;

            while (true)
            {
                auto begin = clock::now();

                for (auto &callback : callbacks_)
                    callback(event_);

                auto end = clock::now();

                auto elapsed_ns =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

                auto sleep_time_ns = period_ns_ - elapsed_ns;
                
                if (sleep_time_ns > 0)
                {
                    std::this_thread::sleep_for(
                        std::chrono::nanoseconds(sleep_time_ns));
                }
            }
        }
    };

}

#endif
