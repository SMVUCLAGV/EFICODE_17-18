#ifndef DATAARRAY_H
#define DATAARRAY_H

#define GROWTH_FACTOR 1.3
#define STARTING_SIZE 100

class DataArray
{
	public:
	
	DataArray();
	~DataArray();
	
	void push(
		double input
		);
	void clear();
	const double* getConstArrayPtr() const;
	double operator[](
	    unsigned long int index
	    ) const;
	double mean() const;
	double size() const;
	
	private:
	
	unsigned long int m_size;
	unsigned long int m_cpos;
	double* m_array;
	double m_mean;
};

#endif
