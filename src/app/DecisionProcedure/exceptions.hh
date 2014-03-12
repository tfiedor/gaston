#ifndef __EXCEPTIONS__H__
#define __EXCEPTIONS__H__

#include <exception>
#include <iostream>

class NotImplementedException : public std::exception {
	public:
		virtual const char* what() const throw () {
			return "Functionality not implemented yet";
		}
};

#endif
