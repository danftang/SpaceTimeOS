#ifndef ONESHOTPERMISSION_H
#define ONESHOTPERMISSION_H

#include <functional>

// Wraps a std::function<void()> so that it can only be executed once.
class OneShotExecutable {
protected:
    std::function<void()>   function;
public:
    OneShotExecutable(const OneShotExecutable &) = delete;
    OneShotExecutable(OneShotExecutable &&other) = default;

    template<class T>
    OneShotExecutable(T &&function) : function(std::forward<T>(function)) {};

    inline void operator()() {
        function();
        invalidate();
    }

    inline bool isValid() { return static_cast<bool>(function); }

    inline void invalidate() { function = nullptr; }
};

#endif
