#ifndef index_tokenizer_h
#define index_tokenizer_h

struct TokenizerT_ {
	char* copied_string;
	char* current_position;
};
typedef struct TokenizerT_ TokenizerT;

/*
 * Description: determines if a character is a member of the set of escaped characters
 * Parameters: character to be evaluated
 * Modifies: nothing
 * Returns: appropriate escape character sequence if it is a member, 0 if it is not
 *
 */
char is_escape_character(char character);

/*
 * Description: converts a hex digit represented as a character into its actual integer value
 * Parameters: character to be converted
 * Modifies: nothing
 * Returns: value of the hex digit character as an integer
 *
 */
int char_to_hex(char character);

/*
 * Description: converts a octal digit represented as a character into its actual integer value
 * Parameters: character to be converted
 * Modifies: nothing
 * Returns: value of the octal digit character as an integer
 *
 */
int char_to_oct(char character);

/*
 * Description: determines if a character represents a octal digit
 * Parameters: character to be evaluated
 * Modifies: nothing
 * Returns: 1 if it is an octal character, 0 if it is not
 *
 */
int is_oct_digit(char oct_digit);

/*
 * Description: given an input stream converts escaped characters into their appropriate char representation and returns it as a new string
 * Parameters: string to be converted
 * Modifies: nothing
 * Returns: resulting string with all the escaped characters with their appropriate values
 *
 */
char* unescape_string(char* string);

/*
 * Description: creates a new tokenizer struct from the token stream and delimiters
 * Parameters: set of delimiters, token stream
 * Modifies: nothing
 * Returns: a pointer to a tokenizer struct on success, a null pointer on failure
 *
 */
TokenizerT *TKCreate(char *ts);

/*
 * Description: destroys tokenizer struct and deallocates all memory
 * Parameters: tokenizer to be destroyed
 * Modifies: tokenizer struct: deallocates memory
 * Returns: nothing
 */
void TKDestroy(TokenizerT *tk);

/*
 * Description: determines if a particular character is a an alphanumeric character
 * Parameters: character to be checked
 * Modifies: Nothing
 * Returns: 1 if character is a delimiter, 0 if it is not
 */
char is_delimiter(char character);

/*
 * Description: returns the next token from the token stream specified within the tokenizer
 * Parameters: tokenizer from which to extract token
 * Modifies: tokenizer->current_position: identifies starting point of next token; creates a new string with
 * Returns: token extracted as a char* on success, null on failure/end of string;
 */
char *TKGetNextToken(TokenizerT *tk);
#endif