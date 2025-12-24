#ifndef __input_port_h
#define __input_port_h

#include <concepts>

#include "port_event.h"
#include "delegate.h"

namespace abacos
{
    template <typename TComponent_Input_Port, typename T>
    concept Input_Port_Concept =
        requires(
            TComponent_Input_Port port,
            int id,
            std::string topic,
            Delegate<void(const Port_Event &)> listener,
            Delegate<void(const T &)> consumer) {
            TComponent_Input_Port{id, topic};

            { port.listen(listener) } -> std::same_as<void>;

            { port.bind(consumer) } -> std::same_as<void>;
        };

}

#endif
