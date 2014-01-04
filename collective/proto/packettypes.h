#ifndef PACKETTYPES_H
#define PACKETTYPES_H
//
// Packet Descriptors
//
// here we define the Protocol Descriptors for all packet types which will be used in PacketProcessor and it's Modules
//

// User Module
#define PACKET_DESCRIPTOR_USER_LOGIN "user.login"
#define PACKET_DESCRIPTOR_USER_LOGOUT "user.logout"
#define PACKET_DESCRIPTOR_USER_SELF_GET_INFO "user.myprofile"
#define PACKET_DESCRIPTOR_USER_PING "user.ping"
#define PACKET_DESCRIPTOR_USER_PONG "user.pong"


// Contact Module (require User Module)
#define PACKET_DESCRIPTOR_CONTACT_GET_LIST "messenger.contactlist"
#define PACKET_DESCRIPTOR_CONTACT_ALTERED "messenger.contactaltered"

// Chatmanager Module
#define PACKET_DESCRIPTOR_CHAT_PRIVATE "messenger.chatmessage"

#define PACKET_DESCRIPTOR_BULKFILE "messenger.bulkfile"

#endif // PACKETTYPES_H
