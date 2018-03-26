// DataArray.cpp

#include "math.h"
#include "DataArray.h"

DataArray::DataArray()
{
	m_array = new double[STARTING_SIZE];
	m_size = STARTING_SIZE;
	m_cpos = 0;
	m_mean = 0;
}

DataArray::~DataArray()
{
	delete[] m_array;
}

void DataArray::push(
	double input
	)
{
	if (m_cpos < this->size())
	{
		m_array[m_cpos] = input;
	}
	else 
	{
		double* new_array = new double[(int) floor(this->size() * GROWTH_FACTOR)];
		for (unsigned long int i = 0; i < m_cpos; i++)
		{
			new_array[i] = m_array[i];
		}
		delete[] m_array;
		m_array = new_array;
		m_array[m_cpos] = input;
		m_size = m_size * GROWTH_FACTOR;
	}
	m_mean = (input + m_cpos * m_mean) / (m_cpos + 1);
	m_cpos++;
}

void DataArray::clear()
{
	m_cpos = 0;
	m_mean = 0;
}

const double* DataArray::getConstArrayPtr() const
{
	return m_array;
}

double DataArray::operator[](
	unsigned long int index
	) const
{
	return m_array[index];
}

double DataArray::mean() const
{
	return m_mean;
}

double DataArray::size() const
{
	return m_cpos + 1;
}
