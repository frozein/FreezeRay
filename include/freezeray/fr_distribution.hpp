/* fr_distribution.hpp
 *
 * contains a definition for a distribution class
 */

#ifndef FR_DISTRIBUTION_H
#define FR_DISTRIBUTION_H

#include <vector>
#include <utility>

//-------------------------------------------//

namespace fr
{

template<typename T>
class Distribution
{
public:
	Distribution(const std::vector<std::pair<T, float>>& pmf, float totalDensity = 1.0f);

	const T& sample(float u) const;

private:
	std::vector<float> m_acceptanceTable;
	std::vector<uint32_t> m_aliasTable;
	std::vector<T> m_domain;
};

//-------------------------------------------//

template<typename T>
Distribution<T>::Distribution(const std::vector<std::pair<T, float>>& pmf, float totalDensity)
{
	//validate:
	//---------------
	if(pmf.size() == 0)
		throw std::invalid_argument("PMF must contain at least 1 element");

	if(totalDensity <= 0.0f)
		throw std::invalid_argument("total density must be postive");

	//initialize domain, create scaled pmf, create small/large worklists:
	//---------------
	m_domain.resize(pmf.size());

	std::vector<float> scaledDensity(pmf.size());

	std::vector<uint32_t> small;
	std::vector<uint32_t> large;

	for(uint32_t i = 0; i < pmf.size(); i++) 
	{
		m_domain[i] = pmf[i].first;

		float density = pmf[i].second / totalDensity;
		scaledDensity[i] = density * pmf.size();

		if(scaledDensity[i] < 1.0f)
			small.push_back(i);
		else if(scaledDensity[i] > 1.0f)
			large.push_back(i);
		else
		{
			m_acceptanceTable[i] = 1.0f;
			m_aliasTable[i] = i;
		}
	}

	//process small/large worklists:
	//---------------
	m_acceptanceTable.resize(pmf.size());
	m_aliasTable.resize(pmf.size());

	while(!small.empty() && !large.empty()) 
	{
		uint32_t s = small.back(); 
		uint32_t l = large.back(); 

		small.pop_back();
		large.pop_back();

		m_acceptanceTable[s] = scaledDensity[s];
		m_aliasTable[s] = l;

		scaledDensity[l] = (scaledDensity[l] + scaledDensity[s]) - 1.0f;

		if(scaledDensity[l] < 1.0f)
			small.push_back(l);
		else if(scaledDensity[l] > 1.0f)
			large.push_back(l);
		else
		{
			m_acceptanceTable[l] = 1.0f;
			m_aliasTable[l] = l;
		}
	}

	for(uint32_t l : large)
		m_acceptanceTable[l] = 1.0f;

	for(uint32_t s : small)
		m_acceptanceTable[s] = 1.0f;
}

template<typename T>
const T& Distribution<T>::sample(float u) const
{
	float select = u * m_domain.size();
	uint32_t selectIdx = std::min((uint32_t)select, (uint32_t)m_domain.size() - 1);

	float acceptanceProb = select - selectIdx;
	if(acceptanceProb > m_acceptanceTable[selectIdx])
		selectIdx = m_aliasTable[selectIdx];

	return m_domain[selectIdx];
}

}; //namespace fr

#endif //#ifndef FR_DISTRIBUTION_H