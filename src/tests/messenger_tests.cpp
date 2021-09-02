#define BOOST_TEST_MODULE messenger_test

#include "precompiled.hpp"
#include "../client/client.hpp"

#include "../server/listener.hpp"

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

struct Fixture
{
    Fixture() // setup fixture
    {
        BOOST_TEST_MESSAGE( "setup fixture" );

        // Create and launch a listening port
        std::make_shared<Listener>(ioc_, tcp::endpoint{ tcp::v4(), 9999 }, std::make_shared<SharedState>())->Run();

        ioc_thread_ = std::thread( [this]{ ioc_.run(); });
    }

    ~Fixture() // teardown fixture
    {
        BOOST_TEST_MESSAGE( "teardown fixture" );

        ioc_.stop();
        ioc_thread_.join();
    }

    std::thread ioc_thread_;
    net::io_context ioc_;
};

BOOST_AUTO_TEST_SUITE(test_messenger_server, *utf::fixture<Fixture>())

BOOST_AUTO_TEST_CASE(test_add_user)
{
    net::io_context ioc;
    std::ostringstream oss;
    std::make_shared<session>(ioc, oss)->run("localhost", "9999", "#iam test_user");
    ioc.run();

    BOOST_CHECK(oss.str() == "Hello test_user\n");
}

BOOST_AUTO_TEST_SUITE_END()
