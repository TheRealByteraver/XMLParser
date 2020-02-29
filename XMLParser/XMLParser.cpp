// XMLParser.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <conio.h>
#include "XMLParser.h"

std::string& XMLFile::trimTrailingSpaces( std::string& trimStr )
{
    size_t strLen = trimStr.length ();
    for ( ; strLen > 0; strLen-- ) {
        if ( trimStr[strLen] != ' ' )
            break;
    }
    trimStr.erase ( strLen );
    return trimStr;
}

bool XMLFile::readFileToBuffer( 
    std::stringstream& rawData, 
    std::string& filename )
{
    // Open the XML file
    std::ifstream xmlFile( filename.c_str() );
    if ( !xmlFile.is_open() ) 
        return false;

    // read the whole file into memory
    rawData << xmlFile.rdbuf();
    xmlFile.close();
    return true;
}

bool XMLFile::extractFromStringStream(
    std::stringstream& rawData,
    std::string startMarker,
    std::string endMarker,
    std::string& destStr,
    std::string& precedingStr,
    std::string& trailingStr )
{
    trailingStr.clear();
    destStr.clear();

    // start reading where we left of
    std::string s( precedingStr );
    precedingStr.clear();

    if ( s.length() == 0 ) {
        rawData >> s;
        if( s.length() > 0)
            s += ' ';
    }
    // start looking for the start marker (e.g. '<')
    size_t pos1 = s.find( startMarker );
    for ( ; (rawData.eof() == false) && (pos1 == std::string::npos); ) {        
        precedingStr += s;
        rawData >> s;
        s += ' ';
        pos1 = s.find( startMarker );
    }
    // we didn't find a tag start marker: exit with error
    if ( pos1 == std::string::npos )
        return false; 

    // we found the start marker! We copy the last characters preceding the 
    // marker to the preceding string. This is usually the value of the 
    // preceding element.
    precedingStr += s.substr( 0,pos1 );
    // we continue with everything after the start marker,
    // including the marker itself:
    s = s.substr( pos1 );

    // start looking for the end marker (e.g. '>')
    size_t pos2 = s.find( endMarker );
    for ( ; (rawData.eof() == false) && (pos2 == std::string::npos); ) {
        destStr += s;
        rawData >> s;
        s += ' ';
        pos2 = s.find( endMarker );
    }
    // we didn't find a tag end marker: exit with error
    if ( pos2 == std::string::npos )
        return false;

    // we found the end marker!
    // check if the end marker (e.g. '>') is at the end of the string:
    if ( pos2 != (s.length() - endMarker.length()) ) {
        // exclude the end marker from the trailing string:
        trailingStr = s.substr( pos2 + endMarker.length() );
        // cut off the excess characters but keep the end marker:
        s.resize( pos2 + endMarker.length() );
    }
    destStr += s;
    return true;
}

bool XMLFile::tagIsAComment( const std::string& tagStr, bool& isValidComment )
{
    isValidComment = false;

    // make sure we don't trip over a leading space:
    std::stringstream ss( tagStr );
    std::string head;
    ss >> head;
    int startMarkerLength = std::string( COMMENT_START_MARKER ).length();
    head.resize( startMarkerLength );

    // exit if we can't find the comment start marker:
    if ( head != COMMENT_START_MARKER )
        return false;

	size_t endMarkerLength = std::string( COMMENT_END_MARKER ).length();
    size_t tagLength = tagStr.length();

    // At this point it is a comment alright. If there is no end marker
    // it is malformed and we return without setting isCommentValid to true    
    std::string endMarker = tagStr.substr( tagLength - endMarkerLength );

    // remove trailing spaces just to be sure:
	endMarker = trimTrailingSpaces( endMarker );

    if ( endMarker != COMMENT_END_MARKER )
        return true; 

    // Now we need to check if the comment includes the string "--" apart
    // from the start and end marker (this is not allowed).
    size_t pos = tagStr.find( COMMENT_ILLEGAL_CONTENT,startMarkerLength );
    if ( pos != (tagLength - endMarkerLength) )
        return true;

    // Comment is OK
    isValidComment = true;
    return true;
}

bool XMLFile::tagIsSelfClosing( const std::string& tagStr )
{
    size_t l = tagStr.length();
    if ( l < 4 ) // ? min size = '<a b="c"/>' --> 10 chars?
        return false;
    return (tagStr[l - 2] == TAG_CLOSE_MARKER[0] );
}

// Will return false on self-closing tags!
bool XMLFile::tagIsAClosingTag( const std::string& tagStr )
{
    size_t l = tagStr.length();
    if ( l < 4 )
        return false;
    return (tagStr[1] == TAG_CLOSE_MARKER[0] );
}

bool XMLFile::tagIsXMLPrologTag( const std::string& tagStr )
{
/*
    Examples of prolog statements:

    <?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
    <?xml-stylesheet type="text/css" href="/style/design"?>
    <!-- This is a comment -->
    <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN""http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

*/
    bool isValidComment;
    bool isAComment = tagIsAComment( tagStr,isValidComment );
    if ( isAComment && isValidComment )
        return false;

    size_t l = tagStr.length();
    if ( l < 4 )
        return false;

    return ( 
         (tagStr[0    ] == '<') &&
         (tagStr[l - 1] == '>') &&
        ((tagStr[1    ] == '?') || (tagStr[1    ] == '!')) &&
        ((tagStr[l - 2] == '?') || (tagStr[l - 2] == '!')) 
        );
}

bool XMLFile::tagIsXMLVersionTag( const std::string& tagStr )
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

/* 
    This procedure will fill up the parameter "xmlElement" with the tag and 
    the list with attribute & attribute value sets. Noteworthy here is that
    if the tag is a closing tag, the name of the tag will include the '/'.
    For example, if the tag is </closedTag>, this function will set the tag 
    name to "/closedTag" (excluding the quotes).

    - Returns true on success. 
    - Returns false if the tag is malformed. This should cause the XML parser
      to stop decoding the file.
*/
bool XMLFile::extractTagNameAndAttributes( 
    std::string& tagStr,
    XMLElement& xmlElement )
{    
    // closing tags have no attributes, so we only extract the tag
    bool isClosingTag = tagIsAClosingTag( tagStr );

    std::string s, s2;
    std::stringstream src( tagStr );
    src >> s2;
    bool isProlog = tagIsXMLPrologTag( tagStr );
    if ( isProlog ) { 
        s = s2.substr( 2 );
    }
    else { 
        if ( s2.length() == 0 ) {
            std::cout << "\nTag '" << tagStr << "' is empty!";
            return false;
            //return true;
        }
        s = s2.substr( 1 );
    }
    // tag should follow start marker (e.g. '<') immediately
    if ( s.length() == 0 ) {
        std::cout << "\nTag does not immediately follow tag start marker!";
        return false;
    }

    // defective tag: first char must be a letter
    if ( !isalpha( s[isClosingTag ? 1 : 0] ) ) {
        std::cout << "\nTag starts with an illegal character '"
            << s[isClosingTag ? 1 : 0] << "'";
        return false;
    }

    // strip the closing markers if the tag has no attributes:
    xmlElement.tag = s;
    int l = xmlElement.tag.length() - 1;
    if ( xmlElement.tag[l] == TAG_END_MARKER[0] ) {
        xmlElement.tag.resize(  
            tagIsSelfClosing( tagStr ) ? (l - 1) : l
        );            
        return true;
    }

    // and now the attributes:
    XMLAttribute xmlAttribute;
    s.clear();
    for ( ;!src.eof();) {        
        // First the attribute name. We look for the '=' marker
        size_t pos1 = std::string::npos;
        std::string s2;
        for ( ;!src.eof();) {
            src >> s2;
            s += s2;
            pos1 = s.find( ATTRIBUTE_ASSIGNMENT_MARKER );
            if ( pos1 != std::string::npos )
                break;
        }
        // attribute without assignment or value without attribute is invalid
        // just make sure we are not analysing the prolog end marker
        if ( pos1 == std::string::npos ) { // no '=' marker found
            // posibilities:
            // - we encountered a closing tag like />, ?>, >, etc
            // error in xml markup
            if ( (s == XML_VERSION_END_MARKER) || 
                (s == XML_SELF_CLOSING_MARKER) ||
                (s == TAG_END_MARKER) )
                return true;
            std::cout << "\nIllegal end marker found!"; 
            return false;
        }
        if ( pos1 == 0 ) { // attribute value without attribute is illegal
            std::cout << "\nFound attribute value without attribute name!";
            return false;
        }

        xmlAttribute.name = s;
        xmlAttribute.name.resize( pos1 );

        //std::cout << "\n  attribute name : " << xmlAttribute.name << "=#"; // DEBUG

        if ( (pos1 + 1) >= s.length() )
            s.clear();
        else 
            s = s.substr( pos1 + 1 ); // continue with what is left after '='

        // look for opening " or ' (double or single quote) marker
        size_t pos2;
        std::string valueDelimiter;
        pos1 = s.find( ATTRIBUTE_VALUE_DELIMITER1 );
        pos2 = s.find( ATTRIBUTE_VALUE_DELIMITER2 );
        if ( pos1 != std::string::npos ) {
            if ( pos2 != std::string::npos ) {
                valueDelimiter = (pos1 < pos2) ?
                    ATTRIBUTE_VALUE_DELIMITER1 :
                    ATTRIBUTE_VALUE_DELIMITER2;
            } else
                valueDelimiter = ATTRIBUTE_VALUE_DELIMITER1;
        } else {
            if ( pos2 != std::string::npos )
                valueDelimiter = ATTRIBUTE_VALUE_DELIMITER2;
            else {
                std::cout << "\nCharacters found between '=' and (double) quote attribute value delimiter!";
                return false;
            }
        }
        if ( valueDelimiter == ATTRIBUTE_VALUE_DELIMITER2 )
            pos1 = pos2;
        for ( ;(!src.eof()) && (pos1 == std::string::npos);) {
            src >> s2;
            s += s2 + ' ';
            pos1 = s.find( valueDelimiter );
        }
        // attribute has no value which is illegal
        if ( pos1 == std::string::npos ) {
            std::cout << "\nAttribute without value found!";
            return false;
        }

        // values between '=' and " are illegal therefor we check for it
        if ( pos1 > 0 ) { 
            std::string s3 = s;
            s3.resize( pos1 );
            std::stringstream ss( s3 );
            ss >> s;
            if ( s.length() > 0 ) return false;
        }
        // trim everything before the initial "   // !!! values before " are illegal -> should fail
        s = s.substr( pos1 + 1 ) + ' ';  

        // look for closing " (double quote) marker
        pos1 = s.find( valueDelimiter );
        for ( ;(!src.eof()) && (pos1 == std::string::npos);) {
            src >> s2;
            s += s2 + ' ';
            pos1 = s.find( valueDelimiter );
        }
        if ( pos1 == std::string::npos ) {
            std::cout << "\nCouldn't find a closing marker for the attribute value!";
            return false; // no closing " found which is illegal
        }

        // we exclude the ending double quote
        xmlAttribute.value = s;
        xmlAttribute.value.resize( pos1 ); 

        if ( (pos1 + 1) < s.length() )
            s = s.substr( pos1 + 1 );
        else s.clear();
        
        xmlElement.attributes.push_back( xmlAttribute );
        //std::cout << xmlAttribute.value << "#"; // DEBUG
                    
        // we continue with whatever is left after the ending "
        if ( (pos1 + 1) >= s.length() )
            s.clear();
        else 
            s = s.substr( pos1 + 1 ); // continue with what is left after '='
    }
    return true;
}

bool XMLFile::readFile()
{
    // Open the XML file
    std::stringstream rawData;
    readFileToBuffer( rawData,filename_ );

    std::string precedingStr; 
    std::string tagStr;
    std::string trailingStr;
    std::string previousTagValue;

    int lineNr = 0;

    // keep track of how deep into the tree we go:
    int stackIndex = 0; // nr of items on the stack minus one
    std::stack< XMLElement* > xmlElementPtrStack;
    XMLElement *prevXMLPtr = &elements_;

    // true when we found the final closing tag
    bool finishedReading = false; 

    /*********************************
        Start of XML file decoding
    **********************************/
    for ( ; (!rawData.eof()) && (!finishedReading); ) {
        // read a tag (could be a whole element)  
        extractFromStringStream(
            rawData,
            std::string( TAG_START_MARKER ),
            std::string( TAG_END_MARKER ),
            tagStr,precedingStr,trailingStr );

        // check if we are dealing with an XML style comment and skip the 
        // comment if it is valid or exit if it is invalid:
        bool isValidComment;
        bool isComment = tagIsAComment( tagStr,isValidComment );
        if ( isComment ) {
            if ( !isValidComment ) {
                std::cout << "\nIllegal Comment, exiting!\n"; // DEBUG
                return false;
            }
            // the XML version prolog tag must be on the first character of 
            // the first line, so we keep track of the line number:
            lineNr++; 
            // Prepare for next tag read:
            precedingStr = trailingStr;            
            continue;
        }

        // check if this tag is the xml version description (prolog)
        bool isXMLVersion = tagIsXMLVersionTag( tagStr );
        if ( isXMLVersion ) { 
            // XML version prolog is only allowed in the very beginning of the
            // xml file, not even a comment or whitespace can precede it
            if ( (lineNr > 0) || (precedingStr.length() > 0) ) {
                std::cout << "\nIllegal XML version tag, exiting!\n";  // DEBUG
                return false;
            }
            // exit if error whilst reading xml prolog tag
            XMLElement  xmlElement;
            if ( !extractTagNameAndAttributes( tagStr,xmlElement ) ) {
                std::cout << "\nError extracting attributes, exiting!\n"; // DEBUG
                return false;
            }
            for ( XMLAttribute xmlAttribute : xmlElement.attributes ) {
                if ( xmlAttribute.name == "version" )
                    xmlVersion_ = xmlAttribute.value;
                if ( xmlAttribute.name == "encoding" )
                    xmlCharset_ = xmlAttribute.value;
            }
            // Prepare for next tag read:
            precedingStr = trailingStr;
            continue;
        }

        // check if this tag is any other type of prolog tag and skip it:
        bool isXMLPrologTag = tagIsXMLPrologTag( tagStr );
        if ( isXMLPrologTag ) {
            /*
                XML prologs should only appear in the very beginning of the
                xml file, chrome however allows them to appear elsewhere
            // if ( elementNr > 0 )
            //    return false;
            */
            lineNr++;
            // Prepare for next tag read:
            precedingStr = trailingStr;            
            continue;
        }

        // OK so we are done with filtering prolog stuff and comments,
        // now start with reading the xml elements
        XMLElement xmlElement;

        /*
            First, check if this tag is the start of a <![CDATA[ section.
            CDATA sections are not allowed outside of elements, meaning
            after the last closing tag or before the first normal opening 
            tag. 
            Multiple CDATA tags can occur in a single element.
            They cannot be inside another tagname though.
        */
        size_t position = tagStr.find( CDATA_START_MARKER );
        if ( position != std::string::npos ) { // it is a CDATA section!

            // TODO: built in check if we found a real opening tag already!

            if ( position > 0 ) { 
                precedingStr += ' ' + tagStr.substr( 0, position );
                tagStr = tagStr.substr( position );
            }           
            for ( position = std::string::npos;
                (!rawData.eof()) && (position == std::string::npos);) {
                std::string s;
                rawData >> s;
                position = tagStr.find( CDATA_END_MARKER );
                tagStr += ' ' + s;
            }
            // no end marker found means error in xml file
            if ( position == std::string::npos ) {                 
                std::cout << "\nCDATA end marker not found, exiting!\n"; // DEBUG
                return false;
            }
            position = tagStr.find( CDATA_END_MARKER );
            size_t endPos = position + std::string( CDATA_END_MARKER ).length();
            trailingStr = tagStr.substr( endPos ) + ' ';
            tagStr.resize( endPos );

            //std::cout << "\n@" << tagStr << "@, |" << trailingStr << "|\n";

            // skip for now:
            precedingStr = trailingStr;
            continue;
        }

        bool extractSuccess = extractTagNameAndAttributes( tagStr,xmlElement );
        if ( extractSuccess == false )
            return false; // exit on decoding error

        bool isClosingTag = tagIsAClosingTag( tagStr );
        bool isSelfClosingTag = tagIsSelfClosing( tagStr );

        // whatever precedes the closing tag is the value inside the element:
        if ( isClosingTag ) {
            xmlElement.value = precedingStr;
            // trim the trailing space:
            int l = xmlElement.value.length();
            if ( l >= 1 ) {
                l--;
                if ( xmlElement.value[l] == ' ' )
                    xmlElement.value.resize( l );
            }
        }
        // Prepare for next tag read:
        precedingStr = trailingStr;

        // *****************************************************
        // Now load the xml file into the memory tree structure:
        // *****************************************************
        if ( !isClosingTag ) { // found opening tag
            if ( !isSelfClosingTag ) { 
                if ( stackIndex == 0 ) { // found the initial opening tag  
                    // assign opening tag and attributes to the root tag
                    *prevXMLPtr = xmlElement;
                }
                else { 
                    // we are not dealing with the root tag but with child tag:
                    prevXMLPtr->children.push_back( xmlElement );                

                    // save the pointer and go inside the child xmlElement:
                    xmlElementPtrStack.push( prevXMLPtr );

                    // we switch to the newly created child XMLElement:
                    prevXMLPtr = &(prevXMLPtr->children.back());
                }
                // we keep track of where we are in the tree:
                stackIndex++;
            }
            else { // tag is selfclosing
                prevXMLPtr->children.push_back( xmlElement );
            }

            // debug **********************************************************
            std::cout << "\n<" << xmlElement.tag;
            for ( unsigned i = 0; i < xmlElement.attributes.size(); i++ )
                std::cout << " " << xmlElement.attributes[i].name
                << "=\"" << xmlElement.attributes[i].value << "\"";
            if ( isSelfClosingTag )
                std::cout << "/";
            std::cout << ">";
            // end debug *****************************************************/
        }
        else { // tag is a closing tag //or selfclosing tag
            if ( isSelfClosingTag )
                std::cout << "\nSelf closing tag found in unexpected area!"; // DEBUG

            // check if the closing tag corresponds to the opening tag of
            // the current tag we are working on. First we must               
            // remove the '/' character from the closing tag:
            std::string closingTag = xmlElement.tag.substr( 1 );
            if ( prevXMLPtr->tag != closingTag ) {
                std::cout << "\nClosing and Opening tags do not match!";  // DEBUG
                return false;
            }

            // the only thing left to add is the value of the element now
            // that it is closed:
            prevXMLPtr->value = xmlElement.value;

            // now we go back to the previous element:
            if ( stackIndex > 1 ) {
                prevXMLPtr = xmlElementPtrStack.top();
                xmlElementPtrStack.pop();
                stackIndex--;
            }
            else {                
                finishedReading = true;
            }
            // debug
            std::cout 
                << xmlElement.value
                << "<" << xmlElement.tag << ">";
            if( finishedReading )
                std::cout << "\nParsing of xml file finished!";
            // end debug
        }
    }
    return true;
}

// this function is recursive!
void XMLFile::printXMLElement( const XMLElement& xmlElement,int depth )
{
    const char *spacer = "   ";
    bool hasAttributes = xmlElement.attributes.size() > 0;
    bool hasChildren = (xmlElement.children.size() > 0);
    // display valueless tags as self-closing
    bool makeSelfClosing = (xmlElement.value.length() == 0) && (!hasChildren);

    std::cout << "\n";
    for ( int i = 0; i < depth; i++ )
        std::cout << spacer;
    std::cout << "<" << xmlElement.tag;
    for ( XMLAttribute xmlAttribute : xmlElement.attributes ) {
        std::cout
            << " " << xmlAttribute.name << "=\""
            << xmlAttribute.value << "\"";
    }
    if ( makeSelfClosing ) 
        std::cout << " />";
    else {
        std::cout << ((hasAttributes && !hasChildren) ? ">\n" : ">");
        for ( XMLElement xmlElement : xmlElement.children ) {
            printXMLElement( xmlElement,depth + 1 );
        }
        if ( hasChildren && (xmlElement.value.length() > 0) ) {
            std::cout << "\n";
            for ( int i = 0; i < depth + 1; i++ )
                std::cout << spacer;
        }
        std::cout << xmlElement.value;
        if ( hasChildren ) {
            std::cout << "\n";
            for ( int i = 0; i < depth; i++ )
                std::cout << spacer;
        }
        std::cout << "</" << xmlElement.tag << ">";
    }
}

void XMLFile::print()
{
    if (!isLoaded_ )
        return;
    printXMLElement( elements_,0 );
}

void main()
{
    XMLFile     xmlFile( "c:\\RTSMedia\\beach-ball.svg" );
    std::cout 
        << "\nXML file was loaded correctly: "
        << (xmlFile.isLoaded() ? "yes" : "no") << "\n";
    xmlFile.print();
	_getch();
    
    XMLFile     xmlFile2( "c:\\RTSMedia\\purchase.xml" );
    std::cout
        << "\nXML file was loaded correctly: "
        << (xmlFile2.isLoaded() ? "yes" : "no") << "\n";
    xmlFile2.print();
    _getch();
}

