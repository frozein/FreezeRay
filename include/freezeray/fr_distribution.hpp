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
class DistributionDiscrete
{
public:
	DistributionDiscrete(const std::vector<std::pair<T, float>>& pmf);

	const T& sample(float u, float& pdf) const;
	float pdf(uint32_t idx) const;

	float get_total_density() const { return m_totalDensity; }

private:
	float m_totalDensity;
	std::vector<std::pair<T, float>> m_pmf;

	std::vector<float> m_acceptanceTable;
	std::vector<uint32_t> m_aliasTable;
};

//-------------------------------------------//

template<typename T>
DistributionDiscrete<T>::DistributionDiscrete(const std::vector<std::pair<T, float>>& pmf) :
	m_pmf(pmf), m_totalDensity(0.0)
{
	//validate:
	//---------------
	if(pmf.size() == 0)
		throw std::invalid_argument("PMF must contain at least 1 element");

	//compute total density:
	//---------------
	m_totalDensity = 0.0;
	for(uint32_t i = 0; i < pmf.size(); i++)
	{
		if(m_pmf[i].second < 0.0)
			throw std::invalid_argument("Individual probabilities must be positive");

		m_totalDensity += m_pmf[i].second;
	}

	if(m_totalDensity == 0.0)
		throw std::invalid_argument("Total density must be positive");

	//initialize domain, create scaled pmf, create small/large worklists:
	//---------------
	m_acceptanceTable.resize(pmf.size());
	m_aliasTable.resize(pmf.size());

	std::vector<float> scaledDensity(pmf.size());

	std::vector<uint32_t> small;
	std::vector<uint32_t> large;

	for(uint32_t i = 0; i < pmf.size(); i++) 
	{
		float density = m_pmf[i].second / m_totalDensity;
		scaledDensity[i] = density * m_pmf.size();

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
const T& DistributionDiscrete<T>::sample(float u, float& pdf) const
{
	float select = u * m_pmf.size();
	uint32_t selectIdx = std::min((uint32_t)select, (uint32_t)m_pmf.size() - 1);

	float acceptanceProb = select - selectIdx;
	if(acceptanceProb > m_acceptanceTable[selectIdx])
		selectIdx = m_aliasTable[selectIdx];

	pdf = m_pmf[selectIdx].second / m_totalDensity;
	return m_pmf[selectIdx].first;
}

template<typename T>
float DistributionDiscrete<T>::pdf(uint32_t idx) const
{
	return m_pmf[idx].second / m_totalDensity;
}

}; //namespace fr

#endif //#ifndef FR_DISTRIBUTION_H