#include "network.hh"
#include "world.hh"
#include "player.hh"

void Server::listen() {
	ENetEvent e;
	while (!m_quit) {
		enet_host_service(m_server, &e, 1000);
		switch (e.type) {
		case ENET_EVENT_TYPE_CONNECT:
			std::cout << "Client connected from " << e.peer->address.host << ":" << e.peer->address.port << std::endl;
			// TODO: Proper generation
			m_world->addActor(100, 100, Actor::REMOTE);
			e.peer->data = &m_world->getActors().back();
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			// TODO: Handle receive
			printf ("A packet of length %u containing %s was received on channel %u.\n",
					e.packet -> dataLength,
					e.packet -> data,
					e.channelID);

			enet_packet_destroy (e.packet); // Clean-up
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
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
			printf ("A packet of length %u containing %s was received on channel %u.\n",
					e.packet -> dataLength,
					e.packet -> data,
					e.channelID);

			int curpcount = m_world->getActors().size();
			int newpcount = e.packet->dataLength / sizeof(PlayerSerialize);
			// New players?
			if (curpcount != newpcount) {
				m_world->addActor(100, 100, curpcount == 0 ? Actor::HUMAN : Actor::REMOTE);
			}
			enet_packet_destroy (e.packet); // Clean-up
			break;
		} case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "Server disconnected." << std::endl;
			// TODO: Handle
			e.peer->data = NULL;
			break;
		default:
			break;
		}
	}

}
