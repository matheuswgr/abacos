namespace abacos
{
    template<typename T>
    concept Serializable =
    requires(T data, char* serialized, int size_bytes)
    {
        T{serialized, size_bytes};
        { data.serialize() } -> std::same_as<char*>;
        { data.size_bytes() } -> std::same_as<int>;
    };
}