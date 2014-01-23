#pragma once

#ifdef USE_NETWORK

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <enet/enet.h>


/// RAII Wrapper
struct ENetContainer {
	ENetContainer() { enet_initialize(); }
	~ENetContainer() { enet_deinitialize(); }
};


class World;

class NetworkObject: public boost::noncopyable {
  public:
	NetworkObject(World* world): m_quit(false), m_id(), m_world(world), m_host(NULL), m_peer(NULL) { }

	~NetworkObject() {
		terminate();
		if (m_host) enet_host_destroy(m_host);
		m_host = NULL;
	}

	virtual void listen() { }

	/// Send a string
	void send(std::string msg, int flag = 0) {
		ENetPacket* packet = enet_packet_create (msg.c_str(), msg.length(), flag);
		if (m_peer) enet_peer_send(m_peer, 0, packet); // Send to peer through channel 0
		else enet_host_broadcast(m_host, 0, packet); // Send through channel 0 to all peers
	}

	/// Send a char
	void send(char ch, int flag = 0) {
		send(std::string(1, ch), flag);
	}

	void terminate() { m_quit = true; m_thread.join(); }

  protected:
	bool m_quit;
	char m_id;
	World* m_world;
	ENetAddress m_address;
	ENetHost* m_host;
	ENetPeer* m_peer;
	boost::thread m_thread;
};

class Server: public NetworkObject {
  public:
	Server(World* world, int port): NetworkObject(world) {
		m_address.host = ENET_HOST_ANY;
		m_address.port = port;
		// Create host at address, peerCount, channelCount, unlimited up/down bandwith
		m_host = enet_host_create(&m_address, 16, 2, 0, 0);
		if (m_host == NULL)
			throw std::runtime_error("An error occurred while trying to create an ENet host.");
		// Start listener thread
		m_thread = boost::thread(boost::bind(&Server::listen, boost::ref(*this)));
	}

	void listen();
};


class Client: public NetworkObject {
  public:
	/// Construct new
	Client(World* world): NetworkObject(world), m_id(0) { }

	void connect(std::string host, int port) {
		// Create host at address, peerCount, channelCount, unlimited up/down bandwith
		m_host = enet_host_create(NULL, 2, 2, 0, 0);
		if (m_host == NULL)
			throw std::runtime_error("An error occurred while trying to create an ENet host.");

		enet_address_set_host(&m_address, host.c_str());
		m_address.port = port;
		// Initiate the connection, allocating the two channels 0 and 1, with 0 data.
		m_peer = enet_host_connect(m_host, &m_address, 2, 0);
		if (m_peer == NULL)
			throw std::runtime_error("No available peers for initiating an ENet connection.");
		// Wait up to 5 seconds for the connection attempt to succeed.
		ENetEvent event;
		if (enet_host_service (m_host, &event, 5000) > 0 &&
		  event.type == ENET_EVENT_TYPE_CONNECT) {
			// Start listener thread
			m_thread = boost::thread(boost::bind(&Client::listen, boost::ref(*this)));
		} else { // Failure
			enet_peer_reset(m_peer);
			throw std::runtime_error(std::string("Connection to ") + host + " failed!");
		}
	}

	void listen();

	char getID() const { return m_id; }

  private:
	char m_id;
};

#else

struct Client {

	char getID() { return 0; }

};

#endif // USE_NETWORK
