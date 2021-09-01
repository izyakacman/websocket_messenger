#define BOOST_TEST_MODULE test_messenger_server

#include "precompiled.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_messenger_server)

BOOST_AUTO_TEST_CASE(test_valid_version)
{
    // The io_context is required for all I/O
    net::io_context ioc;
    /*
    // Create and launch a listening port
    std::make_shared<Listener>(ioc, tcp::endpoint{ tcp::v4(), 9999 }, std::make_shared<SharedState>())->Run();
    */

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()


//int main() {}