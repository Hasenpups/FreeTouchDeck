/*
 * User actions you can modify these functions to do custom stuff.
 * Have a look at the 3 examples for some inspiration. Also, have a look
 * at Actions.h to see what actions you can use (eg. print, write, press, etc.)
 * 
 */

// After any action you might need a delay, this delay (in ms) is defined here:
#define USER_ACTION_DELAY 50

// Function used to print large pieces of text.
void printLargeString(const char string[]);

void userAction1(){
  printLargeString("Rubbeldiekaty01");
}

void userAction2(){
  printLargeString("EXPERT");
  delay(USER_ACTION_DELAY);
  bleKeyboard.write(KEY_TAB);
  delay(USER_ACTION_DELAY);
  printLargeString("LAETUS");
}

void userAction3(){
  printLargeString("wt/user"); // "-" needs to be printed as this
  delay(USER_ACTION_DELAY);
  bleKeyboard.write(KEY_TAB);
  delay(USER_ACTION_DELAY);
  printLargeString("wt");
}

void userAction4(){
  printLargeString("laetusadmin");
  delay(USER_ACTION_DELAY);
  bleKeyboard.write(KEY_TAB);
  delay(USER_ACTION_DELAY);
  printLargeString("la");
}

void userAction5(){

  // (Mac only) This opens a new file in Sublime (has to be installed off course and pastes the last thing you copied to the clipboard.
  // I use this to select pieces of text and copy them to a new file.
  bleKeyboard.press(KEY_LEFT_GUI);
  delay(USER_ACTION_DELAY);
  bleKeyboard.print(" ");
  bleKeyboard.releaseAll();
  printLargeString("Sublime");
  bleKeyboard.write(KEY_RETURN);
  delay(500);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.print("n");
  bleKeyboard.releaseAll();
  delay(USER_ACTION_DELAY);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.print("v");
  bleKeyboard.releaseAll();
  
}

void userAction6(){

  // (Windows only) This opens a new file in Sublime (has to be installed off course and pastes the last thing you copied to the clipboard.
  // I use this to select pieces of text and copy them to a new file.
  bleKeyboard.press(KEY_LEFT_GUI);
  delay(USER_ACTION_DELAY);
  bleKeyboard.print("r");
  bleKeyboard.releaseAll();
  delay(500);
  printLargeString("notepad");
  bleKeyboard.write(KEY_RETURN);
  delay(500);
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.print("v");
  bleKeyboard.releaseAll();
  
}

void userAction7(){
  printLargeString("cukmuq");
  bleKeyboard.print("/"); // "-" needs to be printed as this
  printLargeString("8fiVpu");
  bleKeyboard.print("/");
  printLargeString("xijxzp");
}



/* A simple function to print large strings using bleKeyboard. The function has a delay
 * between keypresses to not miss any, or hang on 1.
*/

void printLargeString(const char string[]){

  for(int i=0; i < strlen(string); i++ ) {
    char c = string[i];
    bleKeyboard.print(c);
    delay(15); // 10ms is on most systems enough to not miss a character
  }
  
}
