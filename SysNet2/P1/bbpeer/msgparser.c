/** 
 * File:   msgparser.c
 *
 * Implements msgparser.h. See msgparser.h for details.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 2/13/2017
 * @info Course COP4635
 */
 
 #include <string.h>
 #include "msgparser.h"
 
/**
 * Scans the file until a given character is reached. The filestream
 * points to the character following the first match after the intial
 * seek position.
 * 
 * @param fileP the pointer to the file of interest
 * @param match the char to find
 * @return 1 when successful, 0 if no match found before the end of the file.
 */
 
 int searchFileForChar(FILE* fileP, char match);
 
 /**
  * Searches MSG_LOG for the XML header of a message with a given ID.
  * Messages should be formatted as described in msgparser.h. If the header
  * is found, the input file pointer is directed to the first character of the
  * following message body.
  * 
  * @param fileP the pointer to the message log
  * @param msgID the id of the message to locate
  * @return msgID if successful, 
  *         otherwise the ID of the last message in the file if no parsing errors occurred, 
  *         otherwise FIRST_MESSAGE_INDEX-1
  */
  
 int searchLogByID(FILE* msgLog, int msgID);
 
/**
 * Adds a newline (\n) character to the end of a file stream if the following conditions hold:
 *      A) The file is not empty.
 *      B) The final character in the stream is not a newline
 * 
 * @param fp file to potentially append a new line to
 */
 
 void addNewLineIfNone(FILE* fp);
  
  // Buffer for storing incoming lines of the message log. Note the maximum line length
  // including the new line and the null-terminating character
 char msgLineBuffer[1024];
 
 int searchFileForChar(FILE* fileP, char match)
 {
     char curChar;
     
     // Loop until EOF or match found
     while (!feof(fileP))
     {
         // Get the next char from the file stream
         curChar = getc(fileP);
         
         if (curChar == match)
         {
             // Match found
             return 1;
         }
     }
     
     return 0;
 } 
 
 int searchLogByID(FILE* msgLog, int match)
 {
    // Initially assume that there is a file error
    int msgCnt = FIRST_MESSAGE_INDEX-1;
    
    // If file was successfully opened
    if (msgLog != NULL)
    {
        // Scan from the beginning to the end of the file. The
        // Last correctly-formed message start is the index of the
        // last file.
        
        int matchResult = FIRST_MESSAGE_INDEX-1;
        int tempMsgCnt = FIRST_MESSAGE_INDEX-1;  // Temporary count isolates candidate values until they are completely vetted
        
        while (!feof(msgLog))
        {
            // Check the format of the message opening line
            // Store msgCnt if the formatting is correct.
            matchResult = fscanf(msgLog, OPENING_XML_TAG_FORMAT, &tempMsgCnt);
            
            // Did the formatting match?
            if (matchResult)
            {
                msgCnt = tempMsgCnt;
                
                // If header with matching id found
                if (msgCnt == match)
                {
                    searchFileForChar(msgLog, '\n');
                    return msgCnt;
                }
            }
            
            // Scan to the next closing bracket (the first possibility for a
            // new opening XML tag)
            searchFileForChar(msgLog, '>');
            
            // Scanning forward to the next newline keeps fscanf from matching
            // the third condition twice due to the EOF. It also increases efficiency.
            searchFileForChar(msgLog, '\n');
       } 
    }
    
    return msgCnt;
 }
 
 int readMessageByID(int match, FILE* outputStream)
 {
     // Open the message log for reading
     FILE *msgLog = fopen(MSG_LOG, "rb");
     
     // Was the message body successfully read?
     int success = 0;
     
     if (msgLog == NULL)
     {
            perror("Error loading message log");
            return success;
     }
     
     // Search for a nonexistent message ID to find the last one in the file
     int searchStatus = searchLogByID(msgLog, match);
     
     // If the desired message was found
     if (searchStatus == match)
     {
         // Print the message until closing tag found
         char* endTagMatch;
         
         // Current line of text from message body (starting at 1 for first line)
         int bodyLineNum = 0;
         
         do
         {
             // Load a line into the buffer
             fgets(msgLineBuffer, sizeof(msgLineBuffer), msgLog);
             
             // search for XML closing tag
             endTagMatch = strstr(msgLineBuffer, CLOSING_XML_TAG_FORMAT);
             
             // Was the closing tag found?
             if (endTagMatch != NULL)
             {
                 // Do not print any text from from the closing tag line
                 success = 1;
                 
                 // Add an extra new line if the message body was empty
                 if (bodyLineNum == 0)
                 {
                        fputc('\n', msgLog);
                 }
                 
                 break;
             }
             else
             {
                 bodyLineNum++;
                 
                 // No closing tag... print the line to the terminal
                 fprintf(outputStream, "%s", msgLineBuffer);
             }
         } while (!feof(msgLog));
     }
     else
     {
         // Error: desired message not found
         fprintf(stderr, "Message %d not found.\n", match);
     }
     
     // Close the message log
     fclose(msgLog);
     
     return success;
 }

 int getMessageCount()
 {
     // Open the message log for reading
     FILE *msgLog = fopen(MSG_LOG, "rb");
     
     if (msgLog == NULL)
     {
            perror("Error loading message log");
            return 0;
     }
     
     // Search for a nonexistent message ID to find the last id in the file
     int lastMessage = searchLogByID(msgLog, FIRST_MESSAGE_INDEX-1);
     
     // Close the message log
     fclose(msgLog);
     
     return lastMessage;
 }
 
 int appendMsg(const char *msg)
 {
     // Open the message log for appending and reading
     // The log file is created if it doesn't already exist
     FILE *msgLog = fopen(MSG_LOG, "a+");
     
     if (msgLog == NULL)
     {
            perror("Error loading message log");
            return 0;
     }
     
     // Search for a nonexistent message ID to find the last id in the file
     int msgCount = searchLogByID(msgLog, FIRST_MESSAGE_INDEX-1);
     
     // Seek to the end of the file
     fseek(msgLog, 0L, SEEK_END);
     
     // If the file was available and not corrupted
     if (msgCount > FIRST_MESSAGE_INDEX-1 || ftell(msgLog) == 0L)
     {
        
        addNewLineIfNone(msgLog);
        
        // If file successfully opened...
        if (msgLog != NULL)
        {
            int msgBodyLen = strlen(msg);
            
            // Write header (assume previous line ends with newline)
            fprintf(msgLog, OPENING_XML_TAG_FORMAT"\n", msgCount+1);
            
            // Write message body
            fputs(msg, msgLog);
            
            // Add a trailing newline if there isn't one in the message
            if(msgBodyLen != 0 && msg[msgBodyLen-1] != '\n')
            {
                fputc('\n', msgLog);
            }
            
            // Write footer
            fprintf(msgLog, CLOSING_XML_TAG_FORMAT"\n");
        }
             
        // Close the message log
        fclose(msgLog);
        
        // Report success
        return 1;
     }
     else
     {
         fprintf(stderr, "Message log is corrupt: Unable to write in new message.\n");
     }
      
     return 0;
 }

 void addNewLineIfNone(FILE* fp)
 {
    // If file not empty
    if (ftell(fp) != 0) 
    {
        // Seek back one character
        fseek(fp, -1L, SEEK_END);
        
        // If last char in the file is not a newline
        if (fgetc(fp) != '\n')
        {
            fputc('\n', fp);
        }
    }
 }
