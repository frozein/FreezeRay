/* fr_thread_pool.hpp
 *
 * contains a very basic, incomplete thread pool implementation
 * for use in renderers
 */

#ifndef FR_THREAD_POOL_H
#define FR_THREAD_POOL_H

#include <stdint.h>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

//-------------------------------------------//

namespace fr
{

template <typename WorkGroup>
class ThreadPool
{
public:
	ThreadPool(uint64_t numWorkers, const std::queue<WorkGroup>& workGroups, std::function<void(const WorkGroup&)> process) :
		m_workGroups(workGroups), m_numWorkGroupsInitial((uint32_t)workGroups.size()), m_process(process)
	{
		m_activeThreads = numWorkers;

		for(uint64_t i = 0; i < numWorkers; i++)
		{
			m_workers.emplace_back(
				[this, i] {
					while(true)
					{
						WorkGroup group;

						{
							std::unique_lock<std::mutex> lock(m_queueMutex);
							
							if(m_workGroups.empty())
							{
								m_activeThreads.fetch_sub(1);
								return;
							}

							group = m_workGroups.front();
							m_workGroups.pop();
						}

						m_process(group);
					}
				}
			);
		}
	}

	bool complete()
	{
		return m_activeThreads.load() == 0;
	}

	float progress()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);

		uint64_t numWorking = m_workGroups.size() + m_activeThreads.load();
		return 1.0f - (float)numWorking / (float)m_numWorkGroupsInitial;			
	}

	~ThreadPool()
	{
		for(uint64_t i = 0; i < m_workers.size(); i++)
			m_workers[i].join();
	}

private:
	std::vector<std::thread> m_workers;
	std::atomic<uint64_t> m_activeThreads = 0;

	uint32_t m_numWorkGroupsInitial;
	std::queue<WorkGroup> m_workGroups;
	std::mutex m_queueMutex;
	std::function<void(const WorkGroup&)> m_process;
};

}; //namespace fr

#endif //#ifndef FR_THREAD_POOL_H