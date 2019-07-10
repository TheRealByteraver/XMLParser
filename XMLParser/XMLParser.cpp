// XMLParser.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <conio.h>
#include "XMLParser.h"


bool XMLFile::readFileToBuffer( 
    std::stringstream& rawData, 
    std::string& filename )
{
    // Open the XML file
    std::ifstream xmlFile( filename.c_str() );
    if ( !xmlFile.is_open() ) return false;

    // read the whole file into memory
    rawData << xmlFile.rdbuf();
    xmlFile.close();
    return true;
}


bool XMLFile::getTag( 
    std::stringstream& rawData,
    std::string& tagStr,
    std::string& precedingStr,
    std::string& trailingStr )
{
    trailingStr.clear();
    tagStr.clear();

    // start reading where we left of
    std::string s( precedingStr );
    precedingStr.clear();

    if ( s.length() == 0 ) {
        rawData >> s;
        s += ' ';
    }
    // start looking for the '<' marker
    size_t pos1 = std::string::npos;
    for ( ; !rawData.eof(); ) {
        pos1 = s.find( TAG_START_MARKER );
        if ( pos1 == std::string::npos ) {
            precedingStr += s;
        }
        else { // we found the '<' marker!
            // we copy the last characters preceding the marker
            // to the preceding string:
            precedingStr += s.substr( 0,pos1 );
            // we continue with everything after the '<' marker,
            // including the marker itself:
            s = s.substr( pos1 );
            if ( s.length() < 2 ) // space after '<' is not allowed
                return false;
            break; // we are done here
        }
        rawData >> s;
        s += ' ';
    }
    if ( pos1 == std::string::npos )
        return false; // we didn't find a tag start marker

    // start looking for the '>' marker
    for ( ; !rawData.eof(); ) {
        // check if string contains a '>':
        size_t pos2 = s.find( TAG_END_MARKER );
        if ( pos2 != std::string::npos ) {
            // the '>' marker might not be at the end of the string:
            if ( (int)pos2 != (s.length() - 1) ) {
                // exclude the '>' character for the trailing string:
                trailingStr = s.substr( pos2 + 1 );
                // cut off the excess characters but keep the '>' marker:
                s.resize( pos2 + 1 );
            }
            tagStr += s;
            return true;
        }
        tagStr += s;
        rawData >> s;
        s += ' ';
    }
    return false;
}

bool XMLFile::tagIsAComment( std::string& tagStr, bool& isValidComment )
{
    isValidComment = false;
    std::string head( tagStr );
    int startMarkerLength = std::string( COMMENT_START_MARKER ).length();
    int endMarkerLength = std::string( COMMENT_END_MARKER ).length();
    int tagLength = tagStr.length();
    head.resize( startMarkerLength );
    if ( head != COMMENT_START_MARKER )
        return false;

    std::string tail = tagStr.substr( tagLength - endMarkerLength );
    std::cout << "\nTail = |" << tail << "|";

    // At this point it is a comment alright. If there is no end marker
    // it is malformed and we return without setting isCommentValid to true
    if ( tail != COMMENT_END_MARKER )
        return true; 

    // Now we need to check if the comment includes the string "--" apart
    // from the start and end marker (this is not allowed).
    size_t pos = tagStr.find( COMMENT_ILLEGAL_CONTENT,startMarkerLength );
    if ( pos != (tagLength - endMarkerLength) )
        return true;
    /*
    std::cout << "\nPos = " << pos
        << "\n(tagLength - endMarkerLength) = " << (tagLength - endMarkerLength)
        << "\ntagLength       = " << tagLength
        << "\nendMarkerLength = " << endMarkerLength
        ;
    std::string pauze;
    std::cin >> pauze;
    */
    // Comment is OK
    isValidComment = true;
    return true;
}

bool XMLFile::tagIsSelfClosing( std::string& tagStr )
{
    size_t l = tagStr.length();
    if ( l < 4 ) // ? min size = '<a b="c"/>' --> 10 chars?
        return false;
    return (tagStr[l - 2] == TAG_CLOSE_MARKER);
}

bool XMLFile::tagIsAClosingTag( std::string& tagStr )
{
    size_t l = tagStr.length();
    if ( l < 4 )
        return false;
    return (tagStr[1] == TAG_CLOSE_MARKER);
}

bool XMLFile::tagIsXMLVersionTag( std::string& tagStr )
{
    // <?xml version="1.0"?> Minimum amount of chars is 19 ?
    size_t l = tagStr.length();
    if ( l < 19 )
        return false;
    // very simple check:
    return ( 
        (tagStr[0] == '<') &&
        (tagStr[1] == '?') &&
        (tagStr[l - 2] == '?') &&
        (tagStr[l - 1] == '>') 
        );
}

bool XMLFile::extractTagNameAndAttributes( 
    std::string& tagStr,
    XMLElement& xmlElement )
{
    std::stringstream s( tagStr );
    for ( ;!s.eof();) {
        std::string s2;
        s >> s2;
        std::cout 
            << "\n" 
            << s2
            << "#";
    }
    return true;
}

bool XMLFile::readFile()
{
    // Open the XML file
    std::stringstream rawData;
    readFileToBuffer( rawData,filename_ );

    // a few flags to keep track of decoding situation
    bool isXMLVersionTag = false;



    // stack of XML Elements, for decoding nested XML elements:
    std::stack< XMLElement > xmlElementStack;
    int stackCount = 0;  // how deep into the stack we are

    std::string precedingStr; 
    std::string tagStr;
    std::string trailingStr;
    std::string previousTagValue;

    for ( ; !rawData.eof(); ) {
        // read a tag (could be a whole element)     
        getTag( rawData,tagStr,precedingStr,trailingStr );

        // current XML element we are decoding:
        XMLElement  xmlElement;
        bool extract = extractTagNameAndAttributes( tagStr,xmlElement );




        // check if we are dealing with an XML style comment
        bool isComment;
        bool isValidComment;
        isComment = tagIsAComment( tagStr,isValidComment );

        // check if we have a complete tag value (what is between two tags)
        if ( tagIsSelfClosing( tagStr ) )
            previousTagValue.clear();
        else if ( tagIsAClosingTag( tagStr ) )
            previousTagValue = precedingStr;
        else previousTagValue.clear();

        // check if this tag is the xml version description
        bool isXMLVersion = tagIsXMLVersionTag( tagStr );
        
        std::cout // DEBUG
            //<< "\nPreceding String: |" << precedingStr << "|"
            << "\nTag: |" << tagStr << "|"
            //<< "\nTrailing  String: |" << trailingStr << "|"
            << "\nTag value: |" << previousTagValue << "|"
          //  << "\nIs it a comment: " << (isComment ? "Yes" : "No")
          //  << ", is comment valid: " << (isValidComment ? "Yes" : "No")
            << "\nIs a XML version tag: " << (isXMLVersion ? "Yes" : "No")
            << "\n";

        if ( isComment && (!isValidComment) )
            return false;

        // Prepare for next tag read:
        precedingStr = trailingStr;

    }
    return true;
}

char *XMLFile::deleteWhiteSpace( char *buf ) const
{
    char *d = buf;
    char *s = buf;
    for ( ; *s != '\0'; )
    {
        if ( (*s != ' ') && (*s != '\t') )
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
    return buf;
}


void main()
{
    XMLFile     xmlFile( "c:\\RTSMedia\\beach-ball.svg" );
    std::cout 
        << "\nXML file was loaded correctly: "
        << (xmlFile.isLoaded() ? "yes" : "no") << "\n";

    _getch();

    XMLFile     xmlFile2( "c:\\RTSMedia\\purchase.xml" );
    std::cout
        << "\nXML file was loaded correctly: "
        << (xmlFile2.isLoaded() ? "yes" : "no") << "\n";

    _getch();
}

