#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#define MAX_TEXT_LENGTH 1000000
#define MAX_SUBSTRING_LENGTH 500
#define FILENAME_SIZE 50
#define ASCII_SIZE 256

//Takes filename and scans text from that file, takes find and replaces strings, case-sensitive or not from terminal
void scanInputs(char *fileName, char *text, char *find, char *replace, char *sensitive);
//Create badchar table for using shifting process
void createBadCharTable(char *find, int badCharTable[ASCII_SIZE], char caseSensitivity);
//Boyer-Moore Horspool algorithm to find substrings in the given string
void horspool(char *text, char *find, char *replace, char sensitivity);
//A function for using in replacement process when replace string is longer or shorter than the find string
void shiftString(char *text, int start, int lenFind, int lenReplace, int direction);
//A function to paste string to replace on string to find, it works when the strings are the same length
void pasteString(char *text, char *replace, int start);
//This function operates shiftString and pasteString functions
void replaceString(char *text, char *find, char *replace, int start);
//This function writes text into file after find and replace process
void writeToFile(char *fileName, char *text);

int main(){
	char text[MAX_TEXT_LENGTH];
	char find[MAX_SUBSTRING_LENGTH];
	char replace[MAX_SUBSTRING_LENGTH];
	char fileName[FILENAME_SIZE];
	//If sensitive is 'n' or 'N', the process proceeds as non-casesensitive, else casesensitive 
	char sensitive;
	
	scanInputs(fileName, text,find, replace, &sensitive);
	//to calculate operation time, I noted the system time before execution of process 
	clock_t begin = clock();
	horspool(text, find, replace, sensitive);
	//to calculate operatio time, I noted the system time after execution of process
	clock_t end = clock();
	//I calculate the running time
	printf("Running Time :%lf miliseconds.", (double)(end-begin));
	writeToFile(fileName, text);
	
	return 0;
}

//This function writes the text into the given filename
void writeToFile(char *fileName, char *text){
	FILE *fp = fopen(fileName, "a");
	if(fp == NULL){
		printf("File Error");
		exit(1);
	}
	fprintf(fp, "\n\nText after find and replace process\n\n");
	fputs(text,fp);
	fclose(fp);
}
//This functions takes text, find, replace string and the index which we starts the replace process from here
void replaceString(char *text, char *find, char *replace, int start){
	int lenText = strlen(text);
	int lenFind = strlen(find);
	int lenReplace = strlen(replace);
	//if length of string found and length of string to be replaced is equal, we don't need shifting we paste replace directly over find
	if(lenFind == lenReplace){
		pasteString(text, replace, start);
	}
	//if length of find is shorter than length of replace, we must shift the array right, and then we can paste replace over find
	else if(lenFind < lenReplace){
		shiftString(text, start, lenFind, lenReplace, 1);
		pasteString(text, replace, start);
	}
	//if length of find is longer than length of replace, we must shift the array rigth, and then we can paste replace over find
	else{		
		shiftString(text, start, lenFind, lenReplace, 0);
		pasteString(text, replace, start);		
	}
}
//this function takes strings text, find, replace and start index, it paste replace over find in the text
void pasteString(char *text, char *replace, int start){
	int lenReplace = strlen(replace);
	int i;
	for(i = 0; i < lenReplace; i++){
		text[start+i] = replace[i];
	}
}
//This function takes string text, start index, length of find and replace strings and the direction to shift
void shiftString(char *text, int start, int lenFind, int lenReplace, int direction){
	int lenText = strlen(text);
	//direction 0 is equal to shift left
	if(direction == 0){
		int range = lenFind-lenReplace;
		int i;
		//it takes the char in text[i+range] and paste it text[i] from start index which we pass a parameter and goes until it find a NULL char
		for(i = start + lenReplace; text[range+i]; i++){
			text[i] = text[range+i];
		}
		//we must set a new NULL char to the end of text because of changed text length
		text[lenText-range] = '\0';
	}
	//direction 1 is equal to right shift
	if(direction == 1){
		int i;
		int range = lenReplace-lenFind;
		//From end of the text to start index+length of find, it takes chars and paste it new position
		//In this process it replaces the null char at the end of the string on the new end of text automatically we don't have to set null manually
		for(i = lenText; i >= lenFind+start; i--){
			text[i+range] = text[i];
		}
	}
}

void horspool(char *text, char *find, char *replace, char sensitivity){
	//This part creates a badCharTable according to rules of Boyer-Moore Horspool algorithm
	int badCharTable[ASCII_SIZE];	
	createBadCharTable(find, badCharTable, sensitivity);
	
	int found = 0;
	int lenText = strlen(text);
	int lenFind = strlen(find);
	int shift = 0;
	//shift is the index pointer on main text, if it is bigger than length of text - length of find, the find string can't match the last part
	//so we can stop the process
	while(shift <= lenText-lenFind){
		int iFind = lenFind-1;
		//if case is sensitive we must only search the same chars
		if(sensitivity == 'Y' || sensitivity == 'y'){
			//if the chars is equal we goes else we break
			while(iFind >= 0 && find[iFind] == text[shift+iFind]){
				iFind--;
			}
		}
		//if the case is nonsens we have to searh their upper and lower case forms
		else{
			//if the char is equal to char or its upper or lower case form we goes else we break
			while(iFind >= 0 && (find[iFind] == text[iFind+shift] || find[iFind]-text[iFind+shift] == 32 || text[iFind+shift]-find[iFind] == 32)){	
				iFind--;
			}
		}
		//if index of find is less then 0, we found a match
		if(iFind < 0){
			//we call replace function and we pass start index of the process as shift
			replaceString(text, find, replace, shift);
			//Increases found counter
			found++;
			//set new shift index,
			if(shift+lenFind < lenText){
				shift += lenFind-badCharTable[text[shift+lenFind]];
			}else{
				shift += 1;
			}
		}
		//if index of find is greater than 0, we couldn't found it
		else{
			//set new shift index
			if(iFind-badCharTable[text[shift+iFind]]>1){
				shift += iFind-badCharTable[text[shift+iFind]];
			}else{
				shift += 1;
			}
		}
		//if we did a find and replace process most probably we changed the text length so we have to set new length
		lenText = strlen(text);
	}
	printf("\nFound and Replaced:%d\n",found);
}
//this function creates bad char table which used in shifting process according to rules of Boyer Moore Horspool algorithm
void createBadCharTable(char *find, int badCharTable[ASCII_SIZE], char caseSensitivity){
	int lenFind = strlen(find);
	int i;
	//set a array its lenght is length of ascii table
	//set all values as -1 default
	for(i = 0; i < ASCII_SIZE; i++){
		badCharTable[i] = -1;
	}
	//if algorithm is casesensitive we only set the exact letter
	if(caseSensitivity == 'y' || caseSensitivity == 'Y'){
		//from start to end of string we set the value of badchartable as index in find string
		//if a letter has more than once, we set the last occurences of it 
		for(i = 0; i < lenFind; i++){
			badCharTable[(int)find[i]] = i;
		}
	}
	//if algorithm is nonsense we have to set upper and lower cases of this letter
	else{
		//from start to end of string we set the value of badchartable as index in find string
		//if a letter has more than once, we set the last occurences of it 
		for(i = 0; i < lenFind; i++){
			//if the letter is lowercase we have to set its uppercase too
			if((int)find[i] <= (int)'z' && (int)find[i] >= (int)'a'){
				badCharTable[(int)find[i]] = i;
				badCharTable[(int)find[i]-32] = i;
			}
			//if the letter is uppercase we have to set its lowercase too
			else if((int)find[i] <= (int)'Z' && (int)find[i] >= (int)'A'){
				badCharTable[(int)find[i]] = i;
				badCharTable[(int)find[i]+32] = i;
			}
		}
	}
}
//scans inputs from user, it is a very basic function I think don't need to explain it
void scanInputs(char *fileName, char *text, char *find, char *replace, char *sensitive){
	printf("FileName:");
	scanf("%[^\n]s", fileName);
	
	FILE *fp = fopen(fileName, "r");
	if(fp == NULL){
		fprintf(stderr,"File Error");
		exit(1);
	}
	int i;
	for(i = 0; !feof(fp); i++){
		text[i] = getc(fp);
	}
	text[i-1] = '\0';
	fclose(fp);
	
	printf("Find:");
	scanf(" %[^\n]s", find);
	printf("Replace:");
	scanf(" %[^\n]s", replace);
	printf("Case sensitivity (Y:yes N:no):");
	scanf(" %c", sensitive);
}
