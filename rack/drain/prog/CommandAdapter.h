/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/

#ifndef DRAINLET_WRAPPER_H_
#define DRAINLET_WRAPPER_H_

#include <map>
#include <set>
//#include <drain/util/Debug.h>
//#include <drain/util/ReferenceMap.h>

#include "util/Debug.h"
#include "util/ReferenceMap.h"

#include "Command.h"
#include "CommandRegistry.h"

using namespace drain;

namespace drain {

/*
 *  \tparam BeanLike
 */
template <class B>
class BeanRefEntry : public Command {
    public: //re 

	B & bean;

	BeanRefEntry(B & bean) : bean(bean) { // : drainLet(defaultDrainLet) {
		//this->parameters.append(bean.getParameters());
		getRegistry().add(*this, bean.getName(), 0);
	};

	BeanRefEntry(B & bean, const std::string & cmdName, char alias = 0) : bean(bean) { // : drainLet(defaultDrainLet) {
		//this->parameters.append(bean.getParameters());
		getRegistry().add(*this, cmdName, alias);
	};


	BeanRefEntry(B & bean, const std::string & section, const std::string & cmdName, char alias = 0) : bean(bean)  { // : drainLet(defaultDrainLet) {
		//this->parameters.append(bean.getParameters());
		getRegistry().add(section, *this, cmdName, alias);
		//add(section, drainLet, name, alias);
	};


	virtual inline
	const std::string & getName() const { return bean.getName(); };

	virtual inline
	const std::string & getDescription() const { return bean.getDescription(); };

	virtual inline
	const ReferenceMap & getParameters() const { return bean.getParameters(); };

	/// Bean may be peaceful.
	virtual
	void run(const std::string & params){
		bean.setParameters(params);
	};

};



/// A command that is automatically added to CommandRegistry upon initialization.
/**
 *
 *  \tparam T - base class, usually Command
 */
template <class T>
class CommandEntry : public T {
    public: //re 

	CommandEntry(char alias = 0) {
		getRegistry().add(*this, T::getName(), alias);
	};

	CommandEntry(const std::string & name, char alias = 0) {
		getRegistry().add(*this, name, alias);
	};


	CommandEntry(const std::string & section, const std::string & name, char alias = 0) {
		getRegistry().add(section, *this, name, alias);
	};


	CommandEntry(const std::string & name, char alias, const T & init) : T(init)  { // use copy constr.
		getRegistry().add(*this, name, alias);
	};

	~CommandEntry(){};


};


} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
