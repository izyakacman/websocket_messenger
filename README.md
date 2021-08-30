# messenger_server
WebSocket messenger server

Listening port: 9999

Commands list:

  - #iam <user_name> - register user;
  - #to <user_name> <message> - send message to the user;
  - #to_group <group_name> - send meesage to all users in the group;
  - #create_group <group_name> - create group, user who create group becomes its administarator;
  - #add_user_to_group <group> <user> - add user to the group, only group administrator can do this;
  - #get_groups_list - get groups list;
  - #get_group_users <group> - get users of the group, only group administrator can do this;
  - #del_user_from_group <group> <user> - delete user from the group, only group administrator can do this;
  - #del_group <group> - delete group, only group administrator can do this;
  - \<message\> - send message to all users.
