#pragma once
#include <functional>

/*
NOTE FOR READER:
    SSignal (SafeSignal) gives a safe connections insert and remove during iteration
    since its a linked list its safe and O(1) to insert/remove nodes
    but when firing its slow since the connections could live very far away in memory so the 
    CPU cant optimize that
*/



// ----------------------------- DECLARATION -------------------------//

namespace jman 
{
    template <typename ...Args>
    class ssignal 
    {
        using Callback = std::function<void(Args...)>;

        public:
            struct Connection 
            {
                bool connected() const;
                bool once() const;
                void disconnect();
            };

            ssignal() = default;
            ~ssignal() noexcept;

            ssignal(const ssignal& other) = delete;
            ssignal& operator=(const ssignal& other) = delete;

            ssignal(ssignal&& other) noexcept = default;
            ssignal& operator=(ssignal&& other) noexcept = default;

            std::size_t connections() const noexcept;

            Connection connect(Callback callback);
            Connection once(Callback callback);

            void fire(Args&&...);
            void disconnect_all();
    };
}

// ------------------------------------ IMPLEMENTATION -------------------------------//