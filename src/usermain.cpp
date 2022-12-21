#include "../h/syscall_cpp.hpp"
#include "../h/printing.hpp"

bool finished = false;

class UserThread : public Thread {

public:
	void run() override {
		int cnt = 0;
		while (cnt != 100) {
            printString("ok\n");
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
