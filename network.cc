#include "network.hh"
#include "world.hh"
#include "player.hh"

#define MYID 80

void Server::listen() {
	ENetEvent e;
	while (!m_quit) {
		enet_host_service(m_server, &e, 1000);
		switch (e.type) {
		case ENET_EVENT_TYPE_CONNECT: {
			std::cout << "Client connected from " << e.peer->address.host << ":" << e.peer->address.port << std::endl;
			// TODO: Proper generation
			m_world->addActor(300, 100, Actor::REMOTE);
			e.peer->data = &m_world->getActors().back();
			// Send starting info
			std::string msg = "  ";
			msg[0] = MYID;
			msg[1] = m_world->getActors().size();
			ENetPacket* packet = enet_packet_create(msg.c_str(), msg.length(), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(e.peer, 0, packet); // Send through channel 0
			enet_host_flush(m_server); // Don't dispatch events
			break;
		} case ENET_EVENT_TYPE_RECEIVE: {
			// TODO: Handle receive
			//printf ("A packet of length %u containing %s was received on channel %u.\n",
					//e.packet -> dataLength,
					//e.packet -> data,
					//e.channelID);
			Actor* pl = static_cast<Actor*>(e.peer->data);
			if (pl && e.packet->dataLength == 1) {
				switch (e.packet->data[0]) {
					case OnlinePlayer::MOVE_LEFT: pl->move(-1); break;
					case OnlinePlayer::MOVE_RIGHT: pl->move(1); break;
					case OnlinePlayer::JUMP: pl->jump(); break;
					case OnlinePlayer::DUCK: pl->duck(); break;
					case OnlinePlayer::STOP_MOVING: pl->stop(); break;
					case OnlinePlayer::STOP_JUMPING: pl->end_jumping(); break;
					case OnlinePlayer::ACTION: pl->action(); break;
				}
			}
			enet_packet_destroy (e.packet); // Clean-up
			break;
		} case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "Client disconnected." << std::endl;
			// TODO: Delete player
			e.peer->data = NULL;
			break;
		default:
			break;
		}
	}
}


void Client::listen() {
	ENetEvent e;
	while (!m_quit) {
		enet_host_service(m_client, &e, 1000);
		switch (e.type) {
		case ENET_EVENT_TYPE_RECEIVE: {
			// TODO: Handle receive
			//printf ("A packet of length %u containing %s was received on channel %u.\n",
					//e.packet -> dataLength,
					//e.packet -> data,
					//e.channelID);
			// Get id
			if (e.packet->data[0] == MYID) {
				m_id = e.packet->data[1];
			} else {
				// Update state
				m_world->update(std::string((char*)e.packet->data, e.packet->dataLength), this);
			}
			// Clean-up
			enet_packet_destroy(e.packet);
			break;
		} case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "Server disconnected." << std::endl;
			// TODO: Handle properly
			e.peer->data = NULL;
			throw std::runtime_error("Server disconnected.");
			break;
		default:
			break;
		}
	}

}
