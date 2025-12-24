#ifndef __output_port_h
#define __output_port_h

namespace abacos
{
    template <typename TComponent_Output_Port, typename T>
    concept Output_Port_Concept =
        requires(
            TComponent_Output_Port port,
            T &data) {
            { port.write(data) } -> std::same_as<void>;
        };
}

#endif
