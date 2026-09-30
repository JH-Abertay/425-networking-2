/*	CMP425 / CMP501
	Lab 2 TCP/UDP client example - by Andrei Boiko

	This client can communicate with the server using either TCP or UDP.

	TCP version connects to the server, waits for others to connect,
	and then provides a name to the server. 

	UDP version sends introduction message to server and gets a reply.
*/

//#include <entt.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "utils.h";

auto serverPort = 53000;

constexpr auto MAX_SIZE = 64;

// We'll use this array to hold the messages we exchange with the server.
char buffer[MAX_SIZE];

sf::IpAddress serverIp(127, 0, 0, 1); // safer option as it avoids string parsing

sf::TcpSocket tcpSpcket;
sf::UdpSocket udpSocket;

void resetBuffer();
void putMessageInBuffer(const char* msg);

int main()
{
	ConnType connection_type = Undefined;

	Utils::printMsg("Client startup...");

	while (connection_type == Undefined) {
		
		Utils::printMsg("How do you want to communicate? Enter 1 for TCP, enter 2 for UDP:");

		std::string input_line;
		std::getline(std::cin, input_line);

		// Handle TCP communication
		if (!input_line.empty() && input_line.at(0) == '1') {
			connection_type = TCP;
			Utils::printMsg("TCP chosen, connecting to server...");

			// Connect the socket to the server.
			sf::Socket::Status status = tcpSpcket.connect(serverIp, serverPort);
			if (status != sf::Socket::Status::Done)
			{
				Utils::printMsg("Error connecting to server!", error);
				// FIXME: currently, the application will continue even if it fails to connect.
				// Handle this more gracefully.
			}
			else {
				size_t message_size; // size of the incoming message

				// Receive the initial message.
				resetBuffer();
				tcpSpcket.receive(buffer, sizeof(buffer), message_size);
				// FIXME: check for errors.
				Utils::printMsg(std::string(buffer).substr(0, MAX_SIZE), debug);

				// Receive confirmation message.
				resetBuffer();
				tcpSpcket.receive(buffer, sizeof(buffer), message_size);
				// FIXME: check for errors.
				Utils::printMsg(std::string(buffer).substr(0, MAX_SIZE), success);

				// Ask for a name and send it to server.
				Utils::printMsg("Please enter your name: ");
				// Read a line of text from the user.
				std::string name_input;
				std::getline(std::cin, name_input);

				// Note that we're not using a buffer here. 
				// We're sending the string directly as c_string (i.e. char array).
				tcpSpcket.send(name_input.c_str(), name_input.size());
				// FIXME: check for errors from send
			}
		}

		// Handle UDP communication
		else if (!input_line.empty() && input_line.at(0) == '2') {
			connection_type = UDP;
			Utils::printMsg("UDP chosen, binding socket...");

			// Bind the socket to a port. 
			// Note that for the client, we can just choose any available port.
			if (udpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done)
			{
				Utils::printMsg("Error binding socket!", error);
			}
			else {
				Utils::printMsg("Socket bound to port: " + std::to_string(udpSocket.getLocalPort()));
			}

			// This will let us try to introduce ourselves to the server forever.
			while (true) {
				// Ask for a name and send it to server.
				Utils::printMsg("Please enter your name: ");
				// Read a line of text from the user.
				std::string name_input;
				std::getline(std::cin, name_input);

				// Send name to server.
				// Note that we can use the same port number here as for TCP, even if both are
				// running at the same time, because TCP and UDP ports are separate.
				if (udpSocket.send(name_input.data(), name_input.size(), serverIp, serverPort) != sf::Socket::Status::Done) {
					Utils::printMsg("Error sending data.", error);
				}

				size_t message_size; // size of the incoming message
				std::optional<sf::IpAddress> incoming_ip;
				unsigned short incoming_port;

				resetBuffer();
				// Handle response.
				if (udpSocket.receive(buffer, sizeof(buffer), message_size, incoming_ip, incoming_port) == sf::Socket::Status::Done) {
					// FIXME: recieve will get any packet addressed to this IP/Port combination
					// ideally we want to filter out data from unexpected sources. Modify this to only
					// print responses from the server we're communicating with.

					Utils::printMsg(std::string(buffer).substr(0, message_size), success);
				}
				else {
					// This can trigger if the previous send did not reach the destination because port is inaccessible.
					// E.g. if the port is behind a firewall or not bound to UDP socket, the IP protocol stack will
					// return an ICMP (Internet Control Message Protocol) "Destination Unreachable" packet. 
					// This packet will be read on the next "recieve", hence the error.
					// Try sending a message to the server wuthout launching it or after closing it.
					Utils::printMsg("Error recieving data or previous send returned an error.", error);
				}
			}
		}
		else {
			Utils::printMsg("Incorrect input, please try again!", warning);
		}
	}

	// See if there is any further communication from the server.
	while (true)
	{
		Utils::printMsg("Waiting for the server...");

		// If using TCP, attempt to recieve to see if we're still connected
		if (connection_type == TCP) {
			size_t message_size;
			sf::Socket::Status status = tcpSpcket.receive(buffer, sizeof(buffer), message_size);
			if (status == sf::Socket::Status::Done) {
				// FIXME: Do something with the data, if there is any.
			}
			else if (status == sf::Socket::Status::Disconnected) {
				Utils::printMsg("Connection lost!", error);
				tcpSpcket.disconnect(); // cleanup socket
				return 0;
			}
			else if (status == sf::Socket::Status::Error) {
				Utils::printMsg("Somethign went wrong!", error);
				tcpSpcket.disconnect(); // cleanup socket
				return 0;
			}
		}

		// FIXME: There is no easy way to check if UDP "conenction" is live as it is connecitonless.
		// Come up with a solution to address this and notify the client that it can stop.
	}
}

///<summary> 
///Fills the buffer with ~ characters.
void resetBuffer()
{
	memset(buffer, '~', MAX_SIZE);
}

///<summary> 
///Replaces part of the data with specified message.
///<param name="msg">Message to replace data with.</param>
///</summary>
void putMessageInBuffer(const char* msg)
{
	memcpy(buffer, msg, strlen(msg));
}