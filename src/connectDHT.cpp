#include <opendht.h>
#include <vector>

#include "opendht_c.h"

int main()
{
    dht::DhtRunner node;

    // Launch a dht node on a new thread, using a
    // generated RSA key pair, and listen on port 4222.
    node.run(4222, dht::crypto::generateIdentity(), true);

    // Join the network through any running node,
    // here using a known bootstrap node.
    node.bootstrap("10.0.2.4", "53826");

    // put some data on the dht
    std::vector<uint8_t> some_data(5, 10);
    // some_data.push_back(5);
    node.put("K7", some_data);

    // put some data on the dht, signed with our generated private key
    // node.putSigned("K6", "V6");

    // // put some data on the dht
    // std::vector<uint8_t> data(10, 20);
    // node.put("other_unique_key", data);

    // // put some data on the dht, signed with our generated private key
    // node.putSigned("other_unique_key_42", data);

    // std::vector<uint_8> MK

    // get data from the dht
    node.get("K7", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values)
        {
            std::cout << "Found value: " << *value << std::endl;
            // std::cout << dht::crypto::PublicKey({'K', '5'}).encrypt({5,10}) << std::endl;
            // std::cout << "Decrypted: " << decrypt(*value) << std::endl;
        }
        return true; // return false to stop the search
    });


    std::cout << "P Data" << std::endl;
    sleep(120);
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