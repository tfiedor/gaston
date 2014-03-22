#ifndef __DIP__H__
#define __DIP__H__

#include <exception>
#include <iostream>

class NotImplementedException : public std::exception {
	public:
		virtual const char* what() const throw () {
			return "Functionality not implemented yet";
		}
};

enum Decision {SATISFIABLE, UNSATISFIABLE, VALID, INVALID};

#endif
