#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;


//class Server {
//
//};


class Client {
  public:
	/// Construct new
	Client(std::string host, short port):
	  m_io_service(),
	  m_own_endpoint(udp::v4(), port),
	  m_remote_endpoint(boost::asio::ip::address::from_string(host), port),
	  m_socket(m_io_service, m_remote_endpoint)
	{
		start_receive();
	}

	/// Send a string
	void send(std::string msg) {
		boost::shared_ptr<std::string> message(new std::string(msg));
		m_socket.async_send_to(boost::asio::buffer(*message), m_remote_endpoint,
			boost::bind(&Client::handle_send, this, message,
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::bytes_transferred));
	}

	/// Send a char
	void send(char ch) {
		send(std::string(ch, 1));
	}

	/// Get the data recieved and reset it
	std::string get() {
		std::string ret(m_data);
		m_data[0] = '\0';
		return ret;
	}

	/// Start listening for data
	void start_receive() {
		m_socket.async_receive_from(
			boost::asio::buffer(m_data, max_length), m_own_endpoint,
			boost::bind(&Client::handle_receive, this,
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::bytes_transferred));
	}

	/// What to do after data received
	void handle_receive(const boost::system::error_code&, std::size_t /*bytes_transferred*/) {
		start_receive();
	}

	/// What to do after sending stuff
	void handle_send(boost::shared_ptr<std::string> /*message*/,
	  const boost::system::error_code& /*error*/,
	  std::size_t /*bytes_transferred*/) {
	}

  private:

	boost::asio::io_service m_io_service;
	udp::endpoint m_own_endpoint;
	udp::endpoint m_remote_endpoint;
	udp::socket m_socket;
	enum { max_length = 4096 };
	char m_data[max_length];
};
