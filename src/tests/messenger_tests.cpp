#define BOOST_TEST_MODULE messenger_test

#include "precompiled.hpp"
#include "../client/client.hpp"
#include "../server/listener.hpp"

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace {

    constexpr auto user_name_1 = "test_user_1";
    constexpr auto user_name_2 = "test_user_2";
    constexpr auto user_name_3 = "test_user_3";
    constexpr auto host = "127.0.0.1";
    constexpr auto port = "9999";
    std::shared_ptr<ClientSession> client_session_1;
    std::ostringstream oss_1;
    std::shared_ptr<ClientSession> client_session_2;
    std::ostringstream oss_2;
    std::shared_ptr<ClientSession> client_session_3;
    std::ostringstream oss_3;
}

void WaitData()
{
    while (oss_1.str().empty() || oss_2.str().empty() || oss_3.str().empty()) 
        std::this_thread::yield();
}

void ClearData()
{
    oss_1 = {};
    oss_2 = {};
    oss_3 = {};
}

struct Fixture
{
    Fixture() // setup fixture
    {
        BOOST_TEST_MESSAGE( "setup fixture" );

        // Delete params file
        std::remove( "server.json" );

        // Server
        std::make_shared<Listener>(ioc_, tcp::endpoint{ tcp::v4(), 9999 }, std::make_shared<SharedState>())->Run();

        // Clients
        client_session_1 = std::make_shared<ClientSession>(ioc_, oss_1);
        client_session_1->Run(host, port);
        client_session_2= std::make_shared<ClientSession>(ioc_, oss_2);
        client_session_2->Run(host, port);
        client_session_3 = std::make_shared<ClientSession>(ioc_, oss_3);
        client_session_3->Run(host, port);

        ioc_thread_ = std::thread([this] { ioc_.run(); });

        // Wait connect

        while (!client_session_1->IsConnect() ||
               !client_session_2->IsConnect() ||
               !client_session_3->IsConnect())
        {
            std::this_thread::yield();;
        }

        // Clear streams

        ClearData();

        // Users registeration

        client_session_1->Write("#iam "s + user_name_1);
        client_session_2->Write("#iam "s + user_name_2);
        client_session_3->Write("#iam "s + user_name_3);

        WaitData();

        // Check ack

        BOOST_CHECK(oss_1.str() == "Hello "s + user_name_1 + "\n");
        BOOST_CHECK(oss_2.str() == "Hello "s + user_name_2 + "\n");
        BOOST_CHECK(oss_3.str() == "Hello "s + user_name_3 + "\n");

        ClearData();

        // Create group

        client_session_1->Write("#create_group g1");

        while (oss_1.str().empty()) std::this_thread::yield();

        BOOST_CHECK(oss_1.str() == "Group g1 is created\n");

        ClearData();

        BOOST_TEST_MESSAGE("setup fixture end");
    }

    ~Fixture() // teardown fixture
    {
        BOOST_TEST_MESSAGE( "teardown fixture" );

        client_session_1->Close();
        client_session_2->Close();
        client_session_3->Close();
        ioc_.stop();
        ioc_thread_.join();
    }

    std::thread ioc_thread_;
    net::io_context ioc_;
};

BOOST_AUTO_TEST_SUITE(test_messenger_server, *utf::fixture<Fixture>())
/*
BOOST_AUTO_TEST_CASE(add_user)
{
    BOOST_TEST_MESSAGE("add_user");

    WaitData();

    BOOST_CHECK(oss_1.str() == "Hello "s + user_name_1 + "\n");
    BOOST_CHECK(oss_2.str() == "Hello "s + user_name_2 + "\n");
    BOOST_CHECK(oss_3.str() == "Hello "s + user_name_3 + "\n");
    
    ClearData();

    BOOST_TEST_MESSAGE("ok");
}*/

BOOST_AUTO_TEST_CASE(send_to_user)
{
    BOOST_TEST_MESSAGE("send_to_user");

    // Send to user2

    client_session_1->Write("#to "s + user_name_2 + " hello");

    while (oss_1.str().empty() || oss_2.str().empty()) std::this_thread::yield(); // wait data

    BOOST_CHECK(oss_1.str() == user_name_1 + ": hello\n"s);
    BOOST_CHECK(oss_2.str() == user_name_1 + ": hello\n"s);
    BOOST_CHECK(oss_3.str().empty() == true);

    ClearData();

    BOOST_TEST_MESSAGE("ok");
}

BOOST_AUTO_TEST_SUITE_END()
