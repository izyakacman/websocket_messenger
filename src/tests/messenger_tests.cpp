#define BOOST_TEST_MODULE messenger_test

#include "precompiled.hpp"
#include "../client/client.hpp"
#include "../server/listener.hpp"

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

namespace {

    constexpr auto user_name = "test_user";
    constexpr auto host = "127.0.0.1";
    constexpr auto port = "9999";
    std::shared_ptr<ClientSession> client_session;
    std::ostringstream oss;
}

struct Fixture
{
    Fixture() // setup fixture
    {
        BOOST_TEST_MESSAGE( "setup fixture" );

        // Server
        std::make_shared<Listener>(ioc_, tcp::endpoint{ tcp::v4(), 9999 }, std::make_shared<SharedState>())->Run();

        // Client
        client_session = std::make_shared<ClientSession>(ioc_, oss); 
        client_session->Run(host, port);

        ioc_thread_ = std::thread([this] { ioc_.run(); });
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
    using namespace std::string_literals;

    while (!client_session->IsConnect()); // wait connect

    oss = {}; // clear stream

    client_session->Write( "#iam "s + user_name );

    while (oss.str().empty()); // wait data

    BOOST_CHECK(oss.str() == "Hello "s + user_name + "\n");
}

//BOOST_AUTO_TEST_CASE()
//{
//
//}

BOOST_AUTO_TEST_SUITE_END()
