/** 
 * File:   msgparser.h
 *
 * msgparser handles access to the shared NFS message database. Msgparser
 * handles all access to this file e.g. retrieving a message witha given index
 * or appending a message. Individual messages can have an arbitrary length
 * and can contain newlines. 
 * 
 * Messages stored in the file are given an unique id starting sequentially at 1. That is,
 * the first message has an ID of 1, the second has an ID of 2, and so forth. This allows
 * users to retrieve messages by ID in a predictable way.
 * 
 * Messages are XML-formatted in the following way:
 *
 * HEADER:
 *      FORMAT:  <message n=number>
 *      USAGE:   Indicates the start of a message. replace number with the message ID.
 *
 * BODY:
 *      FORMAT: ASCII-formatted text
 *      USAGE: Multiple lines of text. They may not contain the header or footer.
 *
 * FOOTER:
 *      FORMAT: </message>
 *      USAGE: Indicates the end of the message.
 *
 * EXAMPLE:
 *      <message n=1>
 *      Roses are red.
 *      Violets are blue.
 *      The binary's sweet
 *      1 + 1 = 10
 *      </message>
 *
 * The XML tags are case-sensitive. The tags should be used exactly as documented above.
 * If the format is not followed exactly, the message will be rejected. 
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 2/13/2017
 * @info Course COP4635
 */
 
#ifndef MSGPARSER_H
#define MSGPARSER_H

#include <stdio.h>
 
#define MSG_LOG "messages.txt" 
#define FIRST_MESSAGE_INDEX 1

#define OPENING_XML_TAG_FORMAT "<message n=%d>"
#define CLOSING_XML_TAG_FORMAT "</message>"
 
 /**
  * Retrieves a message from the database by its ID. This is accomplished by
  * scanning the messages in the database in a linear manner, starting with the
  * first. The message body is printed to the given IO stream (such as the terminal).
  * Error codes are printed to stderr.
  *
  *	USAGE:
  *		// Attempt to retrieve the 12th message
  * 	readMessageByID(12, stdout);
  *
  * @param match the ID the message to find and print
  * @param outputStream the stream to print the message body to
  * @return 1 if successful, 0 otherwise
  */
 
 int readMessageByID(int match, FILE *outputStream);
 
 /**
  * Returns the ID of the most recent (last) message posted to the the <MSG_LOG> 
  * database file. This is also the number of messages in the file. Error codes are printed
  * to stderr.
  *
  *	USAGE:
  *		// Find the number of messages in the database
  * 	count = getMessageCount();
  *
  * @return the ID of the most recent message (the total number of messages) or FIRST_MESSAGE_INDEX-1 if unsuccessful. 
  */

 int getMessageCount();
 
 /**
  * Appends a text string to the end of the <MSG_LOG> log file
  * as a message. The message is given an ID one greater than the ID of the last
  * message in a file. Error codes are printed to stderr.
  * 
  * @param msg pointer to nullterminated string containing the message to append.
  * @return 1 if successful, 0 otherwise
  */
  
 int appendMsg(const char *msg);
 
 #endif
