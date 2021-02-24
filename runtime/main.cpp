#include <csignal>
#include "runtime_impl.hpp"


constexpr std::chrono::seconds ILLIXR_RUN_DURATION_DEFAULT {60};

ILLIXR::runtime* r;


static void sigint_handler(int sig) {
    assert(sig == SIGINT && "sigint_handler is for SIGINT");
	if (r) {
		r->stop();
	}
}

#ifndef NDEBUG
static void sigill_handler(int sig) {
    /// Forward SIGILL from illegal instructions to catchsegv in ci.yaml
    /// Provides additional debugging information via -rdynamic
    assert(sig == SIGILL && "sigill_handler is for SIGILL");
    std::raise(SIGSEGV);
}
#endif /// NDEBUG


class cancellable_sleep {
public:
	template <typename T, typename R>
	bool sleep(std::chrono::duration<T, R> duration) {
		auto wake_up_time = std::chrono::system_clock::now() + duration;
		while (!_m_terminate.load() && std::chrono::system_clock::now() < wake_up_time) {
			std::this_thread::sleep_for(std::chrono::milliseconds{100});
		}
		return _m_terminate.load();
	}
	void cancel() {
		_m_terminate.store(true);
	}
private:
	std::atomic<bool> _m_terminate {false};
};


int main(int argc, char* const* argv) {
	r = ILLIXR::runtime_factory(nullptr);

#ifndef NDEBUG
    /// When debugging, register the SIGILL handler for capturing more info
    std::signal(SIGILL, sigill_handler);
#endif /// NDEBUG

	/// Shutting down method 1: Ctrl+C
    std::signal(SIGINT, sigint_handler);

	/// Shutting down method 2: Run timer
	std::chrono::seconds run_duration = 
		getenv("ILLIXR_RUN_DURATION")
		? std::chrono::seconds{std::stol(std::string{getenv("ILLIXR_RUN_DURATION")})}
		: ILLIXR_RUN_DURATION_DEFAULT;

	assert(errno == 0 && "Errno should not be set after creating runtime");

	std::vector<std::string> lib_paths;
	std::transform(argv + 1, argv + argc, std::back_inserter(lib_paths), [](const char* arg) {
		return std::string{arg};
	});
	assert(errno == 0 && "Errno should not be set before loading dynamic libraries");
	r->load_so(lib_paths);

	cancellable_sleep cs;
	std::thread th{[&]{
		cs.sleep(run_duration);
		r->stop();
	}};

	r->wait(); // blocks until shutdown is r->stop()

	// cancel our sleep, so we can join the other thread
	cs.cancel();
	th.join();

	delete r;
	return 0;
}
