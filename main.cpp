#include <iostream>
#include <string>
#include <coroutine>

using namespace std::string_literals;

struct Chat {
    struct promise_type {
        std::string _msgOut{}, _msgIn{};

        static void unhandled_exception() noexcept {}
        Chat get_return_object() {
            return Chat{this};
        }

        static std::suspend_always initial_suspend() noexcept {
            return {};
        }
        std::suspend_always yield_value(std::string msg) noexcept {
            _msgOut = std::move(msg);
            return {};
        }

        auto await_transform(std::string) noexcept {
            struct awaiter {
                promise_type& pt;

                static constexpr bool await_ready() noexcept {
                    return false;
                }
                std::string await_resume() const noexcept {
                    return std::move(pt._msgIn);
                }

                static void await_suspend(std::coroutine_handle<>) noexcept {}
            };
            return awaiter{*this};
        }

        void return_value(std::string msg) noexcept {
            _msgOut = std::move(msg);
        }

        static std::suspend_always final_suspend() noexcept {
            return {};
        }
    };

    using Handle = std::coroutine_handle<promise_type>;
    Handle mCoroHdl{};

    explicit Chat(promise_type* p)
        : mCoroHdl{Handle::from_promise(*p)}
    {}
    explicit Chat(Chat&& rhs) noexcept
        : mCoroHdl{std::exchange(rhs.mCoroHdl, nullptr)}
    {}

    ~Chat() {
        if (mCoroHdl) {
            mCoroHdl.destroy();
        }
    }

    std::string listen() const {
        if (!mCoroHdl.done()) {
            mCoroHdl.resume();
        }
        return std::move(mCoroHdl.promise()._msgOut);
    }

    void answer(std::string msg) const {
        mCoroHdl.promise()._msgIn = msg;
        if (!mCoroHdl.done()) {
            mCoroHdl.resume();
        }
    }
};

Chat Fun() {
    co_yield "Hello!\n"s;

    std::cout << co_await std::string{};

    co_return "Here!\n"s;
}

void Use() {
    Chat chat = Fun();

    std::cout << chat.listen();

    chat.answer("Where are you?\n"s);

    std::cout << chat.listen();
}

int main() {
    Use();
    return 0;
}
