#include "TCPMessageBridge.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <cctype>

#include <boost/asio.hpp>

#include "Exceptions.hpp"
#include "Message.hpp"

using namespace std;
using namespace std::chrono;
using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost_error = boost::system::error_code;

using namespace Exceptions;

typedef std::chrono::steady_clock Clock;

namespace {

void onReceive(tcp::socket& sock, asio::streambuf& buf,
               MessageQueue& out, atomic_bool& connected, boost_error e, size_t size)
{
	if (!e) {
		assert(size > 0);

		istream is(&buf);
		string line;
		getline(is, line);

		// Only take lines that aren't all whitespace
		if (any_of(begin(line), end(line), [](char c) { return !isspace(c); })) {
			thread_local static Json::Reader reader;
			Json::Value val;
			if (!reader.parse(line, val))
				THROW(IOException, "Could not parse JSON:" + reader.getFormatedErrorMessages());

			out.send(JSONToMessage(val));
		}

		// Re-up for next time
		async_read_until(sock, buf, "\r\n", [&](boost_error errIn, size_t sizeIn) {
			onReceive(sock, buf, out, connected, errIn, sizeIn);
		});
	}
	else {
		connected = false;
	}
}

} // end anonymous namespace

void runTCPMessageServer(MessageQueue& in, MessageQueue& out)
{
	io_service service;
	asio::streambuf buf;
	tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 2564));

	while (true) {
		tcp::socket sock(service);
		// Take a connection
		atomic_bool connected(true);
		acceptor.accept(sock);

		async_read_until(sock, buf, "\r\n", [&](boost_error e, size_t size) {
			onReceive(sock, buf, out, connected, e, size);
		});

		thread_local static Json::FastWriter writer;

		while (connected) {
			service.poll();

			auto msg = in.receiveUntil(Clock::now() + milliseconds(100));

			if (msg == nullptr)
				continue;

			if (msg->getType() == Message::Type::EXIT)
				return;

			string toSend = writer.write(msg->toJSON());

			boost_error sendError;
			write(sock, buffer(toSend + "\r\n"), transfer_all(), sendError);

			if (sendError)
				THROW(IOException, sendError.message());
		}
	}
}

void runTCPMessageClient(MessageQueue& in, MessageQueue& out, std::string server)
{
	io_service service;
	tcp::resolver resolver(service);
	tcp::resolver::query query(server, "2564");
	auto resolution = resolver.resolve(query);

	tcp::socket sock(service);
	connect(sock, resolution);

	asio::streambuf buf;

	atomic_bool connected(true);

	async_read_until(sock, buf, "\r\n", [&](boost_error e, size_t size) {
		onReceive(sock, buf, out, connected, e, size);
	});

	thread_local static Json::FastWriter writer;

	while (connected) {
		service.poll();

		auto msg = in.receiveUntil(Clock::now() + milliseconds(100));

		if (msg == nullptr)
			continue;

		if (msg->getType() == Message::Type::EXIT)
			return;

		string toSend = writer.write(msg->toJSON());

		boost_error sendError;
		write(sock, buffer(toSend + "\r\n"), transfer_all(), sendError);

		if (sendError)
			THROW(IOException, sendError.message());
	}
}
