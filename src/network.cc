#include "config.hh"

#ifdef USE_NETWORK
#include <Box2D.h>
#include "network.hh"
#include "world.hh"
#include "player.hh"

namespace {
	static const char MYID = 80; // Identifies packet as being player id info.
}


void Server::listen() {
	ENetEvent e;
	while (!m_quit) {
		enet_host_service(m_host, &e, 20);
		switch (e.type) {
		case ENET_EVENT_TYPE_CONNECT: {
			std::cout << "Client connected from " << e.peer->address.host << ":" << e.peer->address.port << std::endl;
			char newid = m_world->getActors().size() + 1;
			// Spawn player
			b2Vec2 pos = m_world->randomSpawn();
			m_world->addActor(pos.x, pos.y, Actor::REMOTE, newid);
			// Assign
			e.peer->data = &m_world->getActors().back();
			{ // Send starting info
				std::string msg = "  ";
				msg[0] = MYID;
				msg[1] = newid;
				ENetPacket* packet = enet_packet_create(msg.c_str(), msg.length(), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(e.peer, 0, packet); // Send through channel 0
				enet_host_flush(m_host); // Don't dispatch events
			}
			{ // Send initial world data
				std::string msg = m_world->serialize(false);
				ENetPacket* packet = enet_packet_create(msg.c_str(), msg.length(), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(e.peer, 0, packet); // Send through channel 0
				enet_host_flush(m_host); // Don't dispatch events
			}
			break;
		} case ENET_EVENT_TYPE_RECEIVE: {
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
		enet_host_service(m_host, &e, 20);
		switch (e.type) {
		case ENET_EVENT_TYPE_RECEIVE: {
			if (e.packet->data[0] == MYID) {
				// Get id
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

#endif // USE_NETWORK
