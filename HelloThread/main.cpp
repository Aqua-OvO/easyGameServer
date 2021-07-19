#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "CELLTimestamp.hpp"
using namespace std;

mutex m;
const int tCount = 4;
atomic_int sum = 0;
void workFun(int index)
{
	for (int n = 0; n < 2000000; n++)
	{
		//自解锁
		//lock_guard<mutex> lg(m);
		//临界区 开始
		//m.lock();
		sum++;
		//m.unlock();
		//临界区 结束
	}
}

int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
		
	}
	CELLTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		t[n].join();
	}
	//t.detach();
	//t.join();
	cout << "tTime=" << tTime.getElapsedTimeInMicroSec() << ",sum=" << sum << endl;
	tTime.update();
	sum = 0;
	for (int i = 0; i < 8000000; i++)
	{
		sum++;
	}
	cout << "tTime=" << tTime.getElapsedTimeInMicroSec() << ",sum=" << sum << endl;
	cout << "hello, main thread." << endl;
	return 0;
}