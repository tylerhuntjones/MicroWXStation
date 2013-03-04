CONFIG_FILE="logs2m.conf"
VERSION="0.1"

import time, shelve, os, sys, random, string
import shutil, re

try:
    config = open(CONFIG_FILE).read().split("\n")
except IOError:
    cwd = raw_input("LogSerial2MySQL has detected that you are running LogSerial2MySQL from a nonstandard path\nPlease enter the absolute path for the LogSerial2MySQL directory (logserial2mysql-%s/)\n(e.g. /home/user/logserial2mysql-%s)\n>>> " % (re.compile(".*v(\d+\.\d+).*").findall(VERSION)[0], re.compile(".*v(\d+\.\d+).*").findall(VERSION)[0]))
    os.chdir(cwd)

configdict = shelve.open(".config")
configdict.clear()
configdict.close()

def configure(rehash="NO"):
    config = open(CONFIG_FILE).read().split("\n")
    linenumber = 1
    defined = {}
    for line in config:
        if line == "":
            pass
        elif line[0] == "#":
            pass
        else:
            option = line.split()[0]
            if option == "MySQL_DB":
                if "MySQL_DB" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_DB already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_DB = line.split()[1]
                        defined["MySQL_DB"] = MySQL_DB
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_DB ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

	    elif option == "MySQL_User":
                if "MySQL_User" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_User already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_User = line.split()[1]
                        defined["MySQL_User"] = MySQL_User
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_User ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

	    elif option == "MySQL_Pass":
                if "MySQL_Pass" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_Pass already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_Pass = line.split()[1]
                        defined["MySQL_Pass"] = MySQL_Pass
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_Pass ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

	    elif option == "MySQL_Host":
                if "MySQL_Host" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_Host already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_Host = line.split()[1]
                        defined["MySQL_Host"] = MySQL_Host
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_Host ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

	    elif option == "MySQL_Port":
                if "MySQL_Port" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_Port already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_Port = line.split()[1]
                        defined["MySQL_Port"] = MySQL_Port
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_Port ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

	    elif option == "MySQL_DBTable":
                if "MySQL_DBTable" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_DBTable already defined, using earlier definition ***"
                else:
                    try:
                        MySQL_DBTable = line.split()[1]
                        defined["MySQL_DBTable"] = MySQL_DBTable
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_DBTable ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

            elif option == "SerialPort":
                if "SerialPort" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** SerialPort already defined, using earlier definition ***"
                else:    
                    try:
                        SerialPort = line.split()[1]
                        defined["SerialPort"] = SerialPort
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option SerialPort ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"


            else:
                print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                print "*** Unknown option: " + option + " ***"
                print "*** Ignoring ***"
        linenumber += 1
        
    if "MySQL_DB" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_DB defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "MySQL_User" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_User defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "MySQL_Pass" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_Pass defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "MySQL_Host" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_Host defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "MySQL_Port" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_Port defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "MySQL_DBTable" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_DBTable defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    if "SerialPort" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No SerialPort defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    return defined
