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


// New design (2020)

#ifndef COMMAND_BANK_H_
#define COMMAND_BANK_H_

#include <iostream>

#include "drain/util/Bank.h"
#include "drain/util/Flags.h"
#include "Command.h"
#include "CommandUtils.h"


namespace drain {

/// Container and execution tools for commands derived from Command.
/**
 *   # Storage for commands, providing list, help and cloning facilities
 *   # Utilities for creating scripts and programs
 *
 */
class CommandBank : public BankSuper<Command> { // BankSuper<BasicCommand> {

public:

	typedef data_t command_t;

	/// Set name and brief description of a program, to appear in help dumps.
	inline
	void setTitle(const std::string & title){
		this->title = title;
	};

	drain::Flagger::dict_t sections;
	// Optional section flags for typical programs.
	static const Flags::value_t GENERAL = 1;
	static const Flags::value_t INPUT = 2;
	static const Flags::value_t OUTPUT = 4;
	static const Flags::value_t IO = INPUT | OUTPUT;
	static const Flags::value_t SPECIAL = 8;

	/// Words that are moved from class name prior to composing a command name.
	static
	std::set<std::string> & trimWords();

	/// Given a command class name like MyFileReadCommand, derives a respective command line option ("myFileRead")
	static
	void deriveCmdName(std::string & name, char prefix=0);
	//const std::set<std::string> & trim = trimWords());


	template <class D, char PREFIX=0>
	D & addExternal(D & entry, char alias = 0){
		std::string key(entry.getName());
		deriveCmdName(key, PREFIX);
		return BankSuper<Command>::template addExternal<D>(entry, key, alias);
	}

	template <class D, char PREFIX=0>
	inline
	D & addExternal(D & entry, const std::string & key, char alias = 0){
		return BankSuper<Command>::template addExternal<D>(entry, key, alias);
	}

	///
	std::string defaultCmd;

	/// A mini program executed after each cmd until ']' or ')' is encountered
	Script routine;

	/// Convert program arguments a script. Like in main(), actual command arguments start from 1.
	void scriptify(int argc, const char **argv, Script & script);

	/// Splits a command line to a list of commands, that is, a script.
	void scriptify(const std::string & cmdLine, Script & script);

	/// Converts a Unix/Linux command line to pairs (cmd,params) of strings.
	/**
	 *  \return - true if command argument (argNext) was digested.
	 */
	bool scriptify(const std::string & arg, const std::string & argNext, Script & script);


	/// Converts command strings to executable command objects, appending them to a program.
	/**
	 *   Note: *appends* commands to the end of the program, use prog.clear() if needed.
	 */
	//  void append(const Script & script, Program & prog, Context & context) const ;
	void append(const Script & script, Program & prog) const ;

	void remove(Program & prog) const;

	/// Run a single command
	void run(const std::string & cmd, const std::string & params, Context & ctx);

	/// Unlike compile, "interprets" script by running it command by command. \see compile()
	//void run(ScriptTxt & script, drain::Context & context);
	void run(Script & script, ClonerBase<Context> & contextSrc);


	void help(const std::string & key, std::ostream & ostr = std::cout);

	void help(Flagger::value_t sectionFilter = 0xffffffff, std::ostream & ostr = std::cout);

	/// Checked key and respective command
	void info(const std::string & key, const command_t & cmd, std::ostream & ostr = std::cout, bool detailed=true) const ;


protected:



	/// Name of the program, to appear in help dumps etc.
	std::string title;

	/// Given an alias or a long key, possibly prefixed by hyphens, return the long key if a command exists.
	/**
	 *
	 */
	const std::string & resolveHyphens(const key_t & key) const;

};


/// Global program command registry. Optional utility.
extern
CommandBank & getCommandBank();


} /* namespace drain */

#endif
