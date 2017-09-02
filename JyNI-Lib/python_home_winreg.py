from _winreg import HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, QueryValue

key1 = "SOFTWARE\\Python\\PythonCore\\2.7\\InstallPath"
key2 = "SOFTWARE\\Wow6432Node\\Python\\PythonCore\\2.7\\InstallPath"

def python_home():
	try:
		return QueryValue(HKEY_LOCAL_MACHINE, key2)
	except:
		pass
	try:
		return QueryValue(HKEY_LOCAL_MACHINE, key1)
	except:
		pass
	try:
		return QueryValue(HKEY_CURRENT_USER, key1)
	except:
		pass

if __name__ == '__main__':
	print python_home()[:-1]
