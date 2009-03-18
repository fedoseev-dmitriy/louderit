#pragma once

template<typename Target, typename Source> 
inline Target lexical_cast(const Source &src)
{
	stringstream stream;
	stream << src;
	Target dst;
	stream >> dst;
	return dst;
}