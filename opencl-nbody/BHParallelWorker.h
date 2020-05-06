#include "Simulator.h"
#include "Poco/Runnable.h"
#include "Poco/ThreadPool.h"

class BHParallelWorker : public Poco::Runnable {
public:
	void setSim(BHParallel* sim) {
		this->sim = sim;
	}

	void run()
	{
		sim->setupChildren(index);
	}

	void setParameters(int index) {
		this->index = index;
	}

private:
	BHParallel* sim;
	int index;
};
