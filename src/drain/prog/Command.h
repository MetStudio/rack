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

#ifndef DRAINLET_H_
#define DRAINLET_H_

//#include <map>
//#include <set>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include <drain/Log.h>
#include "drain/util/BeanLike.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/VariableMap.h"

//#include "drain/util/Cloner.h"
#include "Context.h"

namespace drain {

/// Base class for commands: typically actions taking parameters but also plain variable assignments and parameterless actions.
/**
 *
 */
class Command : public Contextual {

public:

	inline
	Command(): section(0){}; //

	inline
	Command(const Command & cmd) : section(cmd.section), lastParameters(cmd.lastParameters){
		// setParameters(cmd.getParameters()); they do not exist yet!
	}

	virtual inline
	~Command(){};

	// TODO: getFullName(), the "C++ name" of this (derived) class.
	// Typically not same as the command line command.

	virtual
	const std::string & getName() const = 0;

	virtual
	const std::string & getDescription() const = 0;

	virtual
	const ReferenceMap & getParameters() const = 0;

protected:

	virtual
	ReferenceMap & getParameters() = 0;

public:

	virtual
	void setParameters(const std::string & args) final;
	/*
	{
		lastParameters = args;
		setAllParameters(args);
	};
	*/


	// TODO: remove
	// virtual
	// void setParameters(const VariableMap & args) final; // REDESIGN?

	// virtual
	// void setParameters(const VariableMap & args) = 0; // REDESIGN?


	// TODO: make this a loop calling set(key,val)
	template <class T>
	void setParameters(const SmartMap<T> & args){
		// TODO: set parameter X=Y ?
		/*
		VariableMap vargs;
		vargs.importCastableMap(args);
		setParameters(vargs);
		*/
		ReferenceMap & parameters = getParameters();
		std::stringstream sstr;
		char separator = 0;
		for (const auto & entry: args){
			parameters[entry.first] = entry.second;
			// setParameter(entry.first, entry.second);
			if (separator)
				sstr << separator;
			else
				separator = ',';
			sstr << entry.first << '=' << entry.second;
		}
		lastParameters = sstr.str();

	}

	/// Set a value to a single parameter.
	/**
	 *
	 *  This method also assigns "<key>=<value>" to lastParameters
	 */
	template <class T>
	void setParameter(const std::string & key, const T & value){
		getParameters()[key] = value;
		std::stringstream sstr;
		sstr << key << '=' << value;
		lastParameters = sstr.str();
	}


	inline
	Command & addSection(drain::Flagger::value_t i){
		section |= i;
		return *this;
	}

	/// Optional method for preparing command to execution.
	/**
	 *   This function will be called prior to running exec()
	 */
	virtual
	void update(){
	}



	//virtual
	//void setParameters(const SmartMapBase & params) = 0;

	// Could also need/replace   void setParameters(const std::map<std::string,T> & p)

	// Rename: hasParameters
	inline
	bool hasParameters() const { // tODO rename

		Logger mout(__FILE__, __FUNCTION__);

		const ReferenceMap & params = this->getParameters();
		//mout.warn(params );

		const ReferenceMap::const_iterator it = params.begin();
		if (it == params.end()) // empty
			return false;
		else
			return it->second.getType() != typeid(void);  // ???
	};

	/// Description of result, comparable to a return type of a function. ?
	virtual inline
	const std::string & getType() const {
		static const std::string empty;
		return empty;
	};

	/// Run the command with current parameter values.
	virtual	inline
	void exec() const {};

	/// Convenience. Sets parameters and executes the command.
	/**
	 *  \see exec() .
	 */
	virtual
	inline
	void run(const std::string & params = ""){
		setParameters(params);
		update();
		exec();
	}


	// Optional bit(s) marking the command type (compare with manual page sections)
	/**
	 *  Typically, zero section is for "hidden" commands not appearing in help dumps.
	 */
	int section = 1;

	/// After executing this command run a routine, if defined.
	// bool execRoutine; -> see section flag TriggerSection;

	/// Future option: single-code Dynamic functions: handle the command string
	// virtual void setKey(const std::string & key) const {}

	inline
	const std::string & getLastParameters() const {
		return lastParameters;
	};

protected:

	// virtual
	// void setAllParameters(const std::string & args); // = 0;

	std::string lastParameters;


};

inline
std::ostream & operator<<(std::ostream & ostr, const Command &cmd){
	ostr << cmd.getName();
	if (cmd.hasParameters()){
		ostr << ' ' << cmd.getParameters(); //  << ' ';
	}
	return ostr;
}

// See new implementations in CommandBank, CommandUtils.
//typedef std::list<std::pair<Command &, std::string> > Script; // TODO: move


/// Retrieves bean dynamically for each call.
/*
template <class B>
class BeanerCommand : public Command {

public:

	typedef B bean_t;

	// Main
	virtual
	const bean_t & getBean() const = 0;

	// Main
	virtual
	bean_t & getBean() = 0;




	inline
	const std::string & getName() const final {
		return getBean().getName();
	};

	inline
	const std::string & getDescription() const final {
		return getBean().getDescription();
	};

	virtual	inline
	const drain::ReferenceMap & getParameters() const final {
		return getBean().getParameters();
	};

protected:

	virtual	inline
	drain::ReferenceMap & getParameters() final {
		return getBean().getParameters();
	};

};
*/

/// Base for derived classes using member BeanLike or referenced BeanLikes.
/**
 *  Beans implement getName(), getDescription(), getParameters()
 *
 *   \tparam B  - bean class
 *   \tparam BS - same as B, or reference of B
 */
template <class B, class B2=B>
class BeanCommand : public Command {
// class BeanCommand : public BeanerCommand<B> { // public Command {

public:

	BeanCommand(){
	};

	BeanCommand(const std::string & name, const std::string & description) : bean(name, description){
	};


	BeanCommand(const BeanCommand & cmd) :  Command(cmd), bean(cmd.bean) {
		// WARNING: this may have undesired beaviour:
		// If B2 is (Bean &), also new instance of beanCmd.bean -> cmd.bean
	};

	BeanCommand(B & b) : bean(b) {
	};

	typedef B bean_t;

// Consider private:
	B2 bean;


	virtual inline
	const bean_t & getBean() const  { // final
		return bean;
	};

	// Main
	virtual inline
	bean_t & getBean() { // final
		return bean;
	};



	inline
	const std::string & getName() const final {
		return getBean().getName();
	};

	inline
	const std::string & getDescription() const final {
		return getBean().getDescription();
	};

	virtual	inline
	const drain::ReferenceMap & getParameters() const final {
		return getBean().getParameters();
	};

protected:

	virtual	inline
	drain::ReferenceMap & getParameters() final {
		return getBean().getParameters();
	};


	/*
	virtual
	void setAllParameters(const std::string & args) override {

		Context & ctx = this->template getContext<>();
		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName() );

		bean.setParameters(args,'=');
		//mout.note() = "updating";
		//this->update();
	}
	*/

	///
	//virtual
	/*
	void setParameters(const VariableMap & params){
		bean.setParameters(params);
		// this->update();
	}
	*/

};



template <class B>
class BeanRefCommand : public drain::BeanCommand<B, B&> {

public:

	BeanRefCommand(B & bean) : drain::BeanCommand<B,B&>(bean){
	}

	BeanRefCommand(const B & bean) : drain::BeanCommand<B,B&>(bean){
	}

	BeanRefCommand(const BeanRefCommand<B> & beanRefCmd) : drain::BeanCommand<B,B&>(beanRefCmd.bean){
	}


};




/// Simple implementation of Command: adds \c name , \c description and \c parameters .
/** Wrapper for simple template classes; no member functions are required.
 *  BasicDrainLet implements getDescription(), getParameters(), setParameters().
 *  The default implementation of run() calls setParameters() and exec().
 *  The default implementation exec() does nothing, it should be redefined in derived classes.
 */
//class BasicCommand : public Command {  // Todo consider BeanLike
class BasicCommand : public BeanCommand<BeanLike> {  // Todo consider BeanLike

public:

	BasicCommand(const std::string & name, const std::string & description);

	inline
	//BasicCommand(const BasicCommand & cmd): Command(cmd), name(cmd.name), description(cmd.description) {
	BasicCommand(const BasicCommand & cmd): BeanCommand<BeanLike>(cmd) {
		// remember to call importParameters()
	}

/* 2024
	virtual	inline
	const std::string & getName() const { return name; };

	virtual	inline
	const std::string & getDescription() const { return description; };

	virtual	inline
	const ReferenceMap & getParameters() const { return parameters; };

protected:

	virtual inline
	ReferenceMap & getParameters(){
		return parameters;
	};

public:

*/
	/*
	virtual
	void setAllParameters(const std::string & args) override; //, char assignmentSymbol='=');
	*/

	/*
	inline
	void setParameters(const VariableMap & params){
		parameters.importCastableMap(params);
		lastParameters = sprinter(parameters, Sprinter::cmdLineLayout);
		//this->update();
	}
	*/

	/*  2024
	template <class T>
	inline
	void setParameters(const SmartMap<T> & params){
		parameters.importCastableMap(params);
	}
	*/

	/*
	template <class T>
	void setParameter(const std::string & key, const T & value) {
		parameters[key] = value;
		//this->update();
	}
	*/


/* 2024
protected:

	const std::string name;

	const std::string description;

	ReferenceMap parameters;

	*/

};




/// A single-parameter command.
/**
 *  \tparam T - storage type of the command parameter
 */
template <class T = std::string>
class SimpleCommand : public BasicCommand {

public:

	T value;

	SimpleCommand(const std::string & name, const std::string & description,
			const std::string & key="value", const T & initValue = T(), const std::string & unit = "") : BasicCommand(name, description) {

		///// parameters.separator = '\0';
		/// 2024
		getParameters().separator = '\0';
		getParameters().link(key, value = initValue, unit);
	};

	/// Constuctor designer for SimpleCommand<Unituple<> > .
	template <class S>
	SimpleCommand(const std::string & name, const std::string & description,
			const std::string & key, std::initializer_list<S> l , const std::string & unit = "") : BasicCommand(name, description) {
		// parameters.separator = '\0';
		value = l;
		getParameters().link(key, value);
		// parameters.link(key, value);
	};


	SimpleCommand(const SimpleCommand & cmd):  BasicCommand(cmd) {
		getParameters().separator = '\0';
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
		/*
		parameters.separator = '\0';
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
		*/
	};

	/*
	void setParameters(const std::string & args){
		const std::string & key = parameters.begin()->first;
		Variable v(typeid(T));
		if (key.empty()){
			v = args;
		}
		else {
			const std::string assignment = key+"=";
			const size_t n = assignment.length();
			if (args.compare(0,n,key)==0){
				v = args.substr(n);
			}
			else
				v = args;
		}
		value = v;
	}
	*/



};

/// Command stored as a single entry in CommandBank and becomes "renamed" upon setKey() prior to execution.
/**
 *
 */
/*
class DynamicCommand : public drain::Command {

public:

	virtual
	void exec() const {
		Context & ctx = getContext<Context>();
		drain::Logger mout(ctx.log,__FILE__, __FUNCTION__);
		mout.note("I am " , getName() , ", invoked as " , key );
	}

	virtual
	void setKey(const std::string & cmdKey) const {
		key = cmdKey;
	}

protected:

	mutable
	std::string key;

};
*/




} // drain::

#endif /* DRAINLET_H_ */
