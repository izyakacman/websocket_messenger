#include "precompiled.hpp"

#include "client.hpp"

int main( int argc, char** argv )
{
	if (argc < 3)
	{
		std::cout << "Usage: messenger_client <host> <port>";
		return -1;
	}

	// The io_context is required for all I/O
	net::io_context ioc;

	// Launch the asynchronous operation
	auto client_session = std::make_shared<ClientSession>(ioc, std::cout);
	client_session->Run(argv[1], argv[2]);

	// Run the I/O service. The call will return when
	// the socket is closed.
	std::thread thr = std::thread([&ioc] {ioc.run(); });

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	net::signal_set signals(ioc, SIGINT, SIGTERM);
	signals.async_wait(
		[&client_session](boost::system::error_code const&, int)
		{
			client_session->Close();
		});

	std::string line;

	while (getline(std::cin, line))
	{
		client_session->Write( line );
	}

	thr.join();
}