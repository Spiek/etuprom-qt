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
#define PACKET_DESCRIPTOR_USER_SELF_GET_INFO "user.self.getinfo"

// Contact Module (require User Module)
#define PACKET_DESCRIPTOR_CONTACT_GET_LIST "contact.getlist"
#define PACKET_DESCRIPTOR_CONTACT_ALTERED "contact.altered"

// Chatmanager Module
#define PACKET_DESCRIPTOR_CHAT_PRIVATE "chat.private"

#endif // PACKETTYPES_H
