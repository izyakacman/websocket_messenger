// main.cpp: определяет точку входа для приложения.
//

#include "precompiled.hpp"
#include "listener.hpp"
#include "shared_state.hpp"

using namespace std;

int main()
{
    // The io_context is required for all I/O
    net::io_context ioc;

    // Create and launch a listening port
    std::make_shared<Listener>(ioc, tcp::endpoint{ tcp::v4(), 9999 }, std::make_shared<SharedState>())->Run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc](boost::system::error_code const&, int)
        {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            ioc.stop();
        });

    // Run the I/O service on the main thread
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    return 0;
}
