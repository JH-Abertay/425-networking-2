/*	CMP425 / CMP501
        Lab 2 multi-client server example - by Andrei Boiko

        This server can communicate with multiple clients using either TCP or
   UDP.

        TCP version waits for connections of MAX_TCP_CLIENTS. When all
   connections are made, it asks for names and saves them. The server then waits
   for input before disconnecting all clients and exiting.

        UDP version recieves messages from "clients" and tracks them by client
   name. If more than MAX_UDP_CLIENTS attempt to talk to the server, it will
   respond, saying it is busy.
*/

// #include <entt.hpp>
#include "utils.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <memory>

constexpr auto MAX_NAME_LENGTH = 12;
constexpr auto MAX_TCP_CLIENTS = 3;
constexpr auto MAX_UDP_CLIENTS = 3;

auto serverPort = 53000;

struct TcpClient {
  std::unique_ptr<sf::TcpSocket> socket;
  std::string name = "Unknown";
};

struct UdpClient {
  sf::IpAddress ip;
  unsigned short port;
  std::string name = "Unknown";
};

std::vector<TcpClient> tcpClients;
std::vector<UdpClient> udpClients;

void runTcpServer();
void runUdpServer();

int main() {
  Utils::printMsg("Server startup...");
  ConnType connection_type = Undefined;

  while (connection_type == Undefined) {

    Utils::printMsg(
        "What kind of server are we? Enter 1 for TCP, enter 2 for UDP:");

    std::string input_line;
    std::getline(std::cin, input_line);
    if (!input_line.empty() && input_line.at(0) == '1') {
      connection_type = TCP;
      Utils::printMsg("UDP chosen, configuring listener.");
      runTcpServer();
    } else if (!input_line.empty() && input_line.at(0) == '2') {
      connection_type = UDP;
      Utils::printMsg("UDP chosen, binding socket...");
      runUdpServer();
    } else {
      Utils::printMsg("Incorrect input, please try again!", warning);
    }
  }
}

bool tryAcceptTcpClient(sf::TcpListener &listenerSocket,
                        std::unique_ptr<sf::TcpSocket> &tcpSocket) {
  // Accept a new TCP connection to the server socket.
  // This will update the newTcpSocket with new connection.
  sf::Socket::Status status = listenerSocket.accept(*tcpSocket);
  if (status == sf::Socket::Status::Done) {
    // Add client
    tcpClients.push_back({std::move(tcpSocket), "Unknown"});
    auto sockPos = tcpClients.size() - 1;

    std::string message = "Connection accepted from ";

    // Check get incoming IP and Port and print them in a message.
    sf::IpAddress incoming_ip =
        *tcpClients.at(sockPos).socket->getRemoteAddress();
    auto port = tcpClients.at(sockPos).socket->getRemotePort();
    Utils::printMsg(
        message + incoming_ip.toString() + ":" + std::to_string(port), success);

    // Construct welcome message and print it for debug purposes.
    Utils::printMsg("Sending welcome message...");
    std::string welcome = "Hello, you are client #";
    welcome.append(std::to_string(tcpClients.size()));
    welcome.append(" of ");
    welcome.append(std::to_string(MAX_TCP_CLIENTS));
    welcome.append(". Please wait...");

    Utils::printMsg(welcome, debug);
    // Send welcome message to the client. Note that we're not using a buffer
    // here. We're sending the string directly as c_string (i.e. char array).
    // The message might end up shorter than the MAX_SIZE on the other end,
    // which is fine for now...
    if (tcpClients.at(sockPos).socket->send(welcome.c_str(), welcome.size()) ==
        sf::Socket::Status::Done) {
      // Message was sent successfully
      Utils::printMsg("Welcome message successfully sent!", info);
    } else {
      Utils::printMsg("Error sending welcome message to new client!", error);
    }
  } else {
    Utils::printMsg("Error accepting connection!", error);
    return false;
  }
  return true;
}

void runTcpServer() {
  // Create a TCP socket that we'll uise to listen for incoming connections.
  sf::TcpListener listenerSocket;

  // Make the socket listen for connections.
  if (listenerSocket.listen(serverPort) == sf::Socket::Status::Done) {
    Utils::printMsg("Listening on port " + std::to_string(serverPort));
  } else {
    Utils::printMsg("Error binding listener socket!", error);
  }

  // With blocking calls, we'll wait until all MAX_TCP_CLIENTS connect before
  // we continue
  while (tcpClients.size() < MAX_TCP_CLIENTS) {
    // Create new socket object for commnication with the client and push to
    // vector.
    auto newTcpSocket = std::make_unique<sf::TcpSocket>();
    bool tcpClientAccepted = false;
    int timesToTryConnecting = 30;

    // Maybe try and make this time buffered with Chrono or something.
    for (int i = 0; i < 30; i++) {
      tcpClientAccepted = tryAcceptTcpClient(listenerSocket, newTcpSocket);

      if (!tcpClientAccepted) {
        Utils::printMsg("Trying to connect to client again...", info);
      } else {
        break;
      }
    }
  }

  // We don't need the listener socket after we accepted MAX_TCP_CLIENTS, so
  // close it.
  // FIXME: But what if a client disconnects and we want to allow another one
  // in? Fix this
  if (tcpClients.size() >= MAX_TCP_CLIENTS)
    listenerSocket.close();

  // Once we have all MAX_TCP_CLIENTS, ask them for names.
  for (TcpClient &client : tcpClients) {
    std::string message = "Everyone is connected. Please identify yourself.";
    if (client.socket->send(message.c_str(), message.size()) !=
        sf::Socket::Status::Done) {
      Utils::printMsg("Failed to send everyone is connected message!", error);
    }
    // FIXME: Handle for errors
  }

  // Get the names back and save them.
  for (TcpClient &client : tcpClients) {
    char buffer[MAX_NAME_LENGTH];
    size_t message_size;
    if (client.socket->receive(buffer, sizeof(buffer), message_size) !=
        sf::Socket::Status::Done) {
      Utils::printMsg("Failed to get name of client!", error);
      continue;
    }

    // For names that are shorter than MAX_NAME_LENGTH, we can just cut the
    // string at the correct number of characters. But if we had a longer name
    // than the MAX_NAME_LENGTH, we would only get part of the name here, and
    // the rest would be read on the next, read call for this socket.
    //
    // FIXME: modify the code to handle variable length names.
    client.name = std::string(buffer).substr(0, message_size);
  }

  // FIXME: Now tell everyone who we have connected to the server:
  for (TcpClient &client : tcpClients) {
    // finish this
    std::string send_string = "Connected to server.";

    if (client.socket.get()->send(send_string.c_str(), send_string.size()) !=
        sf::Socket::Status::Done) {
      Utils::printMsg("Error sending message to client name: " + client.name);
    }
  }

  // Check if we're done. If so, disconnect all clients.
  bool done = false;

  while (!done) {
    Utils::printMsg("Are we done? Enter X to disconnect from clients.");
    std::string input_line;
    std::getline(std::cin, input_line);

    if (!input_line.empty() &&
        (input_line.at(0) == 'X' || input_line.at(0) == 'x')) {
      for (TcpClient &client : tcpClients) {
        // Get socket's IP and Port before we disconnect.
        sf::IpAddress incoming_ip = *client.socket->getRemoteAddress();
        auto port = client.socket->getRemotePort();
        // Disconnect socket.
        client.socket->disconnect();
        // Print message.
        Utils::printMsg(client.name + " disconnected! : " +
                        incoming_ip.toString() + ":" + std::to_string(port));
      }
      done = true;
    } else {
      Utils::printMsg("Incorrect input, please try again!", warning);
    }
  }

  Utils::printMsg("All clients disconnected, we're done here!", warning);
}

void runUdpServer() {
  // get client message, extract IP and name, save them
  // if > MAX_UDP_CLIENTS, reply saying we're busy
  // if same name/IP combo recieved, reply saying they are already here
  // otherwise reply with a welcome message and # of others here

  sf::UdpSocket udpSocket;

  // Bind the socket to a port.
  // Note that we can use the same port number as for TCP.
  if (udpSocket.bind(serverPort) != sf::Socket::Status::Done) {
    Utils::printMsg("Error binding socket!", error);
  } else {
    Utils::printMsg("Socket bound to port: " +
                    std::to_string(udpSocket.getLocalPort()));
  }

  Utils::printMsg("Waiting for messages...");

  bool done = false;

  while (!done) {
    // Get message from client.
    char buffer[MAX_NAME_LENGTH];
    size_t message_size;
    std::optional<sf::IpAddress> incoming_ip;
    unsigned short incoming_port;

    if (udpSocket.receive(buffer, sizeof(buffer), message_size, incoming_ip,
                          incoming_port) == sf::Socket::Status::Done) {
      std::string message = "Message recieved from: ";
      message.append(std::string(buffer).substr(0, message_size));
      message.append("[");
      message.append((*incoming_ip).toString() + ":" +
                     std::to_string(incoming_port));
      message.append("]");

      Utils::printMsg(message, success);

      if (udpClients.size() >= MAX_UDP_CLIENTS) {
        // We're already at capacity, so let client know.
        std::string msg_full = "Sorry, we're full. Try again later!";
        if (udpSocket.send(msg_full.c_str(), msg_full.size(), *incoming_ip,
                           incoming_port) != sf::Socket::Status::Done) {
          Utils::printMsg("Error sending data!", error);
        } else {
          Utils::printMsg("We're full, replying to client:", warning);
          Utils::printMsg(msg_full, debug);
        }
      } else {
        bool isAlreadyPresent = false;
        std::string name = "Unknown";
        for (UdpClient client : udpClients) {
          if (client.ip == *incoming_ip && client.port == incoming_port) {
            // If IP and Port match, we already have this client
            isAlreadyPresent = true;
            name = client.name;
          }
        }

        if (isAlreadyPresent) {
          // Client already registerd, so let them know.
          std::string msg_present =
              "Hi " + name + ", it appears you've already registered!";
          if (udpSocket.send(msg_present.c_str(), msg_present.size(),
                             *incoming_ip,
                             incoming_port) != sf::Socket::Status::Done) {
            Utils::printMsg("Error sending data!", error);
          } else {
            Utils::printMsg("Client already regitered, letting them know:",
                            warning);
            Utils::printMsg(msg_present, debug);
          }
        } else {
          // This is a new client, register them using their IP, port and name
          // they sent.
          name = (std::string(buffer).substr(0, message_size));
          udpClients.push_back({*incoming_ip, incoming_port, name});

          // Send them a welcome message.
          std::string msg_present = "Hi " + name + ", welcome to the server!";
          if (udpSocket.send(msg_present.c_str(), msg_present.size(),
                             *incoming_ip,
                             incoming_port) != sf::Socket::Status::Done) {
            Utils::printMsg("Error sending data!", error);
          } else {
            Utils::printMsg("Welcoming new client:", warning);
            Utils::printMsg(msg_present, debug);
          }
        }
      }

    } else {
      Utils::printMsg("Error recieving data!", error);
    }
  }
}
