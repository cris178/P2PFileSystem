#include <opendht.h>
#include <vector>

int main()
{
    dht::DhtRunner node;

    // Launch a dht node on a new thread, using a
    // generated RSA key pair, and listen on port 4222.
    // node.run(4223, dht::crypto::generateIdentity(), true);

    // Join the network through any running node,
    // here using a known bootstrap node.
    node.bootstrap("10.0.2.15", "4224");

    // put some data on the dht
    std::vector<uint8_t> some_data(5, 10);
    node.put("unique_key", some_data);

    // put some data on the dht, signed with our generated private key
    node.putSigned("unique_key_42", some_data);

    // // put some data on the dht
    // std::vector<uint8_t> data(10, 20);
    // node.put("other_unique_key", data);

    // // put some data on the dht, signed with our generated private key
    // node.putSigned("other_unique_key_42", data);

    // get data from the dht
    node.get("other_unique_key", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values)
            std::cout << "Found value: " << *value << std::endl;
        return true; // return false to stop the search
    });



    sleep(15);
    // int i = 0;
    // while(true)
    // {
    //     ++i;
    // }

    std::cout << "-----------------" << std::endl;


    // wait for dht threads to end
    node.join();
    return 0;
}