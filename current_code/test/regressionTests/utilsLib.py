from ldtp import *

def startAppTest():
	try:
		guitimeout (90)
		testDesc = 'Test: Openning App'
		log (testDesc, 'teststart')
		launchapp ('/bin/bash', ['test/runMe'])
		if waittillguiexist ("frmopenDIAS") == 0:
			raise LdtpExecutionError ('Cannot open application')
		log (testDesc, 'pass')
		log (testDesc, 'testend')
		guitimeout (30)
	except LdtpExecutionError, msg:
		log (msg, 'cause')
		log (testDesc, 'fail')
		log (testDesc, 'testend')
		guitimeout (30)

def closeAppTest():
	try:
		guitimeout (90)
		testDesc = 'Test: Closing App'
		log (testDesc, 'teststart')
		selectmenuitem('frmopenDIAS', 'File;Exit')
		if waittillguinotexist ("frmopenDIAS") == 0:
			raise LdtpExecutionError ('Cannot close application')
		log (testDesc, 'pass')
		log (testDesc, 'testend')
		guitimeout (30)

	except LdtpExecutionError, msg:
		log (msg, 'cause')
		log (testDesc, 'fail')
		log (testDesc, 'testend')
		guitimeout (30)


