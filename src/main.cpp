#include <iostream>
#include "spacetime/IntertialFrame.h"
#include "spacetime/Minkowski.h"
#include "spacetime/ReferenceFrame.h"
#include "SpaceTimePtr.h"
#include "Laboratory.h"

class MyObj {
public:
    int i;
};


int main() {

    spacetime::IntertialFrame<spacetime::Minkowski<2>> laboratoryFrame;

    Laboratory laboratory(laboratoryFrame); // laboratory from which we spawn objects.

    SpaceTimePtr ptr = laboratory.spawn(MyObj());

    ptr->i = 1234;

    auto channel = ptr.newChannel();

    channel.open(ptr);

    // for (int i = 0; i < 10; ++i) {
    //     executor.submit([i]() {
    //         std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << std::endl;
    //     });
    // }

    // std::future<int> r1 = boost::asio::post(pool, asio::use_future([]() { return 2; }));
    // std::cout << "Result = " << r1.get() << '\n';

    // // Optional: Wait for all tasks to complete
    // pool.join();

    std::cout << "Goodbye world" << std::endl;
    return 0;
}