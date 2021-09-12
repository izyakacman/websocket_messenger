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
    std::shared_ptr<ClientSession> client_session_4;
    std::ostringstream oss_4;
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
    oss_4 = {};
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
        client_session_4 = std::make_shared<ClientSession>(ioc_, oss_4);
        client_session_4->Run(host, port);

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

        // Add user to the group

        client_session_1->Write("#add_user_to_group g1 "s + user_name_3);

        while (oss_1.str().empty()) std::this_thread::yield();

        BOOST_CHECK(oss_1.str() == "User "s + user_name_3 +  " added to the group g1\n");

        ClearData();

        BOOST_TEST_MESSAGE("setup fixture end");
    }

    ~Fixture() // teardown fixture
    {
        BOOST_TEST_MESSAGE( "teardown fixture" );

        // del_user_from_group

        client_session_2->Write("#del_user_from_group g1 "s + user_name_3);

        std::this_thread::sleep_for(1s);
        BOOST_TEST_MESSAGE(oss_2.str());


        while (oss_2.str() != "You are not administrator of this group") std::this_thread::yield(); // wait data

        client_session_1->Close();
        client_session_2->Close();
        client_session_3->Close();
        client_session_4->Close();
        ioc_.stop();
        ioc_thread_.join();

        BOOST_TEST_MESSAGE("teardown fixture end");
    }

    std::thread ioc_thread_;
    net::io_context ioc_;
};

BOOST_AUTO_TEST_SUITE(test_messenger_server, *utf::fixture<Fixture>())

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

BOOST_AUTO_TEST_CASE(send_to_all_user)
{
    BOOST_TEST_MESSAGE("send_to_all_user");

    // Send to all users

    client_session_1->Write("Hi all");

    WaitData(); // wait data

    BOOST_CHECK(oss_1.str() == user_name_1 + ": Hi all\n"s);
    BOOST_CHECK(oss_2.str() == user_name_1 + ": Hi all\n"s);
    BOOST_CHECK(oss_3.str() == user_name_1 + ": Hi all\n"s);

    ClearData();

    BOOST_TEST_MESSAGE("ok");
}

BOOST_AUTO_TEST_CASE(send_to_group)
{
    BOOST_TEST_MESSAGE("send_to_group");

    // Send to group

    client_session_1->Write("#to_group g1 hi g1");

    while (oss_1.str().empty() || oss_3.str().empty()) std::this_thread::yield(); // wait data

    BOOST_CHECK(oss_1.str() == user_name_1 + ": hi g1\n"s);
    BOOST_CHECK(oss_2.str().empty() == true);
    BOOST_CHECK(oss_3.str() == user_name_1 + ": hi g1\n"s);    

    ClearData();

    BOOST_TEST_MESSAGE("ok");
}

void get_groups_list(std::shared_ptr<ClientSession>& client_session, std::ostringstream& oss)
{
    client_session->Write("#get_groups_list");
    while(oss.str() != "Groups list:\ng1\n") std::this_thread::yield(); // wait data

    ClearData();
}

BOOST_AUTO_TEST_CASE(get_groups_list_test)
{
    BOOST_TEST_MESSAGE("get_groups_list");

    get_groups_list(client_session_1, oss_1);
    get_groups_list(client_session_2, oss_2);
    get_groups_list(client_session_3, oss_3);

    BOOST_CHECK(true);

    BOOST_TEST_MESSAGE("ok");
}

void get_group_users(std::shared_ptr<ClientSession>& client_session, std::ostringstream& oss)
{
    client_session->Write("#get_group_users g1");
    while(oss.str() != "Group g1 has next users\n"s + user_name_1 + " - admin\n" + user_name_3 + "\n")
        std::this_thread::yield(); // wait data
    ClearData();
}

BOOST_AUTO_TEST_CASE(get_group_users_test)
{
    BOOST_TEST_MESSAGE("get_group_users");
    get_group_users(client_session_1, oss_1);
    get_group_users(client_session_3, oss_3);

    client_session_2->Write("#get_group_users g1");

    while (oss_2.str() != "You are not allowed to get users of this group\n")
        std::this_thread::yield(); // wait data

    ClearData();

    BOOST_CHECK(true);
    BOOST_TEST_MESSAGE("ok");
}

BOOST_AUTO_TEST_CASE(not_registered)
{
    BOOST_TEST_MESSAGE("not_registered");

    client_session_4->Write("spam");

    while (oss_4.str() != "You are not registered\n")
        std::this_thread::yield(); // wait data

    BOOST_CHECK(true);
    BOOST_TEST_MESSAGE("ok");
}

BOOST_AUTO_TEST_SUITE_END()
