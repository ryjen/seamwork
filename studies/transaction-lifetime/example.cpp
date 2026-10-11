#include <iostream>
#include <utility>

#include <seamwork/studies/transaction_lifetime.hpp>

namespace sw = seamwork::studies::transaction_lifetime;

int main()
{
    sw::fake_backend backend;

    auto started = sw::transaction_guard::start(backend);
    if (!started) {
        std::cerr << "begin failed\n";
        return 2;
    }

    auto tx = std::move(*started);

    // Application work would happen here.

    const auto committed = tx.commit();
    if (!committed) {
        std::cerr << "commit failed; active guard will attempt rollback on scope exit\n";
        return 3;
    }

    std::cout << "commit completed explicitly\n";
}
