/**


    Copyright 2015 Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef COMMANDPACK_H_
#define COMMANDPACK_H_

#include "CommandRegistry.h"
#include "CommandAdapter.h"



namespace drain {




class CmdHelp : public SimpleCommand<std::string> { // public BasicCommand {//

public:

	inline
	CmdHelp(const std::string & title, const std::string & usage = "") : SimpleCommand<std::string>(__FUNCTION__, "Dump help and exit.", "keyword"), title(title), usage(usage) { //BasicCommand("Dump help and exit") {
	};

	void exec() const;


	const std::string title;

	const std::string usage;

protected:

	void helpOnModules(std::ostream & ostr, const std::string & excludeKey = "*") const ;

};

/// A base class
/**
 *  Typically, this wrapper is used for displaying version toOStr etc. through defining
 *
 *  Now member  std::string value is applied as toOStr container.
 */
class CommandInfo : public BasicCommand { //public SimpleCommand<std::string> {
    public: //re 

	const std::string info;

	//CommandInfo(const std::string & description, const std::string & toOStr) : SimpleCommand<std::string>(description) { value.assign(toOStr); };
	CommandInfo(const std::string & description, const std::string & info) : BasicCommand(__FUNCTION__, description), info(info) {
		//value.assign(toOStr);
	};


	virtual
	inline
	void run(const std::string & params = ""){
		std::cout << info << std::endl;
	};

};


class ScriptParser : public SimpleCommand<std::string> {
    public: //re 

	Script script;

	int autoExec;

	inline
	ScriptParser() : SimpleCommand<std::string>(__FUNCTION__, "Define a script.", "script"), autoExec(-1) {
	};

	virtual
	inline
	void run(const std::string & s){
		//std::cerr << name << ", value=" << value << std::endl;
		//std::cerr << name << ", calling with param=" << s << std::endl;
		if (autoExec < 0)
			autoExec = 1;
		getRegistry().scriptify(s, script);
	};

};



/// Command loader that executes the commands immediately - it does not store the commands into a script.
class CommandLoader : public SimpleCommand<std::string> {
    public: //re 


	CommandLoader() : SimpleCommand<std::string>(__FUNCTION__, "Execute commands from a file.", "filename") {}; // const std::string & name, char alias = 0

	//CommandCommandLoader(std::set<std::string> & commandSet, const std::string & name, char alias = 0);

	virtual
	void run(const std::string & params);

};



//CommandScriptExec::CommandScriptExec(CommandScript & script) : BasicCommand(name, alias, "Execute a script."), script(script) {}
class ScriptExec : public BasicCommand {
    public: //re 

	inline
	ScriptExec(Script & script) : BasicCommand(__FUNCTION__, "Execute a script."), script(script) {};


	virtual
	inline
	void exec() const {
		getRegistry().run(script);
	};

	Script & script;

};

class CmdFormat : public SimpleCommand<std::string> {
    public: //re 

	CmdFormat() :  SimpleCommand<std::string>(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)", "format","") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	};

};
extern CommandEntry<CmdFormat> cmdFormat;

/// Handler for requests without a handler - tries to read a file.
/**  Currently unused
*/
class DefaultHandler : public BasicCommand {
    public: //re 

	//std::string value;
	DefaultHandler() : BasicCommand(__FUNCTION__, "Handles arguments for which no specific Command was found."){};

	virtual  //?
	inline
	void run(const std::string & params){

		Logger mout(getName());

		mout.debug(1) << "params: " << params << mout.endl;

		/// Syntax for recognising text files.
		//static const drain::RegExp odimSyntax("^--?(/.+)$");

		if (params.empty()){
			mout.error() << "Empty parameters" << mout.endl;
		}
		else {
			try {
				mout.debug(1) << "Assuming filename, trying to read." << mout.endl;
				getRegistry().run("inputFile", params);  // Note: assumes only that a command 'inputFile' has been defined.
			} catch (std::exception & e) {
				mout.error() << "could not handle params='" << params << "'" << mout.endl;
			}
		}

	};

};
//DefaultHandler defaultHandler;

class CmdDebug : public BasicCommand {
    public: //re 

	CmdDebug() : BasicCommand(__FUNCTION__, "Set debugging mode") { // TODO
	};

	inline
	void exec() const {
		CommandRegistry & r = getRegistry();
		r.run("verbose","8");  // FIXME r.setVerbosity();
	};

};


class CmdExpandVariables : public BasicCommand {
    public: //re 

	CmdExpandVariables() : BasicCommand(__FUNCTION__, "Toggle variable expansion on/off") {
	};

	inline
	void exec() const {
		CommandRegistry & r = getRegistry();
		r.expandVariables = ~r.expandVariables;
	};

};


} /* namespace drain */

#endif /* DRAINLET_H_ */
