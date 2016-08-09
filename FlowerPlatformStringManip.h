/*
 *  Author: Anca Barbu
 */

#ifndef FlowerPlatformStringManip_h
#define FlowerPlatformStringManip_h

String changeCaseOfString(String s, char* caseMode) {
	String result = s;
	if (strcmp(caseMode, "lower")) {
		result.toLowerCase();
	} else if (strcmp(caseMode, "upper")) {
		result.toUpperCase();
	}
	return result;
}

String getTrimmedString(String s) {
	String result = s;
	result.trim();
	return result;
}

String getReplacedString(String text, String what, String with) {
	String result = text;
	result.replace(what, with);
	return result;
}

#endif
