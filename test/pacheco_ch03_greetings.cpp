// Greetings

#include<mpi3/main.hpp>
#include<mpi3/process.hpp>

namespace mpi3 = boost::mpi3;

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) {  // NOLINT(bugprone-exception-escape)
    auto const my_rank = world.rank();
    auto const p = world.size();

    assert( p > 1 );

    {
        char message[100] = {};
        if (my_rank != 0) {
            std::sprintf(message, "Greetings from process %d!", my_rank);
            world.send_n(std::data(message), std::size(message), 0);
        } else {
            for(int source = 1; source != p; ++source) {
                char message_check[100] = {};
                std::sprintf(message_check, "Greetings from process %d!", source);

                world.receive_n(std::data(message), std::size(message), source);
                std::cout << message << " === " << message_check << std::endl;
                assert( std::strcmp(message_check, message) == 0 );
            }
        }
    }

    {
        std::array<char, 100> message;
        if (my_rank != 0) {
            std::sprintf(message.data(), "Greetings from process %d!", my_rank);
            world.send_n(message.data(), message.size(), 0);
        } else {
            for(int source = 1; source != p; ++source) {
                world.receive_n(message.data(), message.size(), source);

                std::array<char, 100> message_check;
                std::sprintf(message_check.data(), "Greetings from process %d!", source);  // NOLINT(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
                assert( std::strcmp(message_check.data(), message.data()) == 0 );
            }
        }
    }
    {
        if (my_rank != 0) {
            std::string message = "Greetings from process "+std::to_string(my_rank)+"!";
            world.send_n(&message, 1, 0);
        } else {
            for(int source = 1; source != p; ++source) {  // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
                std::string message;
                world.receive_n(&message, 1, source);

                assert(message == "Greetings from process "+std::to_string(source)+"!");
            }
        }
    }
    {
        if (my_rank != 0) {
            std::string const message = "Greetings from process "+std::to_string(my_rank)+"!";
            world[0] << message;
        } else {
            for(int source = 1; source != p; ++source) {  // NOLINT(altera-unroll-loops,altera-id-dependent-backward-branch)
                std::string message;
                world[source] >> message;

                assert(message == "Greetings from process "+std::to_string(source)+"!");
            }
        }
    }

    return 0;
}
