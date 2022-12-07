#include "../h/syscall_cpp.hpp"

bool finished = false;

class UserThread : public Thread {

public:
	void run() override {
		int cnt = 0;
		while (cnt != 100) {
			putc('o');
			putc('k');
			putc('\n');
			cnt++;
		}
		finished = true;
		
	}
};

void userMain() {

	Thread* t1 = new UserThread();
	t1->start();
	
	while (!finished) Thread::dispatch();
}
