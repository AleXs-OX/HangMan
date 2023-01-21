#include "HD44780.hpp"
#include "Arduino.h"
#include <LiquidCrystal.h>
#include "libADC.hpp"
#include <stdio.h>

const int rs = 8, en = 9, d4 = 4, d5 = 4, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Functions
int prueba(void);
void printLowBars(int n);
void printAlphabet(void);
void printNumber(void);
void guessALetter(void);
void updateVisor(int pos, bool direction);
bool checkLetter(int pos);
void winOrLose(bool result);
void loseMessage(void);
void winMessage(void);

//DEFINE lives 5;

//Vars
int nLwBars = 0;
int randmN = 3; //random(3); You can choose random or set an specific word to test the program.
int posSelector = 0; //0-26
int wordChosen = 0; //Word chosen within the alphabet
int lifeCounter = 5;

//Predefined size of the Words
int lettersChosen = 0;
char *lowWord[14];
String dictionary[10] = { "hello", "one", "three","lucia","win","horse" };

uint16_t raw;

char *alphabet[26] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
		"l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "w", "x", "y", "z",
		" " };
// "" -> pos 25

char *alphabetASCII[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'w', 'x', 'y',
		'z', ' ' };
// "" -> pos 25

//Default setup guess a letter
char *visor[7] = { ">", alphabet[0], "<", alphabet[1], alphabet[2], alphabet[3],
		alphabet[4] };

void setup() {
	ADC_Init();
	LCD_Initalize();
	LCD_Clear();

	Serial.begin(9600);
	//cogeLetra()

	//Choose random word from the library

	//Set the number of low bars
	nLwBars = dictionary[randmN].length();
	//nLwBars = 7;
	//Fill the lowBars
	for (int i = 0; i < nLwBars; i++) {
		lowWord[i] = "_";
	}
	//[Deprecated] Set the last position to know the size of the array char
	//[Deprecated] lowWord[nLwBars] = ".";
}

void loop() {

	//====================================================

	//Print the alphabet
	printAlphabet();

	//Print the low bars
	printLowBars(nLwBars);

	//Guess the letter and check if the player win or lose
	guessALetter();

	//Test print the number direction
	//printNumber();

}
void printLowBars(int n) {

	/*This function print the lowBars*/
	for (int i = 0; i < nLwBars; i++) {
		LCD_GoTo((i + 5), 1);
		LCD_WriteText(lowWord[i]);
	}
}

//Function that print the Alphabet (VISOR)
/*The visor is the array composed by 7 letters that will be show
 in the LCD*/

void printAlphabet() {
	char *alphabetb[26] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
			"k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "w", "x",
			"y", "z" };

	char str[8];
	LCD_GoTo(5, 0);
	for (int i = 0; i < 7; i++) {
		//sprintf(str ,"%d",visor[i]);
		LCD_WriteText(visor[i]);
	}

	LCD_GoTo(0, 0);
	sprintf(str, "L:%d", lifeCounter);
	LCD_WriteText(str);

	delay(150);
}

void guessALetter() {

	//Function that detect the buttons pressed
	LCD_GoTo(5, 0);
	int nPressed = 0, pos = 0, ok = 0;
	char str[8];

	//Button detection
	while (ok == 0) {
		//====================================================================
		ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC));
		raw = ADC;
		/*
		 RIGHT		0
		 UP 		131
		 DOWN		306
		 LEFT		480
		 SELECT	721
		 */
		//====================================================================
		delay(5);

		//Right And setting the limit alphabet
		if ((raw >= 0) && (raw < 100) && (posSelector < 24)) {
			ok = 1;
			updateVisor(posSelector++, true);
		}
		//Left And setting the limit alphabet
		else if ((raw > 350) && (raw < 500) && (posSelector > 0)) {
			ok = 1;
			updateVisor(posSelector--, false);
		}
		//Select
		else if (raw > 500 && raw < 800) {
			ok = 1;
			//Call function that check if the letter exist
			/*
			 * Send the result to the function wir or lose to check if the word introduced is correct
			 * and count the remaining lives
			 * */
			winOrLose(checkLetter(posSelector));
		}
	}
}

void updateVisor(int pos, bool direction) {

	//Important: Only Update position 1,3,4,5,6
	//Direction true -> Right
	//Direction false -> Left

	//Distinguishes between right or left movement
	int direc = 0;

	/*Change the movement, if is left then just once go back one position
	 if is right move along*/

	/*The code is prepared to detect the movement left or right
	 * if is left direct = -1 to go back one position in the vector VISOR, once this
	 * is completed the update of the vector VISOR is normal. Move the position
	 * The var direct act like a sum +1 only to advance position*/

	if (direction)
		direc = 1;
	else
		direc = -1;

	/* "> 20" because the position N21 is the top of the array to show correctly
	 * the letters of the VISOR
	 */

	int t = pos;
	if (pos > 20) {
		for (int i = 0; i < 7; i++) {

			if ((i != 2 && i != 0)) {
				if (i > (26 - t)) {
					visor[i] = alphabet[25];
				} else {
					visor[i] = alphabet[pos + direc];
					pos = pos + direc;
					//Reload the normal direction
					direc = 1;
				}
			}
		}
	} else {
		for (int i = 0; i < 7; i++) {
			if (i != 2 && i != 0) {
				visor[i] = alphabet[pos + direc];
				pos = pos + direc;
				//Reload the normal direction
				direc = 1;
			}
		}
	}

	printAlphabet();
}

void printNumber() {
	LCD_GoTo(0, 1);
	char str[8];
	int num = 0;
	num = ADC_conversion();
	sprintf(str, "%d", num);
	LCD_WriteText(str);

}

bool checkLetter(int pos) {

	//Detect if the letter selected is correct

	//Transform String to char to go through and compare them
	String word = dictionary[wordChosen];

	//Create char with the word length
	char wordC[word.length() + 1];
	char *letter = alphabetASCII[pos];
	//Check if the word exist
	bool exist = false;
	for (int i = 0; i < word.length(); i++) {
		if ((int) word[i] == (int) letter) {
			exist = true;
			lowWord[i] = alphabet[pos];
			lettersChosen++;
		}
	}
	//Update the alphabet and mark the letter chosen
	alphabet[pos] = "*";
	delay(100);

	//Print the alphabet
	printAlphabet();

	return exist;
}
void winOrLose(bool result) {
	bool stillAlive = false;;

	if (!result){
		lifeCounter--;
		if(lifeCounter<=0){
			loseMessage();
		}
	}else{
		if(lettersChosen == nLwBars)
			winMessage();
	}
}

void loseMessage() {
	LCD_Clear();
	while (1) {
		LCD_GoTo(3, 0);
		LCD_WriteText("You Lose!!");
		LCD_GoTo(0, 1);
		LCD_WriteText("Rset the Arduino!");
	}
}

void winMessage(){
	LCD_Clear();
	while (1) {
		LCD_GoTo(3, 0);
		LCD_WriteText("You Win!!");
		LCD_GoTo(0, 1);
		LCD_WriteText("Rset the Arduino!");
	}
}


