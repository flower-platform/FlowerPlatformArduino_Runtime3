/*
 *  Author: Anca Barbu
 */

#ifndef FlowerPlatformStringManip_h
#define FlowerPlatformStringManip_h

String changeCaseOfString(String s, String caseMode) {
	String result = s;
	
	if (caseMode == "lower") {
		result.toLowerCase();
	} else if (caseMode == "upper") {
		result.toUpperCase();
	}
	Serial.println(s);
	Serial.println(result);
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

//TODO when we add suport for lists, it needs to be modified
/*String* splitText(String text, String delim) {
	String copyText = text;
	int c, found = 0;
	while ( (c = copyText.indexOf(delim)) != -1) {
		found ++;
		copyText = copyText.substring(c + delim.length()	, copyText.length());
	}
	String result[found];
	int pos, i = 0;
	copyText = text; 
	
	for (i = 0; i < found; i++) {
		pos = copyText.indexOf(delim);
		result[i] = copyText.substring(0, pos);
		copyText = copyText.substring(pos + delim.length(), copyText.length());		
	}
	result[found] = copyText;
	return result;
}

//TODO when we add suport for lists, it needs to be modified
String* splitTextAtFirst(String text, String delim) {
	String result[2];
	int c;
	if ( (c = text.indexOf(delim)) != -1) {
		result[0] = text.substring(0, c);
		result[1] = text.substring(c + delim.length(), text.length());
	}
	return result;

}*/

#endif

